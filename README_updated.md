[![Clang](https://github.com/clshortfuse/renodx/actions/workflows/clang-x64.yml/badge.svg)](https://github.com/clshortfuse/renodx/actions/workflows/clang-x64.yml) [![Ninja](https://github.com/clshortfuse/renodx/actions/workflows/ninja-x64.yml/badge.svg)](https://github.com/clshortfuse/renodx/actions/workflows/ninja-x64.yml) [![Visual Studio](https://github.com/clshortfuse/renodx/actions/workflows/visual-studio-x64.yml/badge.svg)](https://github.com/clshortfuse/renodx/actions/workflows/visual-studio-x64.yml) 

[![Discord](https://img.shields.io/discord/1408098019194310818?logo=discord&logoColor=%23fff&label=Discord&labelColor=%235865F2)](https://discord.gg/F6AUTeWJHM)

# renodx
RenoDX, short for "Renovation Engine for DirectX Games", is a toolset to mod games. Currently it can replace shaders, inject buffers, add overlays, upgrade swapchains, upgrade texture resources, and write user settings to disk. Because RenoDX uses Reshade's add-on system, compatibility is expected to be pretty wide. Using Reshade simplifies all the hooks necessary to tap into DirectX without worrying about patching version-specific exe files.


# Contributing

* See [CONTRIBUTING.md](./docs/CONTRIBUTING.md)
* For the live devkit and MCP workflow, see [DEVKIT_MCP.md](./docs/DEVKIT_MCP.md)

# Mods

* See [Mods page](https://github.com/clshortfuse/renodx/wiki/Mods)

# Utilities

* [renodx-fpslimiter.addon64](https://clshortfuse.github.io/renodx/renodx-fpslimiter.addon64) &ndash; FPS Limiter
* [renodx-devkit.addon64](https://clshortfuse.github.io/renodx/renodx-devkit.addon64) &ndash; Developer kit to help build addons
* [decomp.exe](https://clshortfuse.github.io/renodx/decomp.exe) &ndash; Shader Model 6.0+ Decompiler

# RenoOGL: OpenGL RenoDX and DevKit

**Experimental source integration, not a prebuilt Windows release.** This copy
contains the OpenGL DevKit target, Generic OpenGL addon, a patched ReShade
backend, and the recovered x86 process-heap alignment fix. The Windows x64
build path has now been tested successfully, including loading the patched
ReShade OpenGL proxy and RenoDX Generic OpenGL addon in Wolfenstein: The New
Order and capturing live GLSL vertex/fragment shaders. Replacement/HDR behavior
still requires game-specific validation. The included build steps produce the
required Windows binaries on your PC; no prebuilt DLL/addon binaries are
included in this archive.

## Put this copy in your new repository

The archive contains a `renodx` folder matching the uploaded repository.
Extract it under `C:\Users\ctied\Documents\renoogl`, so the root containing
`CMakeLists.txt` is `C:\Users\ctied\Documents\renoogl\renodx`.
Keep your existing folder as a backup before replacing it. If you instead place
the repository directly in `renoogl`, adjust the commands below accordingly.
The supplied Git metadata and existing downloaded build tools are retained.
The ReShade submodule now contains local changes: do not reset/clean it or use
`git submodule update --force`. Other dependencies still need initialization.

## What changed

- An OpenGL-only `devkit_opengl` target, with x86 and x64 metadata. It shares the
  normal DevKit implementation and filters out non-OpenGL devices.
- Inspect GLSL text, SPIR-V disassembly, registered Add-on code, and file code.
  Linked vertex/fragment/compute shader bytes are retained before draw capture.
- Original / Add-on / File selection. Automatic reload keeps the selected
  source; explicit Load Shaders selects file replacements. Shader compilation
  or linking failures leave the original program available.
- Native `glShaderBinary`, `glSpecializeShader`, and `glSpecializeShaderARB`
  capture. Entry point and specialization IDs/values are retained and copied
  into the cached pipeline descriptors. Relinking invalidates the old program
  record before publishing the new one.
- A private ReShade export builds replacement programs with original attribute
  and fragment-output bindings. Before a draw/dispatch, the backend copies
  supported loose uniforms and UBO/SSBO block bindings, binds the replacement,
  then restores the original. Game `glUniform` calls keep targeting the original.
  Selection/file changes are refreshed at draws, even without another
  `glUseProgram` call.
- Correct OpenGL UBO injection binding, zero-offset enforcement, and zero-filled
  trailing padding to a 16-byte boundary. This does not fix mismatched field
  offsets inside a C++/GLSL layout.
- Binary-safe dumping and reload detection for `.glsl` and `.spv`; OpenGL
  decompilation semantics; owned Add-on bytes; specialization-array lifetime
  and descriptor-array allocation/free fixes.
- The recovered x86 fix uses matched aligned process-heap allocation/free for
  shared objects and containers, with allocation-size overflow checks.

The backend changes are under `external/reshade`; this is based on ReShade
`4a50d1eddace85734871d91792ff214f13f66c01` and RenoDX
`9b212edad4dde9bca2b823b1e045b712b1a8d854`.

## Build on Windows with RenoDX CMake presets

Install Visual Studio with Desktop development with C++, the MSVC toolset
required by your installed Visual Studio version, Windows SDK, LLVM/clang-cl,
CMake and Ninja; also install Git and Python 3. The verified x64 ReShade build
uses Visual Studio 18 / toolset `v145`. Use a VS developer PowerShell targeting
the game's architecture.
Run the following commands from the repository root:

```powershell
Set-Location 'C:\Users\ctied\Documents\renoogl\renodx'
```

### One-time dependency setup

```powershell
git submodule update --init --recursive
python -m pip install -r .\external\reshade\deps\glad\requirements.txt
.\scripts\setup-dev-env.ps1 -Install -Tools @('dxc', 'slang', 'glslang')
```

Do not add `--force` or `--remote` to the submodule command: retain the patched
ReShade files and pinned revisions. Pass Tools as a PowerShell array as shown.

### 32-bit OpenGL DevKit

Configure once, or again after adding a new addon target:

```powershell
cmake --preset clang-x86
```

Build using RenoDX's normal Release build preset:

```powershell
cmake --build --preset clang-x86-release --target devkit_opengl
```

Output: `build32\Release\renodx-devkit_opengl.addon32`.

### 32-bit Doom 3 addon

With your Doom 3 mod installed under `src\games\doom3`, configure and build:

```powershell
cmake --preset clang-x86
cmake --build --preset clang-x86-release --target doom3
```

Output: `build32\Release\renodx-doom3.addon32`.
The uploaded base repository did not contain `src\games\doom3`; the command
requires your separate Doom 3 addon sources to be present. The OpenGL DevKit
and the Doom 3 game addon are separate targets, so build both when using both.
For subsequent Doom 3 rebuilds, use just:

```powershell
cmake --build --preset clang-x86-release --target doom3
```

### 64-bit OpenGL DevKit

In a developer PowerShell targeting x64:

```powershell
cmake --preset clang-x64
cmake --build --preset clang-x64-release --target devkit_opengl
```

Output: `build\Release\renodx-devkit_opengl.addon64`.
For another game addon, replace `devkit_opengl` with its actual target name
and use the architecture supported by that addon and game executable.

The configure presets are `clang-x86` / `clang-x64`; the Release **build**
presets are `clang-x86-release` / `clang-x64-release`. These addon commands
use clang-cl. Rebuild each game addon to include the shared injection and
alignment fixes; rebuilding only the DevKit does not update game addons.

### Patched ReShade DLL: separate build

The RenoDX CMake targets above build addons. They do not build the patched
ReShade DLL required by this OpenGL replacement path. ReShade has its own
Visual Studio solution.

The **verified 64-bit build** uses MSVC toolset `v145` and ReShade's solution
platform name `64-bit`:

```powershell
msbuild .\external\reshade\ReShade.sln `
  /t:ReShade `
  /m `
  /p:Configuration=Release `
  /p:Platform="64-bit" `
  /p:PlatformToolset=v145
```

This produces:

```text
external\reshade\bin\x64\Release\ReShade64.dll
```

Rename `ReShade64.dll` to `opengl32.dll` when installing it beside a 64-bit
OpenGL game executable. Use **Release**, not the restricted-addon
`Release Signed`.

The ClangCL ReShade build was useful for finding source/compiler issues, but
with the current tree it reaches the final link step and hits unresolved
symbols. Use the verified MSVC `v145` command above for the patched ReShade DLL.
The RenoDX addon CMake presets can still use clang-cl.

The optional `scripts\build-opengl.ps1` wrapper from the source package can
still build/stage everything, but verify its selected MSVC toolset against your
installed Visual Studio version. Manual builds above output directly to their
build folders and do not populate `dist\opengl`. Rebuild all RenoDX addons
loaded together so they use the same shared allocator implementation.

## Install and inspect

1. Back up the game's existing local ReShade DLL/addons. Copy the matching ReShade DLL
   from the build output listed above beside the game executable and name it
   `opengl32.dll`. Copy `renodx-devkit_opengl.addon32` or `.addon64` from its
   corresponding build output beside it. Do not copy either into a Windows system folder.
2. Use this DevKit **instead of** the normal DevKit. Use the patched ReShade
   built from this tree; stock ReShade lacks the replacement export. You can
   also load a rebuilt game-specific RenoDX addon to supply its settings buffer.
3. Launch the game and open the ReShade overlay. In RenoDX DevKit, capture a
   snapshot and select a shader. Inspect original GLSL/SPIR-V, Add-on code,
   or the Live Shader view. Linked/pending/unavailable counts report build
   state, not proof of correct rendering.
4. Dump a shader, put a copy in the configured live-shader folder, and keep its
   original hash in the filename. Start by loading an unchanged shader.
5. Click **Load Shaders**, then switch **Original**, **Add-on**, or **File** in
   the Source column. Add-on requires a registered replacement. Enable Live
   Shaders for reload while editing. Keep only one file per hash in that folder.
6. Make a small visible change and switch back to Original. Check textures,
   animation, UI, resize/alt-tab and `ReShade.log`. Test with your target GPU.

## GLSL source and SPIR-V

| Input | Example filename | Behavior |
| --- | --- | --- |
| Driver GLSL source | `0xHASH.frag.glsl` | Driver compiles text; no automatic include preprocessing |
| OpenGL-targeted SPIR-V | `0xHASH.frag.gl.spv` | Driver specializes the binary |
| Offline GLSL to OpenGL SPIR-V | `0xHASH.frag.gl.glsl` | Existing glslang workflow uses OpenGL semantics |

Use `.vert` / `.comp` for vertex / compute stages. Replace `HASH` with the
DevKit hash. Binary SPIR-V starts with the magic word `0x07230203`; the file
extension alone does not establish its format or target environment. For
OpenGL use glslang `-G`, **not** Vulkan `-V`. Supply `glslangValidator.exe`
(or `glslang.exe`), `spirv-dis.exe`, and `spirv-cross.exe` through Tools Path
for their respective compilation/disassembly/decompilation features.

GLSL source and SPIR-V are two shader delivery formats. `glProgramBinary`
is a third, opaque driver program-cache path; it is not a SPIR-V shader file.
SPIR-V source capture needs the game to call the hooked binary/specialization
functions after the patched backend is loaded. Opaque cached programs cannot
be reconstructed into original shader modules by this patch.

A linked program cannot mix GLSL and SPIR-V stages. Keep replacement format,
entry point, specialization IDs, and stage interfaces compatible with the
original program. There is no automatic whole-program conversion. Same module
bytes used with different entry points/specializations share a byte hash, so
file replacement by hash affects all such uses; their metadata stays per-program.

## Settings injection

See `src/addons/devkit_opengl/examples/injection_layout.hpp` and
`injection_layout.glsl.txt` for matching scalar layouts. OpenGL uses a GLSL
`layout(std140, binding = N) uniform` block in place of an HLSL cbuffer. Pick
an unused binding within the driver's limits and configure the game addon to
inject into that same slot. Match field order and every std140 offset; C++
`alignas(16)` alone does not align vec3/array/matrix members correctly.
The game addon must register the injection data and the affected shader.
Choosing File in DevKit does not create a settings buffer by itself.

## Supported scope and known limits

- Replacement state preservation requires an **OpenGL 4.3+** context. SPIR-V
  additionally needs OpenGL 4.6 or `GL_ARB_gl_spirv`. Older source shaders may be
  inspectable, but this backend declines the new replacement path there.
- Intended path: monolithic linked vertex/fragment/compute programs using
  ordinary numeric uniforms, texture/image unit uniforms, UBOs and SSBOs.
  CPU readback/copy of uniforms on each draw can cost performance.
- Separable pipelines, transform-feedback programs, subroutine uniforms,
  incompatible uniform types, ambiguous unnamed block matches, and multiple
  attached shader objects in one stage are rejected for replacement.
- Bindless handles, unusual compatibility built-ins, concurrent mutation of
  shared programs across contexts, geometry/tessellation inspection, and
  specialized draw extension paths are not validated/supported by this DevKit.
  The main OpenGL draw/dispatch hooks are covered, not every vendor extension.
- Legacy `!!ARBfp` / `!!ARBvp` assembly can be identified/inspected where exposed,
  but is not replaceable through the GLSL pipeline path. Doom 3 direct-ARB hooks
  remain a separate implementation. Fixed-function draws have no GLSL shader.
- This is shader development/injection infrastructure. It does not automatically
  remove game clamps, port DirectX bytecode replacements, add Psycho settings,
  upgrade framebuffer formats, or establish a correct HDR output path for every
  OpenGL game. Those require game-specific shader and output work.

## Validation in this environment

Passed on Linux with GCC 13 and Mesa 25.2.8, OpenGL 4.5 core:

- Byte-format tests: GLSL termination, empty inputs, SPIR-V header/alignment,
  and ARB identification.
- Real GL programs: attribute bindings, float/vector/array/matrix/integer/uint
  and sampler state, UBO/SSBO block bindings, updated values on later draws,
  invalidated replacement lifetime, and incompatible uniform rejection.
- Pixel readback: original red -> replacement half-red -> original red, without
  an intervening application program bind.
- Actual allocator function bodies with a mock heap, simulating native
  8-byte/x86 and 16-byte/x64 alignment: over-alignment, zeroing, overflow,
  shared-object construction, and matched frees.

Verified on Windows x64:

- Patched ReShade builds successfully with MSVC toolset `v145` using the
  `Release|64-bit` solution configuration.
- `renodx-devkit_opengl.addon64` and `renodx-generic_opengl.addon64` load under
  the patched OpenGL ReShade proxy.
- Wolfenstein: The New Order creates its OpenGL contexts successfully and
  Generic OpenGL captures live GLSL vertex and fragment shader source.

Not yet validated: Windows x86 execution, native SPIR-V capture/replacement on
a Windows driver, full GLSL replacement behavior across games, or HDR output.

Portable test commands from the repository root:

```bash
g++ -std=c++20 test/opengl_shader_format/tests.cpp -o /tmp/format-test
/tmp/format-test
python3 test/process_heap_alignment/run.py
python3 -m pip install -r external/reshade/deps/glad/requirements.txt
PYTHONPATH=external/reshade/deps/glad python3 -m glad --api gl:compatibility=4.6 --out-path /tmp/renoogl-glad --reproducible c --mx
g++ -std=c++20 -I/tmp/renoogl-glad/include test/opengl_program_state/tests.cpp /tmp/renoogl-glad/src/gl.c /lib/x86_64-linux-gnu/libEGL.so.1 -ldl -o /tmp/state-test
/tmp/state-test
```

The GL integration test requires Mesa's surfaceless EGL platform on Linux.

Reference: Khronos [ARB_gl_spirv specification](https://github.com/KhronosGroup/OpenGL-Registry/blob/main/extensions/ARB/ARB_gl_spirv.txt), including binary specialization and unnamed-resource reflection.


