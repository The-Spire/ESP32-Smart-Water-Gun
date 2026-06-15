#include "WaterGunLogic.h"
#include "../ui/UIManager.h"
#include "../hardware/LedStrip.h"

// 构造函数：保存子系统指针并初始化逻辑状态
WaterGunLogic::WaterGunLogic(InputManager* input, UIManager* ui, OutputManager* output,
                             VoiceModule* voice, SettingsManager* settings, LedStrip* led)
    : _inputManager(input),
      _uiManager(ui),
      _outputManager(output),
      _voiceModule(voice),
      _settingsManager(settings),
      _ledStrip(led),
      _currentState(AppState::STATE_MAIN_SCREEN) {
    _performance_data_enabled = true;
    _last_activity_time = 0;
}

// 初始化逻辑模块：恢复默认状态并应用初始设置
void WaterGunLogic::init() {
    _currentState = AppState::STATE_MAIN_SCREEN;
    _currentAmmo = _settingsManager->settings.maxAmmo;
    _uiManager->updateAmmo(_currentAmmo, _settingsManager->settings.maxAmmo);
    _lastShotTime = 0;

    _reloadPhase = ReloadPhase::IDLE;
    _reloadPhaseStartTime = 0;
    _emptyStateEnterTime = 0;
    _isWaitingForTriggerReleaseInMenu = false;

    if (_settingsManager->settings.humidifierEnabled) {
        _outputManager->humidifierOn();
    }

    if (_settingsManager->settings.laserEnabled) {
        _outputManager->laserOn();
    }

    applyAndSaveAllSettings();
    resetStandbyTimer();
}

// 重置待机计时器：每次有输入时调用
void WaterGunLogic::resetStandbyTimer() {
    _last_activity_time = millis();
}

// 将当前设置同步到硬件输出和 UI 显示
void WaterGunLogic::updateLiveSettings(AppSettings* live_settings) {
    if (!live_settings || !_ledStrip || !_voiceModule || !_outputManager) {
        return;
    }

    // 更新 LED 效果颜色和速度
    _ledStrip->setAllEffectsColor(live_settings->lightColor);
    _ledStrip->setAllEffectsSpeed(live_settings->lightSpeed);

    // 应用音量设置
    _voiceModule->setVolume(live_settings->volume);

    // 控制系统信息显示开关
    _uiManager->setSystemInfoVisibility(
        live_settings->showFps,
        live_settings->showRam,
        live_settings->showCpu
    );

    // 加湿器开关
    if (live_settings->humidifierEnabled) {
        _outputManager->humidifierOn();
    } else {
        _outputManager->humidifierOff();
    }

    // 激光开关
    if (live_settings->laserEnabled) {
        _outputManager->laserOn();
    } else {
        _outputManager->laserOff();
    }
}

// 是否启用性能数据计算
bool WaterGunLogic::shouldCalculatePerformanceData() const {
    return _performance_data_enabled;
}

// 应用当前设置并保存到配置存储
void WaterGunLogic::applyAndSaveAllSettings() {
    updateLiveSettings(&_settingsManager->settings);
    _settingsManager->save();
}

