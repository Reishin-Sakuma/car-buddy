// =================================
// Car-Buddy シンプルMP3テストシステム
// GPIO26 DAC + PAM8403
// =================================

#include <Arduino.h>
#include <SPIFFS.h>
#include <Audio.h>  // ESP32-audioI2S ライブラリ

// 現在の配線をそのまま使用
#define I2S_DOUT  26  // GPIO26 → PAM8403 LのIN（現在の配線）
#define I2S_BCLK  25  // GPIO25（温度センサーピンを一時的に使用、または未使用）
#define I2S_LRC   33  // GPIO33（空きピン）

// 音声オブジェクト
Audio audio;

// 音声状態管理
bool audioInitialized = false;
bool isPlaying = false;
String currentFile = "";

// 音割れ対策版の初期化
bool initAudioSystem() {
  Serial.println("🎵 シンプルMP3システム初期化中...");
  
  // SPIFFS初期化
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS初期化失敗");
    return false;
  }
  
  Serial.println("✅ SPIFFS初期化成功");
  
  // 音声ファイル一覧表示
  Serial.println("📁 検出された音声ファイル:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int mp3Count = 0;
  
  while (file) {
    String fileName = String(file.name());
    if (fileName.endsWith(".mp3")) {
      Serial.print("  🎵 ");
      Serial.print(fileName);
      Serial.print(" (");
      Serial.print(file.size());
      Serial.println(" bytes)");
      mp3Count++;
    }
    file = root.openNextFile();
  }
  
  if (mp3Count == 0) {
    Serial.println("⚠️ MP3ファイルが見つかりません");
    Serial.println("📖 ファイルアップロード方法:");
    Serial.println("   1. PlatformIOのFile System Uploaderを使用");
    Serial.println("   2. dataフォルダにMP3ファイルを配置");
    Serial.println("   3. 'Upload Filesystem Image'を実行");
    return false;
  }
  
  // I2S音声出力初期化（音割れ対策）
  Serial.println("🔌 I2S音声出力初期化中...");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  
  // 音割れ対策設定
  audio.setVolume(5);        // 非常に低い音量から開始（0-21）
  audio.setTone(-40, -40, -40); // 高音域を下げる
  audio.forceMono(true);     // モノラル強制（音割れ軽減）
  
  Serial.println("✅ シンプルMP3システム初期化完了");
  Serial.println("⚠️ 音割れ対策: 低音量・モノラル設定");
  audioInitialized = true;
  
  return true;
}

// 音声ファイル存在確認
bool checkFile(const String& filename) {
  String fullPath = filename.startsWith("/") ? filename : "/" + filename;
  File file = SPIFFS.open(fullPath, "r");
  if (!file) {
    Serial.print("❌ ファイルが見つかりません: ");
    Serial.println(fullPath);
    return false;
  }
  file.close();
  return true;
}

// MP3再生（シンプル版）
bool playMP3Simple(const String& filename) {
  if (!audioInitialized) {
    Serial.println("❌ 音声システムが初期化されていません");
    return false;
  }
  
  String fullPath = filename.startsWith("/") ? filename : "/" + filename;
  
  if (!checkFile(fullPath)) {
    return false;
  }
  
  // 現在の再生を停止
  if (isPlaying) {
    audio.stopSong();
    delay(100);
  }
  
  Serial.print("🎵 再生開始: ");
  Serial.println(fullPath);
  
  // 新しい音声を再生
  if (audio.connecttoFS(SPIFFS, fullPath.c_str())) {
    isPlaying = true;
    currentFile = fullPath;
    Serial.println("✅ 再生開始成功");
    return true;
  } else {
    Serial.print("❌ 再生失敗: ");
    Serial.println(fullPath);
    return false;
  }
}

// 音声停止
void stopAudio() {
  if (audioInitialized && isPlaying) {
    audio.stopSong();
    isPlaying = false;
    currentFile = "";
    Serial.println("⏹️ 音声停止");
  }
}

// 音量調整（0-100%）- 音割れ対策版
void setVolume(int volumePercent) {
  if (!audioInitialized) return;
  
  volumePercent = constrain(volumePercent, 0, 100);
  
  // 音割れ対策：最大音量を制限
  int maxVolume = 15; // 21の約70%に制限
  int audioVolume = map(volumePercent, 0, 100, 0, maxVolume);
  audio.setVolume(audioVolume);
  
  Serial.print("🔊 音量設定: ");
  Serial.print(volumePercent);
  Serial.print("% (実際: ");
  Serial.print(audioVolume);
  Serial.print("/21, 制限値: ");
  Serial.print(maxVolume);
  Serial.println(")");
  
  if (volumePercent > 70) {
    Serial.println("⚠️ 高音量注意: 音割れの可能性があります");
  }
}

