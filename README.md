# Cyberpunk 2077 Red Hot Tools (Enhanced Performance & Scalability Edition)

[![Cyberpunk 2077](https://img.shields.io/badge/Cyberpunk%202077-2.21+-red.svg)](https://www.cyberpunk.net/)
[![RED4ext](https://img.shields.io/badge/RED4ext-1.27.0+-orange.svg)](https://docs.red4ext.com/)
[![CET](https://img.shields.io/badge/Cyber%20Engine%20Tweaks-1.35.0+-blue.svg)](https://github.com/yamashi/CyberEngineTweaks)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/Platform-Windows%20%7C%20Wine%2FProton-purple.svg)]()

**Red Hot Tools** is an indispensable runtime development, inspection, and hot-reloading suite for **Cyberpunk 2077** modders and developers. It provides deep in-game inspection of world nodes, game entities, physics actors, and Ink UI widgets, alongside instant hot-reloading of scripts, tweaks, and archives without restarting the game.

This fork merges the latest upstream master updates with critical multi-threading, scalability, and performance optimizations designed for heavy mod lists (100k+ streamed world nodes and heavy asset pools).

---

## 🚀 Key Features

### 🔍 World Inspector & Physics Tracer
- **Real-Time World Node Inspection**: Inspect streaming sectors, node definitions, static bounds, components, and entity instances in the 3D viewport.
- **Physics Actor Resolution**: Automatically resolves physics proxy hit objects, actor counts, sub-shapes, local/world transforms, and collision hulls.
- **Visual Highlighting**: Overlay custom highlight shaders, collision boundaries, and wireframes on targeted world objects directly in-game.

### 🎨 Ink UI Inspector
- **Widget Hierarchy Explorer**: Live tree inspection of Ink layers, windows, canvases, texts, images, and custom game widgets.
- **Resource Path Resolving**: Instantly resolves hashes for texture atlases, font families, styles, shape resources, and video clips.
- **Real-Time Property Viewer**: View layout anchors, margins, dimensions, tints, states, and event listeners.

### ⚡ Hot Reloading Engine
- **Scripts**: Instantly recompile and hot-reload `redscript` files while the game is running.
- **Tweaks**: Hot reload `TweakXL` / TweakDB flat records and modifications on the fly.
- **Archives**: Hot reload `ArchiveXL` and game archives dynamically without game restarts.
- **Compilation Guard**: Intercepts script compilation failures at launch, preventing the game from crashing into an unrecoverable state.

### 🛠️ Editor & Tool Integrations
- **Cyber Engine Tweaks (CET)**: Full in-game ImGui overlay UI with dedicated tabs for Hot Reload, Ink Inspector, and World Inspector.
- **Visual Studio Code**: Dedicated VS Code extension (`red-hot-vscode`) providing one-click hot reload commands and shortcut keybindings.
- **WolvenKit**: Seamless one-click pack and hot-reload integration directly from the WolvenKit editor.

---

## ⚡ Enhancements & Merged Improvements

This version merges upstream master fixes with specialized enterprise-grade performance and stability enhancements:

| Component | Improvement | Impact |
| :--- | :--- | :--- |
| **World Inspector Scalability** | Adaptive frustum update throttling (30 Hz $\to$ 15 Hz $\to$ 8 Hz $\to$ 4 Hz based on node count $>100\text{k}$) and on-demand pause when the CET overlay is closed. | **Eliminates framerate degradation** in dense city sectors and heavy mod setups. |
| **ResourcePathRegistry** | Decoupled background loading of 160k+ hashes with read-fastpath (`std::shared_lock`). | **Eliminates game thread lock stalls** when creating new resource paths during gameplay. |
| **ObjectRegistry GC Scaling** | Replaced static capacity boundary checks with an adaptive 2x growth threshold. | **Prevents hitching/freezes** during frequent scriptable handle registrations. |
| **Physics Actor Counting** | Integrated upstream master `PhysicsProxy::GetNumActors` resolving. | Correctly resolves actor counts and shape instances across all collision targets. |
| **ResourceLogger Hooks** | Made expensive missing-resource diagnostic hooks opt-in via build flag (`--resource_logger=y`). | Prevents unnecessary function interception overhead on hot file-loading paths. |
| **Toolchain & Linker Fixes** | Pinned `tiltedcore.mimalloc` to `2.x`, enabled `/Ob2` release inlining, and fixed build headers. | Clean compilation on modern xmake/MSVC and Wine/Proton cross-compilation environments. |

---

## 📋 Requirements & Compatibility

- **Cyberpunk 2077**: Patch **2.21** or newer
- **[RED4ext](https://docs.red4ext.com/getting-started/installing-red4ext)**: 1.27.0+
- **[Cyber Engine Tweaks (CET)](https://github.com/yamashi/CyberEngineTweaks)**: 1.35.0+
- **[ArchiveXL](https://github.com/psiberx/cp2077-archive-xl)**: 1.21.0+ *(recommended)*
- **[TweakXL](https://github.com/psiberx/cp2077-tweak-xl)**: 1.10.0+ *(recommended)*
- **[redscript](https://github.com/jac3km4/redscript)**: 0.5.25+ *(optional, for script hot reloading)*

---

## 📦 Installation

### Automatic / Manual Installation (ZIP)

1. Download the latest release package: [`RedHotTools-1.3.0.zip`](https://github.com/0xra0/cp2077-red-hot-tools/releases).
2. Extract the archive directly into your **Cyberpunk 2077** installation directory:
   - **GOG**: `C:\Games\Cyberpunk 2077\`
   - **Steam**: `C:\Program Files (x86)\Steam\steamapps\common\Cyberpunk 2077\`
   - **Epic Games**: `C:\Program Files\Epic Games\Cyberpunk 2077\`
3. Verify that the files are placed in the correct directories:
   ```text
   Cyberpunk 2077/
   ├── bin/
   │   └── x64/
   │       └── plugins/
   │           └── cyber_engine_tweaks/
   │               └── mods/
   │                   └── RedHotTools/
   │                       ├── init.lua
   │                       ├── version.lua
   │                       ├── libs/
   │                       └── modules/
   └── red4ext/
       └── plugins/
           └── RedHotTools/
               ├── RedHotTools.dll
               └── Resources.txt
   ```

### VS Code Extension (Optional)

1. Download `red-hot-vscode-0.1.0.vsix` from the [Releases](https://github.com/0xra0/cp2077-red-hot-tools/releases) page.
2. In Visual Studio Code, press `Ctrl+Shift+X` (Extensions view), click the `...` menu at the top, and select **Install from VSIX...**.
3. In VS Code settings (`settings.json`), set your game path:
   ```json
   {
     "redHotTools.gameDir": "C:\\Games\\Cyberpunk 2077"
   }
   ```

---

## 🎮 How to Use

### In-Game UI Overlay (CET)
1. Launch Cyberpunk 2077 and load into your save.
2. Press your **Cyber Engine Tweaks** overlay binding (default: `~` or `F4`).
3. Locate the **Red Hot Tools** window.
4. Available tabs:
   - **Hot**: One-click buttons to reload scripts, tweaks, or game archives.
   - **World**: Point your crosshair at any object, vehicle, or static mesh to inspect its node data, collision properties, components, and sector IDs. Toggle wireframes and highlights.
   - **Ink**: Open menus or HUD elements and inspect the active widget tree, layout anchors, font sizes, colors, and textures.

### VS Code Shortcuts
When editing `.reds` (redscript) or `.yaml` (TweakXL) files:
- Press `Ctrl+Shift+P` and execute `RedHotTools: Reload Scripts` or `RedHotTools: Reload Tweaks`.
- Or bind these commands to convenient hotkeys in VS Code.

---

## ⚙️ Building from Source

### Prerequisites
- [xmake](https://xmake.io/) (v2.8+)
- MSVC v143+ (Visual Studio 2022 C++ build tools) or Wine/Proton cross-compiler on Linux

### Build Steps

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/0xra0/cp2077-red-hot-tools.git
cd cp2077-red-hot-tools

# Configure for Windows x64 release
xmake f -p windows -a x64 -m release

# Build the DLL
xmake

# (Optional) Enable verbose missing/unloaded resource diagnostics
xmake f -p windows -a x64 -m release --resource_logger=y
xmake
```

Compiled binary output will be generated at:
`build/windows/x64/release/RedHotTools.dll`

---

## ⚠️ Known Limitations

- Struct memory layouts cannot be dynamically expanded at runtime if existing instances are active during hot reloading.
- Dynamic scriptable system request handlers must be registered at game session start; adding new handlers requires reloading the save game.

---

## 📜 Credits & Acknowledgments

- **psiberx**: Original creator of [Red Hot Tools](https://github.com/psiberx/cp2077-red-hot-tools), ArchiveXL, TweakXL, and Codeware.
- **Yamashi & the CET Team**: Cyber Engine Tweaks framework.
- **WopsS**: RED4ext plugin framework.
- **CP77 Modding Community**: Ongoing research and reverse-engineering contributions.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
