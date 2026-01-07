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

## 非同期ベイク処理（実装済み）

### 概要
NavMeshベイク中のUIフリーズを解消するため、非同期処理を実装。

### 実装詳細
- `std::async` でベイク処理をバックグラウンド実行
- `std::atomic<float>` で進捗を管理
- `std::mutex` でステージ名の排他制御
- ベイク中はモーダルポップアップで進捗バーを表示
- 全UI操作をブロックして完了を待機

### 関連メンバー変数（EditorUI.h）
```cpp
std::atomic<float> navMeshBakeProgress_{0.0f};
std::string navMeshBakeStage_;
std::mutex navMeshBakeMutex_;
std::future<bool> navMeshBakeFuture_;
std::atomic<bool> navMeshBaking_{false};
```

---

## NavAgent可視化（実装済み）

### 概要
NavAgentコンポーネントを持つオブジェクトに対し、エージェントの半径と高さを円柱で描画。

### 描画内容
- 底面・上面に24分割の円（agentRadius）
- 4本の縦線で円柱を表現（agentHeight）
- 選択中は黄色、通常はシアン
- パス移動中はパスラインも緑で表示

### 設定値の参照
NavMeshBuildSettingsの`agentRadius`と`agentHeight`を使用し、リアルタイムで反映。

### NavMeshタブのNavAgent UI
- 選択オブジェクトへのNavAgent追加/削除
- 移動速度、回転速度、停止距離の設定

---

## DetourCrowd統合（新規実装）

### 概要
DetourCrowdライブラリを統合し、複数エージェントの衝突回避と滑らかなステアリングを実現。
狭い迷路でも高精度なナビゲーションが可能。

### NavMeshManager拡張
```cpp
// Crowd初期化（NavMeshビルド後に呼び出し）
navMesh.InitializeCrowd(128, 0.6f);

// エージェント追加
int agentId = navMesh.AddCrowdAgent(position, radius, height, speed, acceleration);

// 目的地設定
navMesh.SetAgentTarget(agentId, targetPos);

// 毎フレーム更新
navMesh.UpdateCrowd(deltaTime);

// ランダムポイント取得（徘徊用）
DirectX::XMFLOAT3 randomPoint;
navMesh.GetRandomPointOnNavMesh(randomPoint);
navMesh.GetRandomPointAroundCircle(center, radius, randomPoint);
```

### 障害物回避パラメータ（狭い通路向け最適化）
```cpp
params.velBias = 0.4f;
params.weightDesVel = 2.0f;
params.weightCurVel = 0.75f;
params.weightSide = 0.75f;
params.weightToi = 2.5f;
params.horizTime = 2.5f;
params.gridSize = 33;
params.adaptiveDivs = 7;
params.adaptiveRings = 2;
params.adaptiveDepth = 5;
```

---

## NavAgentComponent拡張機能

### 新しい状態
- `Idle`: 待機中
- `Moving`: 移動中
- `Arrived`: 到着
- `Wandering`: 徘徊中
- `Patrolling`: パトロール中
- `Chasing`: 追跡中

### 徘徊（Wander）
```cpp
// スポーン地点周辺で徘徊
agent->StartWander(WanderMode::AroundSpawn, 10.0f);

// 完全ランダム
agent->StartWander(WanderMode::Random);

// 現在位置周辺
agent->StartWander(WanderMode::AroundCurrent, 5.0f);

agent->StopWander();
```

### パトロール（Patrol）
```cpp
std::vector<DirectX::XMFLOAT3> points = {{0,0,0}, {10,0,0}, {10,0,10}};
agent->StartPatrol(points, true); // loop = true
agent->StopPatrol();
```

### 追跡（Chase）
```cpp
agent->StartChase(targetGameObject, 0.5f); // 0.5秒間隔で更新
agent->StopChase();
```

---

## Luaスクリプトバインディング

