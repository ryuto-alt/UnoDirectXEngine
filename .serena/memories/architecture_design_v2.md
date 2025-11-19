# UnoEngine Architecture Design v2

## Design Philosophy
- **Rendering**: Forward Rendering Pipeline
- **ECS**: Hybrid ECS (GameObject/Component + System layer)
- **Resource**: Centralized ResourceManager
- **Layer**: Renderer moved to Engine layer
- **Implementation**: Incremental migration

## Directory Structure
```
Engine/
├── Core/              # Application, Scene, GameObject, Transform, Camera
├── Rendering/         # 🆕 Renderer, RenderSystem, LightManager, RenderView/Item
├── Graphics/          # Low-level API (GraphicsDevice, Pipeline, Mesh, Material)
├── Resource/          # 🆕 ResourceManager, ResourceLoader, ObjLoader
├── Systems/           # 🆕 ISystem interface
├── Math/
├── Input/
├── Window/
└── UI/

Game/
├── GameApplication.h/cpp
├── Components/        # Player, PlayerController
├── Systems/           # PlayerSystem (implements ISystem)
└── Scenes/            # GameScene
```

## Key Changes
1. **Rendering Layer Separation**
   - `Game/Renderer.*` → `Engine/Rendering/Renderer.*`
   - `Engine/Core/RenderSystem.*` → `Engine/Rendering/`
   - `Engine/Graphics/LightManager.*` → `Engine/Rendering/`
   - `Engine/Graphics/RenderView.h` → `Engine/Rendering/`
   - `Engine/Graphics/RenderItem.h` → `Engine/Rendering/`

2. **Resource Layer**
   - `Engine/Graphics/ResourceLoader.*` → `Engine/Resource/`
   - `Engine/Graphics/ObjLoader.*` → `Engine/Resource/`
   - New `Engine/Resource/ResourceManager.h/cpp`

3. **System Foundation**
   - New `Engine/Systems/ISystem.h`
   - `PlayerSystem` implements `ISystem`
   - `Scene` has System management

4. **Dependency Fix**
   - `Application.h` no longer includes `Game/Renderer.h`
   - Now includes `Engine/Rendering/Renderer.h`

## Migration Phases
- Phase 1: Create directories (Rendering, Resource, Systems), remove Utils
- Phase 2: Move Rendering layer files
- Phase 3: Move Resource layer files
- Phase 4: Create System foundation
- Phase 5: Fix Application dependencies

## Implementation Notes
- Incremental approach: keep existing code working during migration
- Minimal comments, modern C++20 style
- Use serena MCP tools for file operations
