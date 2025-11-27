# UnoEngine Interface Design v3

## 1. Component (Engine/Core/Component.h)

```cpp
#pragma once
#include "Types.h"

namespace UnoEngine {

class GameObject;

class Component {
public:
    Component() = default;
    virtual ~Component() = default;

    // Lifecycle methods
    virtual void Awake() {}           // Called immediately after AddComponent
    virtual void Start() {}           // Called once before first Update (after all Awake)
    virtual void OnUpdate(float deltaTime) {}  // Called every frame
    virtual void OnDestroy() {}       // Called on RemoveComponent or GameObject destruction

    // State
    GameObject* GetGameObject() const { return gameObject_; }
    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

    // Internal state tracking
    bool HasStarted() const { return hasStarted_; }

protected:
    friend class GameObject;
    friend class Scene;
    
    GameObject* gameObject_ = nullptr;
    bool enabled_ = true;
    bool hasStarted_ = false;  // Track if Start() was called
};

} // namespace UnoEngine
```

---

## 2. Logger (Engine/Core/Logger.h)

```cpp
#pragma once
#include <string>
#include <format>

namespace UnoEngine {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();

    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);

    // Format support
    template<typename... Args>
    static void Debug(std::format_string<Args...> fmt, Args&&... args) {
        Debug(std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Info(std::format_string<Args...> fmt, Args&&... args) {
        Info(std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Warning(std::format_string<Args...> fmt, Args&&... args) {
        Warning(std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Error(std::format_string<Args...> fmt, Args&&... args) {
        Error(std::format(fmt, std::forward<Args>(args)...));
    }

private:
    static LogLevel currentLevel_;
};

} // namespace UnoEngine
```

---

## 3. ResourceManager (Engine/Resource/ResourceManager.h)

```cpp
#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>

namespace UnoEngine {

class GraphicsDevice;
class SkinnedModel;
class Texture;
class Material;
class AnimationClip;

class ResourceManager {
public:
    explicit ResourceManager(GraphicsDevice* device);
    ~ResourceManager();

    // Load resources (cached)
    SkinnedModel* LoadSkinnedModel(const std::string& path);
    Texture* LoadTexture(const std::string& path);
    Material* LoadMaterial(const std::string& path);
    AnimationClip* LoadAnimation(const std::string& path);

    // Generic load with type
    template<typename T>
    T* Load(const std::string& path);

    // Cache management
    void UnloadUnused();
    void Clear();

    // Stats
    size_t GetCachedResourceCount() const;

private:
    GraphicsDevice* device_;
    
    std::unordered_map<std::string, std::unique_ptr<SkinnedModel>> skinnedModels_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
    std::unordered_map<std::string, std::unique_ptr<AnimationClip>> animations_;
};

} // namespace UnoEngine
```

---

## 4. MeshRendererBase (Engine/Rendering/MeshRendererBase.h)

```cpp
#pragma once
#include "../Core/Component.h"
#include "../Math/BoundingVolume.h"

namespace UnoEngine {

class Material;

class MeshRendererBase : public Component {
public:
    MeshRendererBase() = default;
    virtual ~MeshRendererBase() = default;

    // Material
    void SetMaterial(Material* material) { materialOverride_ = material; }
    Material* GetMaterial() const { return materialOverride_ ? materialOverride_ : defaultMaterial_; }
    Material* GetMaterialOverride() const { return materialOverride_; }
    
    // Bounds (for culling)
    const BoundingBox& GetBounds() const { return bounds_; }
    const BoundingSphere& GetBoundingSphere() const { return boundingSphere_; }

    // Visibility
    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool visible) { isVisible_ = visible; }

protected:
    void SetDefaultMaterial(Material* material) { defaultMaterial_ = material; }
    void UpdateBounds(const BoundingBox& bounds);

    Material* defaultMaterial_ = nullptr;   // From model/mesh
    Material* materialOverride_ = nullptr;  // User override
    BoundingBox bounds_;
    BoundingSphere boundingSphere_;
    bool isVisible_ = true;
};

} // namespace UnoEngine
```

---

## 5. SkinnedMeshRenderer (Engine/Rendering/SkinnedMeshRenderer.h)

