#include <Arduino.h>
#include <SPIFFS.h>
#include <driver/i2s.h>
#include <driver/dac.h>

// I2S設定（内蔵DAC使用）
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_PIN 26  // 未使用（内蔵DACの場合）
#define I2S_WS_PIN 25   // 未使用（内蔵DACの場合）
#define I2S_DATA_PIN 26 // GPIO26 (DAC2)

// バッファサイズ
#define BUFFER_SIZE 512

// WAVヘッダー構造体
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

File wavFile;
uint8_t buffer[BUFFER_SIZE];
WAVHeader wavHeader;

// ボリューム制御用（0-100）
int volumePercent = 50;  // デフォルト50%

// I2S初期化（内蔵DAC使用）
void setupI2S() {
    // DACを有効化
    dac_output_enable(DAC_CHANNEL_2);  // GPIO26
    
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = 44100,  // 初期値（WAVファイルに応じて変更）
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // モノラル出力
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);  // GPIO26のみ有効化
}

// WAVヘッダー読み込み
bool readWAVHeader(File &file) {
    if (file.read((uint8_t*)&wavHeader, sizeof(WAVHeader)) != sizeof(WAVHeader)) {
        return false;
    }

    // WAVファイルの検証
    if (memcmp(wavHeader.riff, "RIFF", 4) != 0 || 
        memcmp(wavHeader.wave, "WAVE", 4) != 0 ||
        memcmp(wavHeader.fmt, "fmt ", 4) != 0) {
        Serial.println("無効なWAVファイルです");
        return false;
    }

    Serial.println("WAVファイル情報:");
    Serial.printf("  サンプルレート: %d Hz\n", wavHeader.sample_rate);
    Serial.printf("  チャンネル数: %d\n", wavHeader.channels);
    Serial.printf("  ビット深度: %d bit\n", wavHeader.bits_per_sample);
    Serial.printf("  データサイズ: %d bytes\n", wavHeader.data_size);

    // I2Sサンプルレートを設定
    i2s_set_sample_rates(I2S_NUM, wavHeader.sample_rate);

    return true;
}

// 16ビットサンプルにボリューム適用（クリッピング防止付き）
int16_t applyVolume16(int16_t sample) {
    // ボリューム適用（0-100% → 0.0-1.0）
    float volumeFactor = volumePercent / 100.0f;
    
    // さらに全体的な音量を下げる（DAC出力の歪み防止）
    volumeFactor *= 0.7f;  // 最大70%に制限
    
    int32_t result = (int32_t)(sample * volumeFactor);
    
    // クリッピング防止
    if (result > 32767) result = 32767;
    if (result < -32768) result = -32768;
    
    return (int16_t)result;
}

// 8ビットサンプルを16ビットに変換してボリューム適用
int16_t convert8to16WithVolume(uint8_t sample8) {
    // 8ビット（0-255）を16ビット（-32768 to 32767）に変換
    int16_t sample16 = ((int16_t)sample8 - 128) * 256;
    return applyVolume16(sample16);
}

