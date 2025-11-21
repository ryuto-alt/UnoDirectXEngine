#include "AnimationComponent.h"
#include "../Core/GameObject.h"
#include <algorithm>

namespace UnoEngine {

void AnimationComponent::OnUpdate(float deltaTime) {
    if (isPlaying_ && animation_) {
        UpdateTime(deltaTime);
        UpdateJointTransforms();
    }
}

void AnimationComponent::SetAnimation(Animation* animation, Skeleton* skeleton) {
    animation_ = animation;
    skeleton_ = skeleton;
    currentTime_ = 0.0f;
    isPlaying_ = false;

    if (skeleton_) {
        jointMatrices_.resize(skeleton_->GetJointCount());
    }
}

void AnimationComponent::Play() {
    isPlaying_ = true;
}

void AnimationComponent::Pause() {
    isPlaying_ = false;
}

void AnimationComponent::Stop() {
    isPlaying_ = false;
    currentTime_ = 0.0f;
}

void AnimationComponent::SetTime(float time) {
    currentTime_ = time;

    if (animation_) {
        float duration = animation_->GetDuration();
        if (isLooping_ && duration > 0.0f) {
            // ループ処理
            while (currentTime_ >= duration) {
                currentTime_ -= duration;
            }
            while (currentTime_ < 0.0f) {
                currentTime_ += duration;
            }
        } else {
            // クランプ
            currentTime_ = std::max(0.0f, std::min(currentTime_, duration));
        }
    }
}

void AnimationComponent::UpdateTime(float deltaTime) {
    if (!isPlaying_ || !animation_) {
        return;
    }

    currentTime_ += deltaTime * playbackSpeed_;
    SetTime(currentTime_); // ループ/クランプ処理を適用
}

void AnimationComponent::UpdateJointTransforms() {
    if (!animation_ || !skeleton_) {
        return;
    }

    // 各チャンネルを評価して、対応するジョイントの変換を更新
    for (const auto& channel : animation_->GetChannels()) {
        if (channel.samplerIndex < 0 || channel.samplerIndex >= static_cast<int32>(animation_->GetSamplers().size())) {
            continue;
        }

        const auto& sampler = animation_->GetSamplers()[channel.samplerIndex];

        if (channel.targetJointIndex < 0 || channel.targetJointIndex >= static_cast<int32>(skeleton_->GetJoints().size())) {
            continue;
        }

        auto& joints = const_cast<std::vector<Joint>&>(skeleton_->GetJoints());
        auto& joint = joints[channel.targetJointIndex];

        switch (channel.path) {
            case AnimationPath::Translation:
                joint.translation = SampleVector3(sampler, currentTime_);
                break;

            case AnimationPath::Rotation:
                joint.rotation = SampleQuaternion(sampler, currentTime_);
                break;

            case AnimationPath::Scale:
                joint.scale = SampleVector3(sampler, currentTime_);
                break;
        }
    }

    // ジョイントのグローバル変換を計算
    skeleton_->ComputeGlobalTransforms();

    // GPU送信用のジョイント行列を計算
    skeleton_->ComputeJointMatrices(jointMatrices_);
}

Vector3 AnimationComponent::SampleVector3(const AnimationSampler& sampler, float time) const {
    const auto& times = sampler.times;
    const auto& outputs = sampler.outputVec3;

    if (times.empty() || outputs.empty()) {
        return Vector3::Zero();
    }

    // 時間が範囲外の場合
    if (time <= times.front()) {
        return outputs.front();
    }
    if (time >= times.back()) {
        return outputs.back();
    }

    // キーフレームを検索
    size_t nextIndex = 0;
    for (size_t i = 0; i < times.size(); ++i) {
        if (times[i] > time) {
            nextIndex = i;
            break;
        }
    }

    size_t prevIndex = (nextIndex > 0) ? nextIndex - 1 : 0;
    float t0 = times[prevIndex];
    float t1 = times[nextIndex];
    float t = (time - t0) / (t1 - t0); // 正規化された時間 [0, 1]

    if (sampler.interpolation == InterpolationType::Step) {
        return outputs[prevIndex];
    }
    else if (sampler.interpolation == InterpolationType::Linear) {
        // 線形補間
        return Vector3::Lerp(outputs[prevIndex], outputs[nextIndex], t);
    }
    else if (sampler.interpolation == InterpolationType::CubicSpline) {
        // CubicSpline補間
        // glTF仕様: 各キーフレームに対して (in-tangent, value, out-tangent) の3つの値が格納される
        size_t stride = 3;
        Vector3 p0 = outputs[prevIndex * stride + 1]; // 前のキーフレームの値
        Vector3 m0 = outputs[prevIndex * stride + 2]; // 前のキーフレームの出力タンジェント
        Vector3 p1 = outputs[nextIndex * stride + 1]; // 次のキーフレームの値
        Vector3 m1 = outputs[nextIndex * stride + 0]; // 次のキーフレームの入力タンジェント

        float dt = t1 - t0;
        float t2 = t * t;
        float t3 = t2 * t;

        // Hermite spline
        return (2.0f * t3 - 3.0f * t2 + 1.0f) * p0 +
               (t3 - 2.0f * t2 + t) * dt * m0 +
               (-2.0f * t3 + 3.0f * t2) * p1 +
               (t3 - t2) * dt * m1;
    }

    return Vector3::Zero();
}

Quaternion AnimationComponent::SampleQuaternion(const AnimationSampler& sampler, float time) const {
    const auto& times = sampler.times;
    const auto& outputs = sampler.outputQuat;

    if (times.empty() || outputs.empty()) {
        return Quaternion::Identity();
    }

    // 時間が範囲外の場合
    if (time <= times.front()) {
        return outputs.front();
    }
    if (time >= times.back()) {
        return outputs.back();
    }

    // キーフレームを検索
    size_t nextIndex = 0;
    for (size_t i = 0; i < times.size(); ++i) {
        if (times[i] > time) {
            nextIndex = i;
            break;
        }
    }

    size_t prevIndex = (nextIndex > 0) ? nextIndex - 1 : 0;
    float t0 = times[prevIndex];
    float t1 = times[nextIndex];
    float t = (time - t0) / (t1 - t0);

    if (sampler.interpolation == InterpolationType::Step) {
        return outputs[prevIndex];
    }
    else if (sampler.interpolation == InterpolationType::Linear) {
        // 球面線形補間 (Slerp)
        return Quaternion::Slerp(outputs[prevIndex], outputs[nextIndex], t);
    }
    else if (sampler.interpolation == InterpolationType::CubicSpline) {
        // CubicSpline補間 (Quaternionの場合は簡易実装としてSlerpを使用)
        // 本来はsquad (spherical quadrangle interpolation) を使うべきだが、
        // 初期実装としてはSlerpで代用
        size_t stride = 3;
        Quaternion q0 = outputs[prevIndex * stride + 1];
        Quaternion q1 = outputs[nextIndex * stride + 1];
        return Quaternion::Slerp(q0, q1, t);
    }

    return Quaternion::Identity();
}

} // namespace UnoEngine
