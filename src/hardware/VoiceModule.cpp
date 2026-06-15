#include "VoiceModule.h"

VoiceModule::VoiceModule() {}

void VoiceModule::init() {
    pinMode(VOICE_SERIAL_PIN, OUTPUT);
    digitalWrite(VOICE_SERIAL_PIN, HIGH); // 总线空闲为高
    delay(500); // 模块上电稳定
    setVolume(30); // 默认音量
}

void VoiceModule::play(SoundID sound) {
    uint8_t soundIndex = static_cast<uint8_t>(sound);
    _sendCommand(0x0A, soundIndex / 10, soundIndex % 10, 0x0B);
}

void VoiceModule::stop() {
    _sendCommand(0x12);
}

void VoiceModule::setVolume(int volume) {
    if (volume >= 0 && volume <= 30) {
        // 音量设置：头 + 十位 + 个位 + 尾
        _sendCommand(0x0A, volume / 10, volume % 10, 0x0C);
    }
}

// ----------------- Private Low-Level Functions -----------------

void VoiceModule::_sendCommand(uint8_t command) {
    _sendByte(command);
}

void VoiceModule::_sendCommand(uint8_t cmd_header, uint8_t param) {
    _sendByte(cmd_header);
    _sendByte(param);
}

void VoiceModule::_sendCommand(uint8_t cmd_header, uint8_t param1, uint8_t param2, uint8_t cmd_footer) {
    _sendByte(cmd_header);
    _sendByte(param1);
    _sendByte(param2);
    _sendByte(cmd_footer);
}

void VoiceModule::_sendByte(uint8_t data) {
    digitalWrite(VOICE_SERIAL_PIN, HIGH);
    delayMicroseconds(100);
    digitalWrite(VOICE_SERIAL_PIN, LOW);
    delayMicroseconds(3200);

    for (int i = 0; i < 8; i++) {
        digitalWrite(VOICE_SERIAL_PIN, HIGH);
        if (data & 0x01) {
            delayMicroseconds(600);
            digitalWrite(VOICE_SERIAL_PIN, LOW);
            delayMicroseconds(200);
        } else {
            delayMicroseconds(200);
            digitalWrite(VOICE_SERIAL_PIN, LOW);
            delayMicroseconds(600);
        }
        data >>= 1;
    }
    digitalWrite(VOICE_SERIAL_PIN, HIGH);
}