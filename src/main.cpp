#include <Arduino.h>

// PAM8403テスト用定義
#define AUDIO_PIN 26           // GPIO26をPAM8403のLのINに接続
#define TEST_FREQUENCY 1000    // 1kHzのテスト音
#define TEST_DURATION 2000     // 2秒間のテスト

// DAC関連
#define DAC_RESOLUTION 8       // 8-bit DAC
#define DAC_MAX_VALUE 255

// 音声テスト用変数
bool audioInitialized = false;
unsigned long lastTestTime = 0;
const unsigned long TEST_INTERVAL = 5000; // 5秒間隔でテスト

// PAM8403初期化
bool initPAM8403() {
  Serial.println("===================");
  Serial.println("PAM8403 認識テスト開始");
  Serial.println("===================");
  
  // GPIO26がDAC対応ピンか確認
  if (AUDIO_PIN != 25 && AUDIO_PIN != 26) {
    Serial.println("✗ GPIO26はDAC対応ピンではありません");
    return false;
  }
  
  // GPIO26をDACピンとして設定
  dacWrite(AUDIO_PIN, 0);
  Serial.println("✓ GPIO26 DAC設定成功");
  
  // 電源供給確認のため、中間レベル出力
  dacWrite(AUDIO_PIN, DAC_MAX_VALUE / 2);
  delay(100);
  dacWrite(AUDIO_PIN, 0);
  
  Serial.println("✓ PAM8403電源供給テスト完了");
  
  audioInitialized = true;
  return true;
}

// テスト音生成（1kHz正弦波）
void playTestTone(int frequency, int duration) {
  if (!audioInitialized) {
    Serial.println("✗ 音声未初期化");
    return;
  }
  
  Serial.print("🔊 テスト音再生中... ");
  Serial.print(frequency);
  Serial.print("Hz, ");
  Serial.print(duration);
  Serial.println("ms");
  
  unsigned long startTime = millis();
  unsigned long sampleCount = 0;
  
  // 8kHzサンプリングレートで音声生成
  const float sampleRate = 8000.0;
  const float period = sampleRate / frequency;
  
  while (millis() - startTime < duration) {
    // 正弦波生成（0-255範囲）
    float phase = (sampleCount % (int)period) / period;
    int sineValue = (sin(2.0 * PI * phase) + 1.0) * 127.5;
    
    // DAC出力
    dacWrite(AUDIO_PIN, sineValue);
    
    sampleCount++;
    
    // サンプリング間隔調整（125μs = 8kHz）
    delayMicroseconds(125);
  }
  
  // 音声停止
  dacWrite(AUDIO_PIN, DAC_MAX_VALUE / 2);
  delay(50);
  dacWrite(AUDIO_PIN, 0);
  
  Serial.println("✓ テスト音再生完了");
}

// 電源供給確認
void checkPowerSupply() {
  Serial.println("🔌 PAM8403電源供給確認");
  
  // 段階的に出力レベルを上げて応答確認
  for (int level = 0; level <= DAC_MAX_VALUE; level += 51) {
    Serial.print("出力レベル: ");
    Serial.print(level);
    Serial.print("/");
    Serial.println(DAC_MAX_VALUE);
    
    dacWrite(AUDIO_PIN, level);
    delay(200);
  }
  
  // 0に戻す
  dacWrite(AUDIO_PIN, 0);
  Serial.println("✓ 電源供給確認完了");
}

// 詳細ハードウェア情報表示
void showHardwareInfo() {
  Serial.println("\n📋 ハードウェア設定情報");
  Serial.println("========================");
  Serial.print("ESP32音声出力ピン: GPIO");
  Serial.println(AUDIO_PIN);
  Serial.print("DAC解像度: ");
  Serial.print(DAC_RESOLUTION);
  Serial.println("-bit");
  Serial.print("最大DAC値: ");
  Serial.println(DAC_MAX_VALUE);
  Serial.println("期待される接続:");
  Serial.println("  ESP32 GPIO26 → PAM8403 LのIN");
  Serial.println("  ESP32 5V     → PAM8403 電源+");
  Serial.println("  ESP32 GND    → PAM8403 電源-");
  Serial.println("  ESP32 GND    → PAM8403 入力GND");
  Serial.println("========================\n");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n🎵 PAM8403音声アンプ認識テスト");
  Serial.println("================================");
  
  // ハードウェア情報表示
  showHardwareInfo();
  
  // PAM8403初期化
  if (initPAM8403()) {
    Serial.println("✅ PAM8403初期化成功！");
    
    // 電源供給確認
    checkPowerSupply();
    
    // 初回テスト音
    delay(1000);
    playTestTone(TEST_FREQUENCY, TEST_DURATION);
    
  } else {
    Serial.println("❌ PAM8403初期化失敗");
    Serial.println("配線を確認してください");
  }
  
  Serial.println("\n⏰ 定期テスト開始（5秒間隔）");
  lastTestTime = millis();
}

void loop() {
  // 5秒間隔でテスト音再生
  if (audioInitialized && (millis() - lastTestTime > TEST_INTERVAL)) {
    Serial.println("\n--- 定期テスト ---");
    
    // 異なる周波数でテスト
    static int testFrequencies[] = {500, 1000, 1500, 2000};
    static int freqIndex = 0;
    
    int currentFreq = testFrequencies[freqIndex];
    playTestTone(currentFreq, 1000); // 1秒間
    
    freqIndex = (freqIndex + 1) % 4;
    lastTestTime = millis();
    
    // メモリ使用量表示
    Serial.print("💾 空きメモリ: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
  }
  
  // シリアルコマンド処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "test") {
      Serial.println("🎵 手動テスト実行");
      playTestTone(1000, 2000);
    } 
    else if (command == "power") {
      checkPowerSupply();
    }
    else if (command == "info") {
      showHardwareInfo();
    }
    else if (command == "help") {
      Serial.println("📖 利用可能コマンド:");
      Serial.println("  test  - テスト音再生");
      Serial.println("  power - 電源確認");
      Serial.println("  info  - ハードウェア情報");
      Serial.println("  help  - このヘルプ");
    }
    else {
      Serial.println("❓ 不明なコマンド。'help'でコマンド一覧を表示");
    }
  }
}