#ifndef VOICEMODULE_H
#define VOICEMODULE_H

#include "../config.h"

/**
 * @brief 音效标识
 */
enum class SoundID {
    SHOOT       = 0x01,
    EMPTY_CLIP  = 0x06,
    RELOAD      = 0x08
};

/**
 * @brief 语音模块驱动
 *
 * 封装底层单线通信协议，提供播放、停止和音量设置接口。
 */
class VoiceModule {
public:
    VoiceModule();

    /**
     * @brief 初始化语音模块总线（引脚设置、默认音量等）
     */
    void init();
    
    /**
     * @brief 播放指定音效
     * @param sound 要播放的音效 ID
     */
    void play(SoundID sound);

    /**
     * @brief 停止播放
     */
    void stop();

    /**
     * @brief 设置音量
     * @param volume 0-30
     */
    void setVolume(int volume);

private:
    /* 低级通信函数 */
    void _sendCommand(uint8_t command);
    void _sendCommand(uint8_t cmd_header, uint8_t param);
    void _sendCommand(uint8_t cmd_header, uint8_t param1, uint8_t param2, uint8_t cmd_footer);
    void _sendByte(uint8_t data);
};

#endif // VOICEMODULE_H