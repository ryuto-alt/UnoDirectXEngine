# 現在の行列計算順序まとめ

## proh (正しい実装)
1. **AnimationPlayer.cpp line 78**: `MakeAffineMatrix(scale, rotate, translate)` でローカル行列作成
2. **Object3d.cpp line 859**: `joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skeleton.joints[*joint.parent].skeletonSpaceMatrix)` つまり `Local * Parent`
3. **Object3d.cpp line 896**: `mappedPalette[jointIndex].skeletonSpaceMatrix = Multiply(skinCluster.inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix)` つまり `InverseBindPose * Global`
4. **Mymath.cpp line 27**: 行列乗算は`result.m[i][j] += m1.m[i][k] * m2.m[k][j]` (row-major)

## UnoEngine (現在の実装)
1. **AnimationClip.cpp line 76**: `CreateTranslation(position) * CreateFromQuaternion(rotation) * CreateScale(scale)` (DirectXMath column-major なので右から左: S→R→T)
2. **Skeleton.cpp line 122**: `globalTransforms[i] = localTransforms[i] * globalTransforms[bone.parentIndex]` つまり `Local * Parent`
3. **Skeleton.cpp line 126**: `outBoneMatrices[i].skeletonSpaceMatrix = bone.offsetMatrix * globalTransforms[i]` つまり `Offset * Global`
4. **DirectXMath**: 行列乗算は column-major

## 問題の可能性
- Renderer.cppでGPUにコピーする際にtransposeしていない（現在はDirectXMathをそのままコピー）
- SkinnedVS.hlslでの行列の使い方（mul(position, matrix) vs mul(matrix, position)）