### 基本操作
```lua
-- 目的地設定
NavAgent.setDestination(10, 0, 5)

-- 停止
NavAgent.stop()

-- 状態確認
local state = NavAgent.getState() -- "idle", "moving", "arrived", "wandering", "patrolling", "chasing"
local reached = NavAgent.hasReachedDestination()
local dist = NavAgent.getRemainingDistance()
local vx, vy, vz = NavAgent.getVelocity()
```

### 徘徊
```lua
-- スポーン地点周辺で徘徊（半径10m）
NavAgent.startWander(10)

-- 完全ランダム
NavAgent.startWanderRandom()

-- 現在位置周辺
NavAgent.startWanderAroundCurrent(5)

NavAgent.stopWander()
local isWandering = NavAgent.isWandering()
```

### パトロール
```lua
-- パトロールポイント追加
NavAgent.addPatrolPoint(0, 0, 0)
NavAgent.addPatrolPoint(10, 0, 0)
NavAgent.addPatrolPoint(10, 0, 10)

-- パトロール開始（ループ）
NavAgent.startPatrol(true)

NavAgent.stopPatrol()
```

### プロパティ
```lua
NavAgent.setSpeed(5.0)
NavAgent.setAngularSpeed(360.0)
NavAgent.setStoppingDistance(0.5)
NavAgent.setWaitTime(2.0) -- 到着後の待機時間
```

---

## スレッドセーフティとNavMesh再ベイク対応（重要）

### 問題と修正

#### 1. Use-After-Free問題
**症状**: NavMesh再ベイク後にCrowd更新でクラッシュ（`dtNavMesh::getTileAndPolyByRef`で`m_tiles`が無効）

**原因**: 
- `BuildNavMesh()`が`m_navMesh`を破棄・再作成
- しかし`m_crowd`は古いNavMeshへの内部参照を保持し続ける
- Crowd更新時に解放済みメモリにアクセス

**修正** (`NavMeshManager.cpp`):
```cpp
// BuildNavMesh()内で、NavMesh破棄前にCrowdも破棄
if (m_crowd) {
    dtFreeCrowd(m_crowd);
    m_crowd = nullptr;
}
if (m_navMesh) {
    dtFreeNavMesh(m_navMesh);
    m_navMesh = nullptr;
}
```

#### 2. 非同期ベイク中のレース条件
**症状**: 非同期ベイク中にCrowd更新が実行されクラッシュ

**修正**:
- `NavMeshManager`に`m_isBuilding`フラグ追加
- `SetBuilding(bool)`/`IsBuilding()`メソッド追加
- `EditorUI::BakeNavMesh()`でベイク開始時に`SetBuilding(true)`、完了時に`SetBuilding(false)`
- `Scene::OnUpdate()`でCrowd更新前に`IsBuilding()`チェック

```cpp
// Scene::OnUpdate()
if (navMesh.IsCrowdInitialized() && !navMesh.IsBuilding()) {
    navMesh.UpdateCrowd(deltaTime);
}
```

#### 3. エージェント再初期化
**症状**: NavMesh再ベイク後、エージェントが動かない

**修正** (`NavAgentComponent::OnUpdate()`):
```cpp
// ビルド中は処理スキップ
if (navMesh.IsBuilding()) {
    return;
}

// Crowdが破棄された場合、エージェントをリセット
if (crowdAgentIndex_ >= 0 && !navMesh.IsCrowdInitialized()) {
    Logger::Info("[NavAgent] Crowd was reset, re-initializing agent...");
    crowdAgentIndex_ = -1;
    hasInitialDestination_ = false;
}
```

### Crowd更新の正しい場所
- **間違い**: 各`NavAgentComponent::OnUpdate()`でCrowd更新
- **正解**: `Scene::OnUpdate()`でコンポーネント更新前に一度だけ呼び出し

```cpp
// Scene::OnUpdate() - コンポーネント更新の前
auto& navMesh = Navigation::NavMeshManager::Get();
if (navMesh.IsCrowdInitialized() && !navMesh.IsBuilding()) {
    navMesh.UpdateCrowd(deltaTime);
}
```

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
