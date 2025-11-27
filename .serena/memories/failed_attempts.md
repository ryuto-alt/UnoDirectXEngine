# 失敗した試み - 描画結果が変わらなかったもの

## 行列構築順序の変更 (意味がない)
- `S * R * T` と `T * R * S` の順序変更
- AnimationClip.cppのGetLocalTransform関数での順序変更
- これらの変更では描画結果は変わらない

## 理由
- UnoEngineはDirectXMathの関数を直接使用している
- DirectXMathの内部実装が既に正しい順序で処理している
- 表面的な乗算順序を変えても意味がない

## 本当の問題
- Math library自体の実装を見る必要がある
- 特にCreateFromQuaternion, CreateScale, CreateTranslationの実装
- prohのMakeAffineMatrix, Lerp, Slerpの実装と比較する必要がある

## 試した変更（描画結果が変わらなかった）
1. AnimationClip.cpp: `S*R*T` ↔ `T*R*S` の変更
2. Skeleton.cpp: `Parent*Local` ↔ `Local*Parent` の変更
3. Skeleton.cpp: `Global*offsetMatrix` ↔ `offsetMatrix*Global` の変更
4. Renderer.cpp: Transposeの追加/削除
5. AnimationClip.cpp: prohスタイルの手動行列構築（2024-11-26）
6. 上記すべての組み合わせ

## 現在の状態
- prohと同じ行列計算順序にしても、画像のように手足が極端に伸びる
- 数値は正しいのに描画が崩れる
- offsetMatrixの処理に問題がある可能性
- AnimationClip.cppを`S*R*T`に戻しても描画結果が変わらない（2024-11-26確認）
- prohスタイルの手動行列構築でも描画結果が変わらない（2024-11-26確認）

## 問題の本質
- 行列の乗算順序を変えても描画が全く変わらない
- これはDirectXMath (column-major) とproh (row-major) の根本的な違いが原因
- **prohはDirectXMathを全く使っていない（独自のfloat m[4][4]実装）**
- AnimationClip.cppの変更だけでは不十分
- **他の箇所（SkinnedModelImporter, Skeleton, Renderer）に問題がある可能性**
