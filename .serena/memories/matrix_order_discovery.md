# 行列の順序の発見

## prohの行列計算順序 (Object3d.cpp)
```cpp
// Line 856: ローカル行列の作成
joint.localMatrix = MakeAffineMatrix(scale, rotate, translate);

// Line 859: グローバル行列の計算
joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skeleton.joints[*joint.parent].skeletonSpaceMatrix);
// つまり: Global = Local * Parent

// Line 896: 最終スキニング行列
skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix = 
    Multiply(skinCluster.inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
// つまり: FinalMatrix = InverseBindPose * GlobalTransform
```

## UnoEngineに適用した変更
Skeleton.cppで以下の順序に変更:
- `globalTransforms[i] = localTransforms[i] * globalTransforms[bone.parentIndex]` (Local * Parent)
- `outBoneMatrices[i].skeletonSpaceMatrix = bone.offsetMatrix * globalTransforms[i]` (InverseBindPose * Global)

## 注意
DirectXMathはcolumn-majorだが、prohの順序をそのまま適用することで正しい結果が得られる可能性がある。
