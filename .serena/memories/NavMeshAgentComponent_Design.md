# NavMeshAgentComponent 設計書

## 設計思想
- **Unity方式**: NavMeshAgentがTransformを直接書き換え
- **UnoEngine方式**: NavMeshAgentは「意図」を出力、移動は別コンポーネントが担当
- **Separation of Concerns（関心の分離）** を重視

## コンポーネント構成

```
NavMeshAgentComponent          CharacterController / Rigidbody
├─ パス計算                     ├─ 実際の移動処理
├─ desiredVelocity 出力  ───→  ├─ 物理/コリジョン考慮
└─ steeringDirection 出力 ───→ └─ Transform更新
                                       │
              ◄────────────────────────┘
              Feedback Loop (位置同期)
```

## フィードバックループ（重要）

NavMeshAgentは「次にどう動くべきか」を計算するために、「今どこにいるか」を正確に知る必要がある。
物理移動によってズレた位置を、毎フレームNavMeshAgent側に書き戻すフローが必要。

### 同期タイミング
1. フレーム開始時: NavMeshAgentが現在位置を取得（SyncPosition）
2. パス追従計算: desiredVelocity, steeringDirection を算出
3. 物理更新: CharacterController等が実際に移動
4. 次フレーム: 1に戻る

## NavMeshAgentComponent クラス設計

```cpp
class NavMeshAgentComponent : public Component {
    // === エージェント設定 ===
    uint32_t m_agentTypeId = 0;
    float m_radius = 0.5f;
    float m_height = 2.0f;
    
    // === 移動パラメータ ===
    float m_maxSpeed = 3.5f;
    float m_acceleration = 8.0f;
    float m_angularSpeed = 120.0f;  // degrees/sec
    float m_stoppingDistance = 0.1f;
    float m_autoBraking = true;
    
    // === 内部状態 ===
    NavMeshPath m_currentPath;
    XMFLOAT3 m_currentPosition;     // 同期された現在位置
    int m_currentWaypointIndex = 0;
    bool m_hasPath = false;
    bool m_isMoving = false;
    
    // === 出力（他コンポーネントが参照） ===
    XMFLOAT3 m_desiredVelocity;     // 希望移動ベクトル
    XMFLOAT3 m_steeringDirection;   // 向くべき方向
    float m_currentSpeed = 0.0f;
    
    // === API ===
public:
    // 目的地設定
    bool SetDestination(const XMFLOAT3& target);
    void Stop();
    void ResetPath();
    
    // 状態取得
    bool HasPath() const;
    bool IsMoving() const;
    float GetRemainingDistance() const;
    const XMFLOAT3& GetDesiredVelocity() const;
    const XMFLOAT3& GetSteeringDirection() const;
    
    // フィードバックループ（毎フレーム呼び出し）
    void SyncPosition(const XMFLOAT3& worldPosition);
    void UpdatePathFollowing(float deltaTime);
    
    // NavMesh上に位置を補正
    bool SnapToNavMesh();
};
```

## Inspector UI 設計

NavMesh設定ウィンドウのInspectorで以下を表示:
- Agent Type (ドロップダウン)
- Radius / Height
- Speed / Acceleration / Angular Speed
- Stopping Distance
- Auto Braking (チェックボックス)
- 現在の状態表示（HasPath, RemainingDistance等）

## 使用例（スクリプト側）

```cpp
void EnemyAI::Update(float deltaTime) {
    auto* agent = GetComponent<NavMeshAgentComponent>();
    auto* controller = GetComponent<CharacterController>();
    
    // 1. 位置同期（フィードバック）
    agent->SyncPosition(GetTransform()->GetWorldPosition());
    
    // 2. パス追従更新
    agent->UpdatePathFollowing(deltaTime);
    
    // 3. 意図を取得して移動
    if (agent->IsMoving()) {
        XMFLOAT3 velocity = agent->GetDesiredVelocity();
        controller->Move(velocity * deltaTime);
        
        // 回転
        XMFLOAT3 dir = agent->GetSteeringDirection();
        GetTransform()->LookAt(dir);
    }
}
```

## 将来の拡張
- Obstacle Avoidance (回避行動)
- Off-Mesh Links (ジャンプ、はしご等)
- Area Mask (特定エリアの通行可否)
- Agent Priority (混雑時の優先度)
