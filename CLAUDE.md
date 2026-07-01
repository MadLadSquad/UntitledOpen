# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

UntitledOpen is a small, standalone cross-platform C/C++ library (repo: `MadLadSquad/UntitledOpen`) that does two things:

1. **Native file pickers** — open/save files, pick single/multiple files, pick single/multiple folders — implemented on top of the bundled `nativefiledialog-extended` (NFD) submodule.
2. **Opening URIs** with the system default application.

It is developed independently but is vendored into UntitledImGuiFramework as a git submodule under `Framework/Modules/OS/ThirdParty/UntitledOpen` (the `os` module). When editing here you are working *inside that submodule* — commits/pushes go to the UntitledOpen repo, not the framework. Full end-user docs live on the [wiki](https://github.com/MadLadSquad/UntitledOpen/wiki/Home); the framework's own CLAUDE.md is one directory tree up and covers the module system that consumes this library.

## Building

The canonical build is `ci.sh` (used verbatim by CI on Linux/Windows/macOS):

```bash
./ci.sh          # mkdir build && cmake .. -DCMAKE_BUILD_TYPE=RELEASE && (MSBuild || make -j)
```

Manual build:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j$(nproc)
```

Requires C++20 / C99, CMake ≥ 3.21, and (Linux/Unix only) `libdbus-1-dev` found via pkg-config. NFD is built as a subdirectory (`add_subdirectory(NFD/)`) and linked as the `nfd` target. There is no test suite; CI only verifies that the library compiles on all three platforms.

Key CMake options:
- `BUILD_VARIANT_STATIC` — build a STATIC library instead of the default SHARED.
- `UIMGUI_INSTALL` — enable the `install()` rules (headers → `include/UntitledOpen`, pkg-config → `lib/pkgconfig`). The framework sets these when integrating.

The build defines `UIMGUI_OPEN_SUBMODULE_ENABLED` (public) so the framework can feature-detect the module.

## Architecture

Three source layers, all thin:

- **`C/CUntitledOpen.{h,cpp}`** — the real implementation of the picker. Everything routes to NFD's `*_With` U8 dialog functions. This is the C ABI and where actual picker logic lives (the big `switch` on `UOpen_PickerOperation` in `UOpen_pickFile`).
- **`UntitledOpen.{hpp,cpp}`** — a C++ RAII wrapper over the C API: `UOpen::Result` (owns the picker result) and `UOpen::UniqueString` (frees NFD-allocated path strings via a stored free-function pointer). `openURI` is the one piece implemented directly in the C++ TU rather than delegating to NFD.
- **`Common.h`** — shared C ABI: enums (`UOpen_Status`, `UOpen_PickerOperation`, `UOpen_WindowHandlePlatform`), the `UOpen_Filter`/`UOpen_Result` structs, and the `MLS_PUBLIC_API` export macro (only expands to `__declspec(dllexport/dllimport)` on Windows when `MLS_EXPORT_LIBRARY` is defined).

`openURI` is fully platform-forked:
- **Windows** → `ShellExecuteA`.
- **macOS** → CoreFoundation/ApplicationServices (`LSOpenCFURLRef`), linked via `-framework CoreFoundation -framework ApplicationServices`.
- **Linux/Unix** → raw libdbus calls to the `org.freedesktop.portal.OpenURI` XDG desktop portal. `file://` URIs are special-cased: they switch to the portal's `OpenFile` method, pass an open FD (`DBUS_TYPE_UNIX_FD`) instead of a string, and set `ask=true`.

## Things to know before editing

- **Memory ownership is manual and platform-shaped.** Picker results hold NFD-allocated memory. On the C side call `UOpen_freeResult`; multiple-selection results are NFD *path sets* freed differently (`NFD_PathSet_Free` / `NFD_PathSet_FreePath`) from single results (`NFD_FreePath`). On the C++ side `Result`/`UniqueString` handle this via stored free-function pointers — don't mix a raw NFD free with a `UniqueString` over the same pointer.
- **The C++ and C `pickFile` argument order differs.** `UOpen::pickFile` takes `defaultName` *before* `defaultPath`; `UOpen_pickFile` takes `defaultPath` *before* `defaultName`. The C++ wrapper reorders them when delegating. Preserve this when touching either signature.
- **Filter passthrough relies on layout compatibility.** `UOpen_Filter` is `reinterpret_cast` directly to NFD's `nfdu8filteritem_t`. The two structs must stay layout-identical.
- **Wayland needs the display handle.** `init(waylandDisplay)` / `updateWaylandDisplay()` forward to `NFD_SetWaylandDisplay`; without it, portal file dialogs can't parent correctly. On Windows/macOS these calls are compiled out.
- **Window parenting** is passed through as `(UOpen_WindowHandlePlatform, void* windowHandle)` into NFD's `parentWindow` field — the platform enum values are cast straight to NFD's `type` field.
- **Event-safety comments** (`Event Safety - begin, style, post-begin`, `Any time`) in the headers refer to the parent framework's lifecycle events. Keep them accurate when adding/moving APIs, matching the convention documented in the framework's CLAUDE.md.
- **`Common.h` is the ABI boundary.** Changing enum values or struct layout breaks both the C API and any prebuilt consumers — treat it as a stable interface.
- **NFD/ is a submodule** — don't edit files under it here; changes belong in `MadLadSquad/nativefiledialog-extended`.

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).
