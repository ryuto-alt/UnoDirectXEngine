# UnoEngine - Project Overview

## Purpose
UnoEngineは、DirectX 12を使用したC++20ベースの3Dゲームエンジンプロジェクトです。
学習と実用のバランスを取りながら、段階的に機能を拡張していくことを目指しています。

## Tech Stack
- **言語**: C++20
- **グラフィックスAPI**: DirectX 12
- **数学ライブラリ**: DirectXMath (ラッパーとして使用)
- **ビルドシステム**: MSBuild (Visual Studio 2022 v145 toolset)
- **ターゲットプラットフォーム**: Windows 10/11 (x64)
- **外部依存**: DirectXTex (NuGet経由でインストール済み)

## Current Features
- **Graphics**: Device initialization, Command Queue, Swap Chain, Pipeline State, Shader compilation
- **Math**: Vector3, Vector4, Matrix4x4, Quaternion (DirectXMath wrapper)
- **Camera**: Perspective/Orthographic projection, View matrix
- **Rendering**: Basic triangle rendering with MVP transformation
- **Window**: Win32 window management

## Codebase Structure
```
UnoEngine/
├── Engine/
│   ├── Core/           - Application framework, Camera, Basic types
│   ├── Graphics/       - DirectX12 wrapper (Device, Pipeline, Shader, VertexBuffer, ConstantBuffer)
│   ├── Math/           - Math library (Vector, Matrix, Quaternion, MathUtils)
│   ├── Window/         - Win32 window management
│   └── Utils/          - (Reserved for future utilities)
├── Shaders/            - HLSL shader files (BasicVS.hlsl, BasicPS.hlsl)
├── main.cpp            - Application entry point
└── UnoEngine.vcxproj   - Visual Studio project file
```

## Key Architecture Decisions
- **Abstraction Level**: Medium (DirectX12は隠蔽せず、ラップして使いやすくする)
- **Memory Management**: ComPtr for DirectX objects, std::unique_ptr/shared_ptr for others
- **Code Organization**: Feature-based subdirectories
- **Resource Management**: RAII-based resource lifetime
