# Luaスクリプトシステム設計書

## 概要
UnityライクなLuaスクリプトシステムをUnoEngineに実装する。

## 要件
- **言語**: Lua 5.4
- **編集環境**: エディター内蔵 + 外部エディター両対応
- **対象**: 汎用スクリプト（任意のGameObjectに付けられる）
- **ホットリロード**: 必須（ファイル変更を検知して即時反映）
- **API範囲**: 中程度（Transform, Input, Time, Audio, Physics, UI）
- **ライフサイクル**: Unity風（Awake, Start, Update, OnDestroy）
- **Inspector連携**: public変数をInspectorに表示・編集可能
- **エラー処理**: 詳細（行番号、スタックトレース、エディター内表示）
- **ファイル配置**: assets/scripts
- **シーン保存**: 既存のscene.jsonに含める
- **C++標準**: C++20

## 実装フェーズ

### Phase 1: 最小限のLua実行環境
- [x] Luaライブラリ組み込み (external/lua/src)
- [x] sol2ライブラリ (external/sol.hpp)
- [x] LuaState管理クラス (Engine/Scripting/LuaState.h/.cpp)
- [x] 基本的なスクリプト読み込み・実行
- [x] print()関数でコンソール出力 (Logger経由)

### Phase 2: コンポーネント化
- [x] LuaScriptComponent (Engine/Scripting/LuaScriptComponent.h/.cpp)
- [x] GameObjectへのアタッチ
- [x] ライフサイクル関数呼び出し (Awake/Start/Update/OnDestroy)

### Phase 3: エンジンAPI公開
- [x] Transform API (getPosition/setPosition/translate/getRotation/setRotation/getScale/setScale)
- [x] GameObject API (name, isActive, setActive)
- [x] Debug API (log/warn/error)
- [x] Vector3ヘルパー (new/zero/one/up/forward/right)
- [ ] Input API (Phase3.5で追加予定)
- [ ] Time API (deltaTimeは渡している)

### Phase 4: エディター統合
- [x] Inspector表示 (スクリプトのpublic変数)
- [x] プロパティ編集 (bool/int/float/string)
- [x] ホットリロード (ファイル監視)
- [x] エラー表示 (エディター内)
- [ ] 内蔵コードエディター (ImGui) - 後で追加
- [ ] SceneSerializer対応 - 後で追加

## ディレクトリ構造
```
Engine/
  Scripting/
    LuaState.h/.cpp          # Lua状態管理
    LuaScriptComponent.h/.cpp # スクリプトコンポーネント
    LuaBindings.h/.cpp        # エンジンAPIバインディング
    ScriptProperty.h          # Inspector用プロパティ定義

assets/
  scripts/
    example.lua              # サンプルスクリプト
```

## Luaスクリプト例
```lua
-- CameraController.lua
-- public変数（Inspectorに表示される）
moveSpeed = 5.0
mouseSensitivity = 1.4

-- ローカル変数
local yaw = 0
local pitch = 0

function Awake()
    print("CameraController initialized")
end

function Start()
    -- 初期化処理
end

function Update(deltaTime)
    -- 入力取得
    local horizontal = Input.GetAxis("Horizontal")
    local vertical = Input.GetAxis("Vertical")
    
    -- 移動
    local movement = Vector3.new(horizontal, 0, vertical) * moveSpeed * deltaTime
    transform:Translate(movement)
end

function OnDestroy()
    print("CameraController destroyed")
end
```

## 技術的な注意点
- sol2ライブラリを使用してC++とLuaをバインド（型安全）
- スクリプトごとに独立したLua環境（サンドボックス）
- エラー時はpcallでキャッチしてエディターに表示
- ホットリロードはファイルのタイムスタンプを監視
