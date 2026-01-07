# NavMesh System Architecture

## 要件
- **用途**: AIナビゲーション
- **地形**: 複雑な3D（洞窟、橋、複数階層）
- **動的障害物**: 不要（静的NavMesh）
- **入力ソース**: シーン内コライダー
- **階層接続**: 高さ差許容範囲で自動接続
- **可視化**: 詳細表示（ポリゴン・接続・高さ）
- **生成**: エディターでベイク
- **保存形式**: バイナリ (.navmesh)
- **アルゴリズム**: A*
- **パラメータ**: グローバル設定

## コンポーネント構成

### 1. NavMeshConfig
エージェントパラメータのグローバル設定
- agentRadius: エージェント半径
- agentHeight: エージェント高さ
- maxSlope: 最大登坂角度（度）
- stepHeight: 登れる段差の高さ
- cellSize: ボクセル化のセルサイズ
- cellHeight: ボクセル化の高さ分解能

### 2. NavMeshBuilder
コライダーからNavMesh生成（Recastアルゴリズム参考）
1. ボクセル化: コライダーをボクセルグリッドに変換
2. 歩行可能領域抽出: 傾斜・高さでフィルタリング
3. 領域分割: 連続領域をリージョンに分割
4. 輪郭抽出: リージョンの境界を抽出
5. ポリゴン生成: 輪郭を三角形/凸ポリゴンに分割
6. 隣接情報構築: ポリゴン間の接続関係を計算

### 3. NavMesh (データ構造)
```cpp
struct NavMeshPolygon {
    std::vector<uint32_t> vertexIndices;  // 頂点インデックス
    std::vector<uint32_t> neighbors;       // 隣接ポリゴンID
    DirectX::XMFLOAT3 center;             // 重心
    float area;                            // 面積
};

struct NavMeshData {
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<NavMeshPolygon> polygons;
    DirectX::BoundingBox bounds;
};
```

### 4. NavMeshQuery
A*によるパス検索
- FindPath(start, goal) -> std::vector<XMFLOAT3>
- FindNearestPolygon(point) -> polygonId
- IsPointOnNavMesh(point) -> bool

### 5. NavMeshRenderer (エディター用)
- ワイヤーフレーム表示
- ポリゴン塗りつぶし表示
- 接続線表示
- 高さカラーマップ

## ファイル構成
```
Engine/
├── AI/
│   ├── NavMesh/
│   │   ├── NavMeshConfig.h
│   │   ├── NavMeshData.h
│   │   ├── NavMeshBuilder.h / .cpp
│   │   ├── NavMeshQuery.h / .cpp
│   │   ├── NavMeshSerializer.h / .cpp
│   │   └── NavMeshRenderer.h / .cpp
```

## 実装フェーズ（完了）
1. ✅ 基本データ構造 (NavMeshTypes.h)
2. ✅ ビルダー (NavMeshBuilder.h/.cpp) - ボクセル化 → 領域分割 → 輪郭抽出 → ポリゴン生成
3. ✅ A*パス検索 (NavMeshQuery.h/.cpp)
4. ✅ バイナリシリアライザ (NavMeshSerializer.h/.cpp)
5. ✅ システム統合 (NavMeshSystem.h/.cpp)
6. ✅ エディターUI統合 (EditorUI.cpp) - ナビゲーションメニュー、設定ウィンドウ、デバッグ描画

## Recast/Detour版（ベイク機能実装済み）

### ファイル構成
```
Engine/Navigation/
├── NavMeshBuildSettings.h   - ビルド設定構造体
├── NavMeshManager.h         - メインマネージャヘッダ
└── NavMeshManager.cpp       - フル実装（ビルド・クエリ・I/O・DebugDraw）

Engine/Graphics/
└── Mesh.h/cpp               - CPU側頂点/インデックス保持（NavMesh用）
```

### 特徴
- Recast/Detourライブラリを使用（`external/recast/`）
- MeshクラスにCPU側データ保持機能を追加
- EditorUIからのベイク実行
- ビルド後、自動的にワイヤーフレーム表示

### ベイクフロー
1. EditorUI::BakeNavMesh()を呼び出し
2. シーン内の全GameObjectからMeshRendererを取得
3. Mesh::GetVertices()/GetIndices()でCPUデータを取得
4. StaticGeometryに変換してNavMeshManager::BuildNavMesh()
5. ビルド成功時、showRecastNavMesh_をtrueに設定
6. PrepareSceneViewGizmos()でDebugDraw()を毎フレーム呼び出し

### 使用方法（エディター）
- メニュー「ナビゲーション」→「ベイク」(Ctrl+B)
- 設定ウィンドウの「ベイク」ボタン
- ビルド後、自動的にワイヤーフレーム表示

### 使用方法（コード）
```cpp
auto& navMesh = UnoEngine::Navigation::NavMeshManager::Get();
navMesh.Initialize();

std::vector<Navigation::StaticGeometry> geometry;
// シーンからジオメトリを収集...

NavMeshBuildSettings settings;
settings.cellSize = 0.1f;
settings.agentRadius = 0.5f;

navMesh.BuildNavMesh(geometry, settings);

std::vector<DirectX::XMFLOAT3> path;
navMesh.FindPath(start, goal, path);
```

---

## NavAgentコンポーネント（新規追加）

### 概要
GameObjectにアタッチ可能なナビゲーションエージェント。NavMesh上を自動でパス追従移動する。

### ファイル
```
Engine/Navigation/
├── NavAgentComponent.h
└── NavAgentComponent.cpp
```

### プロパティ
- `speed`: 移動速度 (m/s)、デフォルト 3.5
- `angularSpeed`: 回転速度 (deg/s)、デフォルト 360
- `acceleration`: 加速度 (m/s²)、デフォルト 8.0
- `stoppingDistance`: 停止距離 (m)、デフォルト 0.1
- `baseOffset`: 地面からのオフセット
- `autoBrake`: 到着時自動減速

### 状態
- `Idle`: 待機中
- `Moving`: 移動中
- `Arrived`: 到着

### 使用例（Luaから）
```lua
-- エージェントを取得
local agent = gameObject:GetComponent("NavAgent")

-- 目的地を設定
agent:SetDestination({x = 10, y = 0, z = 5})

-- 状態確認
if agent:HasReachedDestination() then
    print("到着!")
end
```

---

## エディターUI（更新）

### インスペクタータブシステム
- **オブジェクトタブ**: 選択オブジェクトのプロパティ、コンポーネント表示
- **NavMeshタブ**: ビルド設定、表示設定、統計情報

### 自動タブ切り替え
- NavMeshベイク成功時: 自動的にNavMeshタブを選択
- メニュー「ナビゲーション」→「設定...」: NavMeshタブを選択

### NavAgentコンポーネントUI
インスペクターのオブジェクトタブで表示:
- 移動/回転速度の調整
- 加速度、停止距離の設定
- 自動減速トグル
- 状態表示（Idle/Moving/Arrived）
- パス可視化トグル
- 「Add NavAgent」ボタンでコンポーネント追加

---

## 使用方法
### エディターから
- メニュー「ナビゲーション」→「NavMeshをベイク」
- 「NavMesh設定」でパラメータ調整
- 「NavMeshを表示」で可視化

### コードから
```cpp
auto& navMesh = NavMeshSystem::GetInstance();
navMesh.BakeNavMesh(scene);

NavMeshPath path = navMesh.FindPath(startPos, goalPos);
for (const auto& wp : path.waypoints) {
    // ウェイポイントを使用
}
```