// 利用可能ファイル一覧表示
void listMP3Files() {
  Serial.println("📋 利用可能なMP3ファイル:");
  Serial.println("=========================");
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int count = 1;
  
  while (file) {
    String fileName = String(file.name());
    if (fileName.endsWith(".mp3")) {
      Serial.print(count);
      Serial.print(". ");
      Serial.print(fileName);
      Serial.print(" (");
      Serial.print(file.size() / 1024);
      Serial.println(" KB)");
      count++;
    }
    file = root.openNextFile();
  }
  
  if (count == 1) {
    Serial.println("❌ MP3ファイルがありません");
  }
  Serial.println("=========================");
}

// 音声状態確認
void checkAudioStatus() {
  if (!audioInitialized) {
    Serial.println("❌ 音声システム未初期化");
    return;
  }
  
  Serial.println("📊 音声システム状態:");
  Serial.print("  再生状態: ");
  if (isPlaying && audio.isRunning()) {
    Serial.println("再生中");
    Serial.print("  現在のファイル: ");
    Serial.println(currentFile);
  } else {
    Serial.println("停止中");
  }
  
  Serial.print("  音量: ");
  Serial.print(audio.getVolume() * 100 / 21);
  Serial.println("%");
  
  Serial.print("  SPIFFSの空き容量: ");
  Serial.print(SPIFFS.totalBytes() - SPIFFS.usedBytes());
  Serial.println(" bytes");
}

// 音声システムのメインループ
void audioLoop() {
  if (audioInitialized) {
    audio.loop();
    
    // 再生完了チェック
    if (isPlaying && !audio.isRunning()) {
      Serial.println("✅ 再生完了");
      isPlaying = false;
      currentFile = "";
    }
  }
}

// セットアップ
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("🎵 Car-Buddy シンプルMP3テストシステム");
  Serial.println("======================================");
  Serial.println("現在の配線: GPIO26 → PAM8403 LのIN");
  Serial.println("");
  
  if (initAudioSystem()) {
    Serial.println("🎉 初期化成功！");
    
    // ファイル一覧表示
    listMP3Files();
    
    Serial.println("\n📋 利用可能コマンド:");
    Serial.println("  'list'          - MP3ファイル一覧");
    Serial.println("  'play filename' - MP3再生 (例: play startup.mp3)");
    Serial.println("  'stop'          - 再生停止");
    Serial.println("  'vol XX'        - 音量設定 (0-100)");
    Serial.println("  'status'        - システム状態確認");
    Serial.println("  'test'          - 最初のMP3ファイルでテスト");
    Serial.println("");
    Serial.println("💡 ヒント: 'test'コマンドで動作確認してください");
    
  } else {
    Serial.println("❌ 初期化失敗");
    Serial.println("");
    Serial.println("🔧 トラブルシューティング:");
    Serial.println("  1. MP3ファイルがSPIFFSにアップロードされているか確認");
    Serial.println("  2. GPIO26とPAM8403の配線確認");
    Serial.println("  3. PAM8403の電源供給確認（ESP32の5Vピン）");
  }
}

// メインループ
void loop() {
  // 音声処理
  audioLoop();
  
  // シリアルコマンド処理
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "list") {
      listMP3Files();
    }
    else if (input.startsWith("play ")) {
      String filename = input.substring(5);
      playMP3Simple(filename);
    }
    else if (input == "stop") {
      stopAudio();
    }
    else if (input.startsWith("vol ")) {
      int volume = input.substring(4).toInt();
      setVolume(volume);
    }
    else if (input == "status") {
      checkAudioStatus();
    }
    else if (input == "test") {
      // 最初のMP3ファイルでテスト
      File root = SPIFFS.open("/");
      File file = root.openNextFile();
      while (file) {
        String fileName = String(file.name());
        if (fileName.endsWith(".mp3")) {
          Serial.print("🧪 テスト再生: ");
          Serial.println(fileName);
          playMP3Simple(fileName);
          break;
        }
        file = root.openNextFile();
      }
    }
    else if (input == "help") {
      Serial.println("📖 コマンドヘルプ:");
      Serial.println("  list           - ファイル一覧");
      Serial.println("  play <file>    - 再生");
      Serial.println("  stop           - 停止");
      Serial.println("  vol <0-100>    - 音量");
      Serial.println("  status         - 状態");
      Serial.println("  test           - テスト再生");
    }
    else if (input.length() > 0) {
      Serial.println("❓ 不明なコマンド。'help'でヘルプを表示");
    }
  }
  
  delay(10);
}