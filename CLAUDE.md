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
- **`UntitledOpen.{hpp,cpp}`** — a C++ RAII wrapper over the C API: `UOpen::Result` (owns the picker result and frees it on destruction) and `UOpen::UniqueString` (frees a single path string via a stored free-function pointer). All picker access goes through the C API, so this TU includes no NFD headers. `openURI` is the one piece implemented directly here rather than delegating to NFD.
- **`Common.h`** — shared C ABI: enums (`UOpen_Status`, `UOpen_PickerOperation`, `UOpen_WindowHandlePlatform`), the `UOpen_Filter`/`UOpen_Result` structs, and the `MLS_PUBLIC_API` export macro (only expands to `__declspec(dllexport/dllimport)` on Windows when `MLS_EXPORT_LIBRARY` is defined).

`openURI` is fully platform-forked, and every branch returns 0 only when the platform reported success:
- **Windows** → `ShellExecuteW`, with the UTF-8 link converted to UTF-16 first so non-ASCII URIs survive.
- **macOS** → CoreFoundation/ApplicationServices (`LSOpenCFURLRef`), linked via `-framework CoreFoundation -framework ApplicationServices`. The URL is parsed as UTF-8, and a malformed one yields a null `CFURLRef` that must not reach `CFRelease`.
- **Linux/Unix** → raw libdbus calls to the `org.freedesktop.portal.OpenURI` XDG desktop portal. `file://` URIs are special-cased: they switch to the portal's `OpenFile` method, pass an open FD (`DBUS_TYPE_UNIX_FD`) instead of a string, and set `ask=true`. The path is percent-decoded first, only an empty or `localhost` authority is accepted, and the FD is opened read-write when possible but falls back to read-only (with `writable=false`) so unwritable files still open.

## Things to know before editing

- **Memory ownership is manual on the C side, RAII on the C++ side.** Picker results hold NFD-allocated memory. In C you must call `UOpen_freeResult`; multiple-selection results are NFD *path sets* freed differently (`NFD_PathSet_Free` / `NFD_PathSet_FreePath`) from single results (`NFD_FreePath`). In C++ `Result` owns that allocation and releases it in its destructor, so nothing needs freeing by hand — don't mix a raw NFD free with a `Result` or `UniqueString` over the same pointer.
- **`Result` and `UniqueString` are both move-only and independently owning.** `Result` frees the picker allocation exactly once, on destruction; copying is deleted because it would free twice. Its getters are `const`, re-entrant, and never mutate or hand away the result's own pointer: a multiple pick's paths are fresh allocations from the path set, and a single pick's path is copied for the caller. That means a returned `UniqueString` outlives the `Result` it came from and getters may be called any number of times, in any order. `getPath` returns an empty string for an out-of-range index (a single pick only has index 0).
- **`UniqueString` is a return type only.** It has no public constructor that takes a string — it is only produced by `Result`'s getters, which pair the pointer with the matching free function. `destroy()` is private for the same reason: freeing early and then letting the destructor run would free twice. Let it go out of scope.
- **`pickFile` argument order is `defaultPath` then `defaultName`** in both the C++ (`UOpen::pickFile`) and C (`UOpen_pickFile`) APIs — the two now match. `defaultName` is only consumed for `UOPEN_SAVE_FILE`; NFD's folder-pick args have no name field, so it is ignored for folder operations.
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
