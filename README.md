# MiniExtItemView

Standalone Union plugin for Gothic games. SDK compilation does not imply
that all game/renderer combinations have passed runtime testing.

Wraps native item descriptions using the current font, preserves values/colors, and centers the item preview. No extra item lines, scrolling, INI or menu.

## Install / uninstall

1. Install Union for your game.
2. Download `MiniExtItemView.vdf` from
   [Releases](https://github.com/Bezimienny111/MiniExtItemView/releases).
3. With the game closed, copy it into the game directory at
   `Data\Plugins\MiniExtItemView.vdf`.
4. Remove any older copy of this plugin from other VDFs or DLL autorun
   directories: do not load the same plugin twice.

The VDF includes the plugin and its pinned `UnionAPI.dll` runtime.
IAI also includes its own menu autorun script. Neither plugin requires CZM,
BetterInventory, campaign scripts, or changes to native save archives.
Uninstall with the game closed by removing this VDF. Global IAI preferences
may remain in `Gothic.ini`; neither plugin creates campaign save data.

Do not assume compatibility with full ExtendedItemView or other plugins replacing the same panel. This snapshot retains the bounded MiniExtItemView-layer.log diagnostic. A reported G1 + GD3D11 ResizeTexture crash remains unconfirmed and was also reported before Mini; this release does not claim to fix it.

See the [technical contract](docs/Fulldoc/miniExtItemView/documentation.md)
and [player-facing functionality](docs/Fulldoc/miniExtItemView/GDD.md).
Release notes distinguish builds/package checks from actual game tests.

## Build from source

Requirements: Git, CMake >= 3.21, Visual Studio 2022 C++ tools and Windows SDK.
Use **x86**, not x64. Dependencies are pinned Git submodules; no CZM checkout
or machine-specific CMake preset is required.

```powershell
git clone --recurse-submodules https://github.com/Bezimienny111/MiniExtItemView.git
Set-Location MiniExtItemView
cmake -S . -B out\build -G "Visual Studio 17 2022" -A Win32 -DBUILD_VDF=OFF
cmake --build out\build --config Release
```

For VDF packaging, provide your existing GothicVDFS installation (the tool
itself is not redistributed in this repository):

```powershell
cmake -S . -B out\build -DBUILD_VDF=ON "-DGOTHIC_VDFS_EXECUTABLE=C:\Tools\GothicVDFS.exe"
cmake --build out\build --config Release --target build_vdf
```

Output: `out\build\package\MiniExtItemView.vdf`. There is **no automatic game deploy**.
One DLL contains all four engine translation units.

## Source of truth / updates

This repository is a publication export from
[Czosnekdll](https://github.com/Bezimienny111/Czosnekdll).
Make changes in that integration repository, then run its
`tools\Export-StandalonePlugins.ps1` again. Do not develop divergent copies.
The export is allowlisted: no unrelated CZM modules, local settings, saves,
logs, dumps, experimental melee code, or BetterInventory sources.
`EXPORT-MANIFEST.json` records the base commit, selected worktree file hashes
and SDK commits. A base commit alone does not identify uncommitted changes.

Plugin functionality is unchanged by extraction; only standalone build,
publication metadata and documentation are added.

## Credits / licensing

Plugin sources originate from the CZM project by Bezimienny111.
Build helpers originate from
[Patrix9999/union-plugin-template](https://github.com/Patrix9999/union-plugin-template).
Union API / Gothic API are by Gratt & Union Team.
No new license for the project's own code is granted by this export.
Third-party components retain their licenses; see
[THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt) and `licenses`.
Binary release descriptions also reproduce the dependency license notices.
