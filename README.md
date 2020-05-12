# Dreamachine
## Description

![Imgur](https://i.imgur.com/RRW1aNL.png)
*Hope to see gif with Zoë here*

It is a tool for viewing and exporting assets from *DreamFall: The Longest Journey*. Currently it can export meshes and part of textures to **FBX**. I'm working on this tool as part of a [remake](https://youtu.be/LZ3Gs5kFNkg) of a small level from *DreamFall: The Longest Journey* on Unreal Engine 4.

## Current state
- [x] Meshes
- [x] Materials (Partially)
- [ ] Backgrounds
- [ ] Bones
- [ ] Animations
- [ ] ?Texts
- [ ] ?Voices

## Analogs

There is a great [DreamView](https://pingus.seul.org/~grumbel/dreamfall/), which was made by the wonderful Tobias Pfaff back in 2006. This tool can export meshes to DirectX, Milkshape or HalfLife2 formats and import them back. I am very grateful to Tobias Pfaff for the work and I regularly return to this tool for references.

# Dependencies
[FBX SDK 2020.0.1](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-0)

```
./vcpkg install DirectXTex corrade magnum[wglcontext,gl-info] magnum-plugins[pngimporter] fmt --triplet x64-windows
```

# How to build
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:\Dev\Tools\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 16 2019" -A x64 --target all ..
cmake --build . --config Release
```