```cpp
#pragma once
#include "MeshRendererBase.h"
#include <string>
#include <vector>

namespace UnoEngine {

class SkinnedModel;
class SkinnedMesh;
class AnimatorComponent;
class ResourceManager;
struct BoneMatrixPair;

class SkinnedMeshRenderer : public MeshRendererBase {
public:
    SkinnedMeshRenderer() = default;
    ~SkinnedMeshRenderer() override = default;

    // Lifecycle
    void Awake() override;
    void Start() override;
    void OnDestroy() override;

    // Model loading
    void SetModel(const std::string& path);
    void SetModel(SkinnedModel* model);
    SkinnedModel* GetModel() const { return model_; }

    // Mesh access
    const std::vector<SkinnedMesh>& GetMeshes() const;
    
    // Bone matrices (from linked Animator)
    const std::vector<BoneMatrixPair>* GetBoneMatrixPairs() const;

    // Animator link (密結合)
    AnimatorComponent* GetAnimator() const { return animator_; }

private:
    void LinkAnimator();  // Find and link AnimatorComponent on same GameObject

    SkinnedModel* model_ = nullptr;
    AnimatorComponent* animator_ = nullptr;
    std::string pendingModelPath_;  // For deferred loading
};

} // namespace UnoEngine
```

---

## 6. RenderSystem (Engine/Rendering/RenderSystem.h)

```cpp
#pragma once
#include <vector>

namespace UnoEngine {

class Scene;
class GraphicsDevice;
struct SkinnedRenderItem;
struct RenderItem;
class Camera;

class RenderSystem {
public:
    explicit RenderSystem(GraphicsDevice* device);
    ~RenderSystem();

    // Collect render items from scene
    void CollectRenderItems(Scene* scene);

    // Get collected items (sorted by material for batching)
    const std::vector<RenderItem>& GetRenderItems() const { return renderItems_; }
    const std::vector<SkinnedRenderItem>& GetSkinnedRenderItems() const { return skinnedRenderItems_; }

    // Clear collected items
    void Clear();

    // Culling (optional)
    void SetCamera(Camera* camera) { camera_ = camera; }
    void EnableFrustumCulling(bool enable) { frustumCullingEnabled_ = enable; }

private:
    void SortByMaterial();
    bool IsVisible(const MeshRendererBase* renderer) const;

    GraphicsDevice* device_;
    Camera* camera_ = nullptr;
    
    std::vector<RenderItem> renderItems_;
    std::vector<SkinnedRenderItem> skinnedRenderItems_;
    
    bool frustumCullingEnabled_ = false;
};

} // namespace UnoEngine
```

---

## 7. ImportOptions (Engine/Resource/ImportOptions.h)

```cpp
#pragma once

namespace UnoEngine {

struct ImportOptions {
    // Coordinate system conversion
    bool convertCoordinateSystem = true;
    
    // Additional rotation (degrees)
    float rotationX = 0.0f;
    float rotationY = 0.0f;  
    float rotationZ = 0.0f;
    
    // Scale
    float scale = 1.0f;
    
    // Animation import
    bool importAnimations = true;
    
    // Material import
    bool importMaterials = true;
    
    // Factory method for common presets
    static ImportOptions Default() { return {}; }
    
    static ImportOptions ForGltf() {
        ImportOptions opts;
        opts.rotationX = -90.0f;  // Stand up Y-up models
        return opts;
    }
    
    static ImportOptions ForFbx() {
        ImportOptions opts;
        // FBX specific settings
        return opts;
    }
};

} // namespace UnoEngine
```

---

## 8. BoundingVolume (Engine/Math/BoundingVolume.h)

```cpp
#pragma once
#include "Vector3.h"

namespace UnoEngine {

struct BoundingBox {
    Vector3 min;
    Vector3 max;
    
    Vector3 GetCenter() const { return (min + max) * 0.5f; }
    Vector3 GetExtents() const { return (max - min) * 0.5f; }
    
    bool Contains(const Vector3& point) const;
    bool Intersects(const BoundingBox& other) const;
    
    static BoundingBox CreateFromPoints(const Vector3* points, size_t count);
};

struct BoundingSphere {
    Vector3 center;
    float radius = 0.0f;
    
    bool Contains(const Vector3& point) const;
    bool Intersects(const BoundingSphere& other) const;
    
    static BoundingSphere CreateFromBox(const BoundingBox& box);
};

} // namespace UnoEngine
```

---

## Directory Structure After Implementation

```
Engine/
├── Core/
│   ├── Component.h/cpp      # Updated with lifecycle
│   ├── GameObject.h/cpp     # Updated for lifecycle calls
│   ├── Scene.h/cpp          # Updated for lifecycle management
│   ├── Logger.h/cpp         # NEW
│   └── ...
├── Rendering/
│   ├── MeshRendererBase.h/cpp    # NEW
│   ├── SkinnedMeshRenderer.h/cpp # NEW (replaces direct model handling)
│   ├── RenderSystem.h/cpp        # Updated (collects from scene)
│   └── ...
├── Resource/
│   ├── ResourceManager.h/cpp     # NEW
│   ├── ImportOptions.h           # NEW
│   └── ...
├── Math/
│   ├── BoundingVolume.h/cpp      # NEW
│   └── ...
└── ...

Samples/                          # NEW folder
└── Scenes/
    └── SampleScene.h/cpp         # Moved from Game/Scenes/GameScene
```