// 主循环更新：读取输入、处理状态机和输出控制
void WaterGunLogic::update() {
    InputEvent event = _inputManager->getEvent();

    // 任何输入事件都会重置自动待机计时器
    if (event != InputEvent::NONE) {
        resetStandbyTimer();
    }

    // 预先转换为毫秒值，避免在状态机中重复计算
    unsigned int fire_interval_ms = 1000 / _settingsManager->settings.fireRate;
    unsigned int reload_interval_ms = 1000 / _settingsManager->settings.reloadRate;
    unsigned int auto_reload_delay_ms = _settingsManager->settings.autoReloadDelay * 1000;

    // 自动待机判断
    if (_settingsManager->settings.autoStandbyMinutes > 0) {
        unsigned long standby_timeout_ms =
            (unsigned long)_settingsManager->settings.autoStandbyMinutes * 60 * 1000;

        if (millis() - _last_activity_time > standby_timeout_ms) {
            enterStandbyMode();
        }
    }

    // 状态机主逻辑
    switch (_currentState) {
        case AppState::STATE_MAIN_SCREEN:
            // 主界面：扳机触发射击，编码器进入设置
            if (event == InputEvent::TRIGGER_PRESS) {
                if (_currentAmmo > 0) {
                    _currentState = AppState::STATE_MAIN_FIRING;

                    if (_settingsManager->settings.bulletType == BulletType::WATER) {
                        if (_settingsManager->settings.pumpEnabled) {
                            _outputManager->pumpOn();
                        }
                    } else {
                        if (_settingsManager->settings.laserEnabled) {
                            _outputManager->laserOn();
                        }
                    }

                    if (_settingsManager->settings.lightsEnabled) {
                        _outputManager->indicatorStartBlinking();
                    }

                    if (_settingsManager->settings.soundEnabled) {
                        _voiceModule->play(SoundID::SHOOT);
                    }

                    _currentAmmo--;
                    _uiManager->updateAmmo(_currentAmmo, _settingsManager->settings.maxAmmo);
                    _lastShotTime = millis();
                } else {
                    _currentState = AppState::STATE_MAIN_EMPTY;
                    _emptyStateEnterTime = millis();

                    if (_settingsManager->settings.soundEnabled) {
                        _voiceModule->play(SoundID::EMPTY_CLIP);
                    }
                }
            } else if (event == InputEvent::ENCODER_ROTATED_CW ||
                       event == InputEvent::ENCODER_ROTATED_CCW) {
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen(&_settingsManager->settings);
            }
            break;

        case AppState::STATE_MAIN_FIRING:
            // 射击状态：持续发射、触发释放返回主界面
            if (event == InputEvent::TRIGGER_RELEASE) {
                _currentState = AppState::STATE_MAIN_SCREEN;
                _outputManager->laserOff();

                if (_settingsManager->settings.pumpEnabled) {
                    _outputManager->pumpOff();
                }

                if (_settingsManager->settings.lightsEnabled) {
                    _outputManager->indicatorStopBlinking();
                }

                if (_settingsManager->settings.soundEnabled) {
                    _voiceModule->stop();
                }
            }

            if (millis() - _lastShotTime > fire_interval_ms) {
                if (_currentAmmo > 0) {
                    _currentAmmo--;
                    _uiManager->updateAmmo(_currentAmmo, _settingsManager->settings.maxAmmo);
                    _lastShotTime = millis();
                }

                if (_settingsManager->settings.bulletType == BulletType::LASER &&
                    _settingsManager->settings.laserEnabled) {
                    if (_laser_is_on) {
                        _outputManager->laserOff();
                        _laser_is_on = false;
                    } else {
                        _outputManager->laserOn();
                        _laser_is_on = true;
                    }
                }

                if (_currentAmmo <= 0) {
                    _currentState = AppState::STATE_MAIN_EMPTY;
                    _emptyStateEnterTime = millis();
                    _outputManager->laserOff();

                    if (_settingsManager->settings.pumpEnabled) {
                        _outputManager->pumpOff();
                    }

                    if (_settingsManager->settings.lightsEnabled) {
                        _outputManager->indicatorStopBlinking();
                    }

                    if (_settingsManager->settings.soundEnabled) {
                        _voiceModule->stop();
                        _voiceModule->play(SoundID::EMPTY_CLIP);
                    }
                }
            }
            break;

        case AppState::STATE_MAIN_EMPTY:
            // 空仓待机：扳机提醒音，等待自动换弹
            if (event == InputEvent::TRIGGER_PRESS) {
                _voiceModule->play(SoundID::EMPTY_CLIP);
            }

            if (millis() - _emptyStateEnterTime > auto_reload_delay_ms) {
                _currentState = AppState::STATE_MAIN_RELOADING;
                _reloadPhase = ReloadPhase::EMPTY_DELAY;
                _reloadPhaseStartTime = millis();
            }
            break;

        case AppState::STATE_MAIN_RELOADING:
            // 换弹流程：先延迟，再逐发填充弹药
            switch (_reloadPhase) {
                case ReloadPhase::EMPTY_DELAY:
                    if (millis() - _reloadPhaseStartTime > 400) {
                        _reloadPhase = ReloadPhase::REFILLING;
                        _reloadPhaseStartTime = millis();

                        if (_settingsManager->settings.soundEnabled) {
                            _voiceModule->play(SoundID::RELOAD);
                        }
                    }
                    break;

                case ReloadPhase::REFILLING:
                    if (millis() - _reloadPhaseStartTime > reload_interval_ms) {
                        if (_currentAmmo < _settingsManager->settings.maxAmmo) {
                            _currentAmmo++;
                            _uiManager->updateAmmo(
                                _currentAmmo, _settingsManager->settings.maxAmmo);
                            _reloadPhaseStartTime = millis();
                        } else {
                            _currentState = AppState::STATE_MAIN_SCREEN;
                            _reloadPhase = ReloadPhase::IDLE;
                        }
                    }
                    break;

                case ReloadPhase::IDLE:
                    _currentState = AppState::STATE_MAIN_SCREEN;
                    break;
            }
            break;

        case AppState::STATE_SETTINGS_SCREEN:
            // 设置界面：滚动设置、短按进入子菜单、长按返回主屏
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSettings(1);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSettings(-1);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    int selected_index = _uiManager->getSelectedSettingIndex();

                    if (selected_index == 0) {
                        _currentState = AppState::STATE_SETTINGS_SUBMENU_BULLET;
                        _uiManager->showBulletSettingsScreen(&_settingsManager->settings);
                    } else if (selected_index == 1) {
                        _currentState = AppState::STATE_SETTINGS_SUBMENU_SYSINFO;
                        _uiManager->showSysInfoScreen(&_settingsManager->settings);
                    } else if (selected_index == 2) {
                        _currentState = AppState::STATE_SETTINGS_SUBMENU_THEME;
                        _uiManager->showThemeSettingsScreen(&_settingsManager->settings);
                    } else if (selected_index == 3) {
                        _currentState = AppState::STATE_SETTINGS_SUBMENU_SOUND;
                        _uiManager->showSoundSettingsScreen(&_settingsManager->settings);
                    } else if (selected_index == 4) {
                        _currentState = AppState::STATE_SETTINGS_SUBMENU_LIGHT;
                        _uiManager->showLightSettingsScreen(&_settingsManager->settings);
                    } else if (selected_index == 5) {
                        _currentState = AppState::STATE_SETTINGS_SUBMENU_FUNC;
                        _uiManager->showFuncSettingsScreen(&_settingsManager->settings);
                    } else if (selected_index == 6) { 
                        _currentState = AppState::STATE_SHOWCASE_ANIM;
                        _uiManager->showShowcaseAnimation();
                        _anim_start_time = millis(); // 记录开始时间
                    }
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                _currentState = AppState::STATE_MAIN_SCREEN;
                _uiManager->showMainScreen();
            }
            break;

        case AppState::STATE_SETTINGS_SUBMENU_FUNC:
            // 功能设置子菜单：左右滚动、确认、长按保存并返回
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSubmenu(1);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSubmenu(-1);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    _uiManager->actionSubmenu();
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                applyAndSaveAllSettings();
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen();
            }
            break;

        case AppState::STATE_SETTINGS_SUBMENU_SOUND:
            // 声音设置子菜单：调整音量时实时应用
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSubmenu(1);
                _voiceModule->setVolume(_settingsManager->settings.volume);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSubmenu(-1);
                _voiceModule->setVolume(_settingsManager->settings.volume);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    _uiManager->actionSubmenu();
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                applyAndSaveAllSettings();
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen();
            }
            break;

        case AppState::STATE_SETTINGS_SUBMENU_BULLET:
            // 子弹设置子菜单：上下选择当前弹药相关项
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSubmenu(1);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSubmenu(-1);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    _uiManager->actionSubmenu();
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                applyAndSaveAllSettings();
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen();
            }
            break;

        case AppState::STATE_SETTINGS_SUBMENU_LIGHT:
            // 灯光设置子菜单：调整灯光参数
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSubmenu(1);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSubmenu(-1);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    _uiManager->actionSubmenu();
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                applyAndSaveAllSettings();
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen();
            }
            break;

        case AppState::STATE_SETTINGS_SUBMENU_THEME:
            // 主题设置子菜单：切换主题样式
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSubmenu(1);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSubmenu(-1);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    _uiManager->actionSubmenu();
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                applyAndSaveAllSettings();
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen(&_settingsManager->settings);
            }
            break;

        case AppState::STATE_SETTINGS_SUBMENU_SYSINFO:
            // 系统信息子菜单：查看系统状态并可返回主设置界面
            if (event == InputEvent::ENCODER_ROTATED_CW) {
                _uiManager->scrollSubmenu(1);
            } else if (event == InputEvent::ENCODER_ROTATED_CCW) {
                _uiManager->scrollSubmenu(-1);
            } else if (event == InputEvent::TRIGGER_PRESS) {
                _isWaitingForTriggerReleaseInMenu = true;
            } else if (event == InputEvent::TRIGGER_RELEASE) {
                if (_isWaitingForTriggerReleaseInMenu) {
                    _isWaitingForTriggerReleaseInMenu = false;
                    _uiManager->actionSubmenu();
                }
            } else if (event == InputEvent::TRIGGER_LONG_PRESS) {
                _isWaitingForTriggerReleaseInMenu = false;
                applyAndSaveAllSettings();
                _currentState = AppState::STATE_SETTINGS_SCREEN;
                _uiManager->showSettingsScreen(&_settingsManager->settings);
            }
            break;

        case AppState::STATE_SHOWCASE_ANIM:
            
            if (millis() - _anim_start_time > 10000 || event == InputEvent::TRIGGER_PRESS) {
                _uiManager->closeShowcaseAnimation(); 
                _currentState = AppState::STATE_SETTINGS_SCREEN;
           
                _uiManager->showSettingsScreen(&_settingsManager->settings);
            }
            break;
    }
}
