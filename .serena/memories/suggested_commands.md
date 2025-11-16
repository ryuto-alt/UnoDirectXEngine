# UnoEngine - Suggested Commands

## Build Commands

### MSBuild (Visual Studio)
ビルドはVisual Studioから実行するのが推奨されます。

#### Debug Build
```
msbuild UnoEngine.vcxproj /p:Configuration=Debug /p:Platform=x64
```

#### Release Build
```
msbuild UnoEngine.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Visual Studio IDE
- **ビルド**: Ctrl+Shift+B
- **リビルド**: メニューから「ビルド」→「ソリューションのリビルド」
- **クリーン**: メニューから「ビルド」→「ソリューションのクリーン」

## Run Commands

### From Visual Studio
- **デバッグ実行**: F5
- **デバッグなし実行**: Ctrl+F5

### From Command Line
```
x64\Debug\UnoEngine.exe
```
または
```
x64\Release\UnoEngine.exe
```

## Shader Compilation
現在、シェーダーは実行時にコンパイルされます（`Shader::CompileFromFile()`を使用）。
プリコンパイルが必要になった場合は、fxcまたはdxcツールを使用します。

### Manual Shader Compilation (Future)
```
fxc /T vs_5_1 /E main /Fo BasicVS.cso Shaders/BasicVS.hlsl
fxc /T ps_5_1 /E main /Fo BasicPS.cso Shaders/BasicPS.hlsl
```

## Testing
現在、ユニットテストフレームワークは設定されていません。
将来的にGoogle TestやCatch2の導入を検討する可能性があります。

## Formatting and Linting
現在、自動フォーマッターは設定されていません。
Visual Studioのデフォルトフォーマッター（Ctrl+K, Ctrl+D）を使用できます。

### Visual Studio Formatting
- **ドキュメント全体**: Ctrl+K, Ctrl+D
- **選択範囲**: Ctrl+K, Ctrl+F

## Git Commands (Windows)
プロジェクトはGitで管理されています。

### Basic Git Operations
```bash
git status
git add .
git commit -m "commit message"
git push
git pull
```

## Windows System Commands

### Directory Operations
```cmd
dir                  # List files in current directory
cd <path>           # Change directory
mkdir <name>        # Create directory
rmdir <name>        # Remove directory
```

### File Operations
```cmd
type <file>         # Display file contents
copy <src> <dst>    # Copy file
del <file>          # Delete file
move <src> <dst>    # Move/rename file
```

### Process Management
```cmd
tasklist           # List running processes
taskkill /PID <id> # Kill process by ID
```

### Search
```cmd
findstr /s /i "pattern" *.cpp    # Search for pattern in cpp files
```

## Package Management (NuGet)
DirectXTexはNuGetでインストール済みです。

### NuGet Commands (from Package Manager Console in VS)
```
Install-Package <package-name>
Update-Package <package-name>
Uninstall-Package <package-name>
```

## Useful Development Commands

### Check DirectX SDK
```cmd
dxcaps
```

### Windows SDK Tools
- **DirectX Caps Viewer**: dxcapsviewer.exe
- **PIX for Windows**: グラフィックスデバッグツール

## Task Completion Checklist
タスク完了時に実行すべき内容については、`task_completion_checklist.md` を参照してください。
