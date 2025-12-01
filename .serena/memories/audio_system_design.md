# UnoEngine Audio System Design

## 基本仕様
- **配置**: Engine/Audio（コア機能）
- **API**: XAudio2
- **フォーマット**: WAVのみ
- **同時再生数**: 32音

## コンポーネント構成

### 1. AudioSystem
- XAudio2エンジンの初期化・管理
- マスターボイスの管理
- ISystemインターフェースを実装

### 2. AudioClip
- WAVファイルをロードするアセット
- 事前ロード方式（シーン開始時にメモリにロード）
- PCMデータとフォーマット情報を保持

### 3. AudioSource（コンポーネント）
- GameObjectにアタッチして音を再生
- 機能:
  - 再生/停止/一時停止
  - 音量調整
  - ループ再生
  - PlayOnAwakeフラグ（シーン開始時に自動再生）
  - 3Dオーディオ（逆二乗減衰）

### 4. AudioListener（コンポーネント）
- 3D音響のリスナー位置
- カメラとは別のコンポーネントとして実装

## 3Dオーディオ
- 減衰モデル: 逆二乗減衰
- リスナー位置との距離で音量計算

## エディタUI
- インスペクター: ファイル選択（ダイアログ）+ 音量/ループ設定 + プレビューボタン
- プレビュー: フル音量で再生（3Dシミュレートなし）
- シリアライズ: 既存の仕組みに従う

## Pause連動
- ゲームビュー一時停止時に音声も一時停止

## 実装順序
1. AudioSystem（XAudio2初期化）
2. AudioClip（WAVロード）
3. AudioSource コンポーネント
4. AudioListener コンポーネント
5. エディタUI（インスペクター + プレビュー）

## ファイル構成（予定）
```
Engine/Audio/
├── AudioSystem.h/cpp      - XAudio2管理
├── AudioClip.h/cpp        - WAVデータ
├── AudioSource.h/cpp      - 音源コンポーネント
└── AudioListener.h/cpp    - リスナーコンポーネント
```
