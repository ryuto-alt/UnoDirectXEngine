# UnoEngine Architecture Design v3 - Component & Renderer System

## Design Decisions Summary (2024年ヒアリング結果)

---

## 1. Component System

### Lifecycle (Unity風フル)
```cpp
class Component {
public:
    virtual void Awake() {}      // コンポーネント作成直後
    virtual void Start() {}      // 最初のUpdate前に一度だけ
    virtual void OnUpdate(float deltaTime) {}  // 毎フレーム
    virtual void OnDestroy() {}  // 破棄時
};
```

### Lifecycle Timing
- **Scene単位**: Scene::OnLoad()完了後に全コンポーネントのAwake→Start順次実行
- **OnDestroy**: RemoveComponent時 AND GameObject破棄時の両方で呼ぶ

---

## 2. Renderer System

### Class Hierarchy
```
Component
  └── MeshRendererBase (Material, BoundingBox/Sphere)
        ├── MeshRenderer (静的メッシュ用)
        └── SkinnedMeshRenderer (スキンメッシュ用)
                └── 密結合: AnimatorComponentを直接参照
```

### MeshRendererBase 共通機能
- Material* (オーバーライド可能)
- BoundingBox / BoundingSphere (カリング用)
- enabled フラグ

### SkinnedMeshRenderer ↔ AnimatorComponent
- **密結合**: SkinnedMeshRendererがAnimatorComponentを直接参照
- ボーン行列の取得はRenderer内で行う

### RenderSystem
- **シーン独立型**: RenderSystemがSceneを走査してRenderItem収集
- Sceneは描画を知らない（GetSkinnedRenderItems廃止）
- **描画順序**: マテリアル順でソート（バッチング最適化）

---

## 3. Resource Management

### ResourceManager
- **所有**: Applicationがインスタンス所有（シングルトンではない）
- **スコープ**: 全リソース（モデル、テクスチャ、マテリアル、シェーダー、アニメーション）
- **キャッシュ**: パスをキーにしたキャッシュ機構

### アニメーションクリップ
- **分離管理**: AnimationClipを独立アセットとして管理
- SkinnedModelからは分離可能

### マテリアル
- **オーバーライド可能**: glTF埋め込みをデフォルトとし、個別に上書き可能

---

## 4. Coordinate System

### glTFインポート時の座標系補正
- **インポーター設定**: ImportOptions構造体で回転を指定
- コードのみ（UIは将来のエディタで対応）

```cpp
struct ImportOptions {
    bool convertCoordinateSystem = true;  // Y-up to Z-up等
    float rotationX = 0.0f;  // 追加回転（度）
    float rotationY = 0.0f;
    float rotationZ = 0.0f;
};
```

---

## 5. System Management

### System所有
- **Applicationが全System所有**
- PlayerSystem, RenderSystem等はApplication内で管理

```cpp
class Application {
    std::unique_ptr<RenderSystem> renderSystem_;
    std::unique_ptr<PlayerSystem> playerSystem_;
    // ...
};
```

---

## 6. Error Handling & Logging

### エラーハンドリング
- **null + ログ**: 失敗時はnullptr返却 + ログ出力
- 例外は使わない

### ロギング
- **Loggerクラス**: 統一的なLoggerクラスを作成
```cpp
Logger::Info("Message");
Logger::Warning("Message");
Logger::Error("Message");
```

---

## 7. Project Structure

### テストコード移動
- Game/Scenes/GameScene → **Samples/**フォルダに移動
- テスト用と明示

### 既存MeshRenderer
- **段階的移行**: まずSkinnedだけ新方式、後でMeshRendererも移行

---

## 8. Future Considerations

### シリアライズ
- **将来的に必要**: 拡張しやすい設計にしておく
- JSONからのシーン読み込み対応予定

---

## Implementation Priority

1. **Component拡張** - Awake/Start/OnDestroyの追加
2. **Loggerクラス** - ログシステム整備
3. **ResourceManager** - 基盤作成
4. **MeshRendererBase** - 基底クラス作成
5. **SkinnedMeshRenderer** - コンポーネント化
6. **RenderSystem** - シーンからの分離
7. **ImportOptions** - 座標系補正機能
8. **Samples移動** - テストコード整理
