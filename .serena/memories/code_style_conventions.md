# UnoEngine - Code Style and Conventions

## Naming Conventions

### Files
- **Header Files**: PascalCase (e.g., `GraphicsDevice.h`, `Matrix.h`)
- **Source Files**: PascalCase (e.g., `GraphicsDevice.cpp`, `Pipeline.cpp`)
- **Shader Files**: PascalCase with shader stage suffix (e.g., `BasicVS.hlsl`, `BasicPS.hlsl`)

### Classes and Structs
- **Classes**: PascalCase (e.g., `GraphicsDevice`, `VertexBuffer`, `Camera`)
- **Structs**: PascalCase (e.g., `Vertex`, `TransformCB`, `ApplicationConfig`)

### Variables and Members
- **Local Variables**: camelCase (e.g., `device`, `cmdList`, `rotation`)
- **Member Variables**: camelCase with trailing underscore (e.g., `pipeline_`, `vertexShader_`, `mappedData_`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `BACK_BUFFER_COUNT`, `PI`, `DEG_TO_RAD`)
- **Namespaced Constants**: PascalCase namespace with UPPER_SNAKE_CASE (e.g., `Math::PI`, `Math::EPSILON`)

### Functions and Methods
- **Functions/Methods**: PascalCase (e.g., `Initialize()`, `GetDevice()`, `SetPosition()`)
- **Inline Utility Functions**: PascalCase (e.g., `Lerp()`, `Clamp()`, `ToRadians()`)

### Namespaces
- **Primary Namespace**: `UnoEngine`
- **Sub-namespaces**: PascalCase (e.g., `UnoEngine::Math`)

## Type System
- Use custom type aliases for fundamental types:
  - `uint8`, `uint16`, `uint32`, `uint64`
  - `int8`, `int16`, `int32`, `int64`
- Use smart pointer aliases: `UniquePtr<T>`, `SharedPtr<T>`, `MakeUnique<T>()`, `MakeShared<T>()`
- Use `ComPtr<T>` for DirectX COM objects

## Code Organization

### Header Guards
- Use `#pragma once` (modern approach, simpler)

### Include Order
1. Corresponding header (for .cpp files)
2. Engine headers (e.g., `"Engine/Core/Types.h"`)
3. System headers (e.g., `<d3d12.h>`, `<DirectXMath.h>`)
4. Standard library headers (e.g., `<vector>`, `<memory>`)

### Class Structure
```cpp
class ClassName {
public:
    // Constructors/Destructor
    ClassName() = default;
    ~ClassName() = default;

    // Public methods
    void PublicMethod();

    // Accessors (getters are prefixed with 'Get')
    Type GetValue() const { return value_; }

private:
    // Private methods
    void PrivateMethod();

    // Member variables (trailing underscore)
    Type value_;
    ComPtr<ID3D12Device> device_;
};
```

## Comments and Documentation
- **日本語コメント**: プロジェクト全体で日本語コメントを使用
- **コメントスタイル**: 
  - 単行: `// コメント内容`
  - ブロック: 通常は使用せず、単行コメントを複数行使用
- **シンプルな説明**: 関数名が自明な場合はコメント不要。複雑なロジックにのみコメントを付ける

## Modern C++ Features
- Use C++20 features where appropriate
- `constexpr` for compile-time constants
- `auto` for type inference where it improves readability
- Range-based for loops when iterating
- Smart pointers for ownership management
- RAII for resource management

## DirectX Best Practices
- Always use `ComPtr` for COM objects
- Use `ThrowIfFailed()` helper for error handling
- Follow DirectX12 synchronization patterns (fences, GPU waits)
- Separate resource creation and usage
- Keep command list recording minimal and efficient

## Windows-Specific
- Use `NOMINMAX` macro to avoid conflicts with `std::min`/`std::max`
- Wrap std functions in parentheses when macro conflicts possible: `(std::min)`, `(std::max)`
- Use wide strings (`wchar_t`, `std::wstring`, `L"..."`) for Win32 APIs
- Character set: Unicode
