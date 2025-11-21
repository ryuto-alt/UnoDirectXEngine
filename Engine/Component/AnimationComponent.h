#pragma once

#include "../Core/Component.h"
#include "../Rendering/Animation.h"
#include "../Rendering/Skeleton.h"
#include "../Core/Types.h"
#include <memory>
#include <vector>

namespace UnoEngine {

/// アニメーション再生コンポーネント
class AnimationComponent : public Component {
public:
    AnimationComponent() = default;
    ~AnimationComponent() override = default;

    // Component interface
    void OnUpdate(float deltaTime) override;

    /// アニメーションとスケルトンを設定
    void SetAnimation(Animation* animation, Skeleton* skeleton);

    /// アニメーション再生制御
    void Play();
    void Pause();
    void Stop();
    void SetLoop(bool loop) { isLooping_ = loop; }

    /// 再生時間を設定 (秒)
    void SetTime(float time);

    /// 再生速度を設定 (1.0 = 通常速度)
    void SetPlaybackSpeed(float speed) { playbackSpeed_ = speed; }

    /// 時間を進める (AnimationSystemから呼ばれる)
    void UpdateTime(float deltaTime);

    /// 現在のアニメーション時間から各ジョイントの変換を更新
    void UpdateJointTransforms();

    /// 現在のジョイント行列を取得 (GPU送信用)
    const std::vector<Matrix4x4>& GetJointMatrices() const { return jointMatrices_; }

    /// 再生状態の取得
    bool IsPlaying() const { return isPlaying_; }
    float GetCurrentTime() const { return currentTime_; }
    Skeleton* GetSkeleton() const { return skeleton_; }

private:
    Animation* animation_ = nullptr;
    Skeleton* skeleton_ = nullptr;

    bool isPlaying_ = false;
    bool isLooping_ = true;
    float currentTime_ = 0.0f;
    float playbackSpeed_ = 1.0f;

    std::vector<Matrix4x4> jointMatrices_;

    /// 指定時間における特定チャンネルの値を補間して取得
    Vector3 SampleVector3(const AnimationSampler& sampler, float time) const;
    Quaternion SampleQuaternion(const AnimationSampler& sampler, float time) const;
};

} // namespace UnoEngine