// WAVファイル再生
void playWAV(const char* filename) {
    wavFile = SPIFFS.open(filename);
    if (!wavFile) {
        Serial.printf("ファイルを開けません: %s\n", filename);
        return;
    }

    Serial.printf("再生開始: %s (サイズ: %d bytes)\n", filename, wavFile.size());
    Serial.printf("現在のボリューム: %d%%\n", volumePercent);

    if (!readWAVHeader(wavFile)) {
        wavFile.close();
        return;
    }

    // データ部分まで移動
    wavFile.seek(44);  // 標準的なWAVヘッダーサイズ

    size_t bytes_written;
    uint32_t bytes_read;
    uint8_t tempBuffer[BUFFER_SIZE];
    
    while (wavFile.available()) {
        bytes_read = wavFile.read(tempBuffer, BUFFER_SIZE);
        
        if (bytes_read > 0) {
            size_t outputSize = 0;
            
            // 8ビットWAVの処理
            if (wavHeader.bits_per_sample == 8) {
                // モノラル8ビット
                if (wavHeader.channels == 1) {
                    for (int i = 0; i < bytes_read; i++) {
                        int16_t sample = convert8to16WithVolume(tempBuffer[i]);
                        buffer[outputSize++] = sample & 0xFF;
                        buffer[outputSize++] = (sample >> 8) & 0xFF;
                    }
                }
                // ステレオ8ビット
                else if (wavHeader.channels == 2) {
                    for (int i = 0; i < bytes_read; i += 2) {
                        int16_t left = convert8to16WithVolume(tempBuffer[i]);
                        int16_t right = convert8to16WithVolume(tempBuffer[i + 1]);
                        int16_t mono = (left + right) / 2;
                        buffer[outputSize++] = mono & 0xFF;
                        buffer[outputSize++] = (mono >> 8) & 0xFF;
                    }
                }
            }
            // 16ビットWAVの処理
            else if (wavHeader.bits_per_sample == 16) {
                // モノラル16ビット
                if (wavHeader.channels == 1) {
                    for (int i = 0; i < bytes_read; i += 2) {
                        int16_t sample = tempBuffer[i] | (tempBuffer[i + 1] << 8);
                        sample = applyVolume16(sample);
                        buffer[outputSize++] = sample & 0xFF;
                        buffer[outputSize++] = (sample >> 8) & 0xFF;
                    }
                }
                // ステレオ16ビット
                else if (wavHeader.channels == 2) {
                    for (int i = 0; i < bytes_read; i += 4) {
                        int16_t left = tempBuffer[i] | (tempBuffer[i + 1] << 8);
                        int16_t right = tempBuffer[i + 2] | (tempBuffer[i + 3] << 8);
                        left = applyVolume16(left);
                        right = applyVolume16(right);
                        int16_t mono = (left + right) / 2;
                        buffer[outputSize++] = mono & 0xFF;
                        buffer[outputSize++] = (mono >> 8) & 0xFF;
                    }
                }
            }

            // I2Sに書き込み
            i2s_write(I2S_NUM, buffer, outputSize, &bytes_written, portMAX_DELAY);
        }
    }

    // 再生終了時にDACをクリア
    i2s_zero_dma_buffer(I2S_NUM);
    
    wavFile.close();
    Serial.println("再生終了");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("SPIFFS WAV再生テスト開始（改良版）");

    // SPIFFS初期化
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS初期化失敗");
        return;
    }
    Serial.println("SPIFFS初期化成功");

    // SPIFFS情報表示
    Serial.printf("SPIFFS総容量: %d bytes\n", SPIFFS.totalBytes());
    Serial.printf("SPIFFS使用量: %d bytes\n", SPIFFS.usedBytes());
    Serial.printf("SPIFFS空き容量: %d bytes\n", SPIFFS.totalBytes() - SPIFFS.usedBytes());

    // I2S初期化
    setupI2S();
    Serial.println("I2S初期化完了");

    // 初期ボリューム設定
    Serial.printf("初期ボリューム: %d%%\n", volumePercent);

    // ルートディレクトリのWAVファイルをリスト表示
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    Serial.println("\nWAVファイル一覧:");
    while (file) {
        String filename = file.name();
        if (filename.endsWith(".wav") || filename.endsWith(".WAV")) {
            Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
    root.close();

    Serial.println("\n使用方法:");
    Serial.println("- シリアルモニタからファイル名を入力して再生");
    Serial.println("- 'list' でファイル一覧表示");
    Serial.println("- 'vol XX' で音量設定 (0-100)");
    Serial.println("- 'test' でボリュームテスト（ビープ音）");
}

// 簡単なビープ音生成（ボリュームテスト用）
void playBeep(int frequency, int duration) {
    Serial.printf("ビープ音再生: %dHz, %dms, 音量%d%%\n", frequency, duration, volumePercent);
    
    const int sampleRate = 22050;
    const int samples = (sampleRate * duration) / 1000;
    const float volumeFactor = (volumePercent / 100.0f) * 0.7f;
    
    i2s_set_sample_rates(I2S_NUM, sampleRate);
    
    uint8_t beepBuffer[512];
    size_t bytes_written;
    
    for (int i = 0; i < samples; i += 256) {
        for (int j = 0; j < 256 && (i + j) < samples; j++) {
            float angle = 2.0f * PI * frequency * (i + j) / sampleRate;
            int16_t sample = (int16_t)(sin(angle) * 32767 * volumeFactor);
            beepBuffer[j * 2] = sample & 0xFF;
            beepBuffer[j * 2 + 1] = (sample >> 8) & 0xFF;
        }
        i2s_write(I2S_NUM, beepBuffer, 512, &bytes_written, portMAX_DELAY);
    }
    
    i2s_zero_dma_buffer(I2S_NUM);
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            if (command == "list") {
                // ファイル一覧表示
                File root = SPIFFS.open("/");
                File file = root.openNextFile();
                Serial.println("\nWAVファイル一覧:");
                while (file) {
                    String filename = file.name();
                    if (filename.endsWith(".wav") || filename.endsWith(".WAV")) {
                        Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
                    }
                    file = root.openNextFile();
                }
                root.close();
            } else if (command.startsWith("vol ")) {
                // ボリューム設定
                int vol = command.substring(4).toInt();
                if (vol >= 0 && vol <= 100) {
                    volumePercent = vol;
                    Serial.printf("ボリューム設定: %d%%\n", volumePercent);
                } else {
                    Serial.println("ボリュームは0-100の範囲で指定してください");
                }
            } else if (command == "test") {
                // ボリュームテスト用ビープ音
                playBeep(1000, 500);  // 1kHz, 500ms
            } else {
                // ファイル再生
                if (!command.startsWith("/")) {
                    command = "/" + command;
                }
                if (!command.endsWith(".wav") && !command.endsWith(".WAV")) {
                    command += ".wav";
                }
                
                playWAV(command.c_str());
            }
        }
    }
    
    delay(100);
}