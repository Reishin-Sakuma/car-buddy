@echo off
echo CarBuddy Python版 - Windows デバッグモード
echo ==========================================

REM 現在のディレクトリを確認
echo 現在のディレクトリ: %CD%

REM Python pythonディレクトリに移動
cd /d "%~dp0python"

REM 依存関係をインストール（初回のみ）
echo.
echo Python依存関係をインストール中...
pip install -r ..\requirements_windows.txt

REM デバッグ用アプリケーション起動
echo.
echo デバッグアプリケーションを起動中...
echo ==========================================
echo 🔧 Windows デバッグモードで実行
echo 📊 温度: 自動変化（15°C〜40°C）
echo 🖼️ 画像フォルダー: ..\images\
echo ⌨️ Escキー: フルスクリーン解除
echo ❌ ウィンドウ閉じる: アプリ終了
echo ==========================================
echo.

python main_debug.py

echo.
echo アプリケーションが終了しました。
pause