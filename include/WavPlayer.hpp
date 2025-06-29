#ifndef WAV_PLAYER_H
#define WAV_PLAYER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <driver/i2s.h>
#include <driver/dac.h>

class WavPlayer {
private:
    static const i2s_port_t I2S_NUM = I2S_NUM_0;
    static const size_t BUFFER_SIZE = 512;
    
    struct WAVHeader {
        char riff[4];
        uint32_t size;
        char wave[4];
        char fmt[4];
        uint32_t fmt_size;
        uint16_t format;
        uint16_t channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t block_align;
        uint16_t bits_per_sample;
        char data[4];
        uint32_t data_size;
    };
    
    bool initialized = false;
    float volume = 0.5f;  // 0.0 ~ 1.0
    uint8_t buffer[BUFFER_SIZE];
    
    // I2S初期化
    bool initI2S() {
        // DACを有効化
        dac_output_enable(DAC_CHANNEL_2);  // GPIO26
        
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
            .sample_rate = 44100,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_MSB,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len = 256,
            .use_apll = false,
            .tx_desc_auto_clear = true,
            .fixed_mclk = 0
        };

        esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
        if (err != ESP_OK) {
            Serial.printf("I2Sドライバインストール失敗: %d\n", err);
            return false;
        }
        
        i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);
        return true;
    }
    
    // ボリューム適用
    int16_t applyVolume(int16_t sample) {
        int32_t result = (int32_t)(sample * volume * 0.7f);  // 最大70%に制限
        if (result > 32767) result = 32767;
        if (result < -32768) result = -32768;
        return (int16_t)result;
    }

public:
    WavPlayer() {}
    
    ~WavPlayer() {
        if (initialized) {
            i2s_driver_uninstall(I2S_NUM);
        }
    }
    
    // 初期化
    bool begin() {
        if (!SPIFFS.begin(true)) {
            Serial.println("SPIFFS初期化失敗");
            return false;
        }
        
        if (!initI2S()) {
            return false;
        }
        
        initialized = true;
        Serial.println("WavPlayer初期化完了");
        return true;
    }
    
    // ボリューム設定（0-100）
    void setVolume(int percent) {
        percent = constrain(percent, 0, 100);
        volume = percent / 100.0f;
        Serial.printf("ボリューム設定: %d%%\n", percent);
    }
    
    // ボリューム取得
    int getVolume() {
        return (int)(volume * 100);
    }
    
    // WAVファイル再生（ブロッキング）
    bool play(const char* filename) {
        if (!initialized) {
            Serial.println("WavPlayerが初期化されていません");
            return false;
        }
        
        File file = SPIFFS.open(filename);
        if (!file) {
            Serial.printf("ファイルを開けません: %s\n", filename);
            return false;
        }
        
        WAVHeader header;
        if (file.read((uint8_t*)&header, sizeof(header)) != sizeof(header)) {
            file.close();
            return false;
        }
        
        // WAVファイル検証
        if (memcmp(header.riff, "RIFF", 4) != 0 || 
            memcmp(header.wave, "WAVE", 4) != 0) {
            Serial.println("無効なWAVファイル");
            file.close();
            return false;
        }
        
        // サンプルレート設定
        i2s_set_sample_rates(I2S_NUM, header.sample_rate);
        
        // データ部分まで移動
        file.seek(44);
        
        size_t bytes_written;
        while (file.available()) {
            size_t bytes_read = file.read(buffer, BUFFER_SIZE);
            if (bytes_read == 0) break;
            
            // 16ビットモノラルの場合のボリューム適用
            if (header.bits_per_sample == 16 && header.channels == 1) {
                int16_t* samples = (int16_t*)buffer;
                for (size_t i = 0; i < bytes_read / 2; i++) {
                    samples[i] = applyVolume(samples[i]);
                }
            }
            // 他のフォーマットは必要に応じて実装
            
            i2s_write(I2S_NUM, buffer, bytes_read, &bytes_written, portMAX_DELAY);
        }
        
        i2s_zero_dma_buffer(I2S_NUM);
        file.close();
        return true;
    }
    
    // ビープ音再生
    void beep(int frequency = 1000, int duration = 200) {
        if (!initialized) return;
        
        const int sampleRate = 22050;
        const int samples = (sampleRate * duration) / 1000;
        
        i2s_set_sample_rates(I2S_NUM, sampleRate);
        
        size_t bytes_written;
        for (int i = 0; i < samples; i += 256) {
            for (int j = 0; j < 256 && (i + j) < samples; j++) {
                float angle = 2.0f * PI * frequency * (i + j) / sampleRate;
                int16_t sample = applyVolume((int16_t)(sin(angle) * 32767));
                buffer[j * 2] = sample & 0xFF;
                buffer[j * 2 + 1] = (sample >> 8) & 0xFF;
            }
            i2s_write(I2S_NUM, buffer, 512, &bytes_written, portMAX_DELAY);
        }
        
        i2s_zero_dma_buffer(I2S_NUM);
    }
    
    // ファイル一覧取得
    void listFiles() {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        Serial.println("WAVファイル一覧:");
        while (file) {
            String filename = file.name();
            if (filename.endsWith(".wav") || filename.endsWith(".WAV")) {
                Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
            }
            file = root.openNextFile();
        }
        root.close();
    }
};

#endif