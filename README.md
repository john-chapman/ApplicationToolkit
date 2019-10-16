**This project has been merged into [GfxSampleFramework](https://github.com/john-chapman/GfxSampleFramework) and is no longer maintained.**

---
Common application tools:

- Platform, compiler and architecture defines + related macros.
- Assert macros (+ callback for app-specific behavior).
- Logging macros (+ callback for app-specific behavior).
- Math types/functions.
- Time functions.
- Hash functions (FNV1a).
- Compression functions (zlib)
- File and file system tools.
- Common file format load/parse (image files, JSON).
- Misc useful base/template classes for common idioms (factory, static initializer, etc.).

## Usage ##
The project is intended to be compiled as a static lib. A [premake](https://premake.github.io/) script is provided in `build/` (requires premake5). See [build/ApplicationTools_premake.lua](build/ApplicationTools_premake.lua) for details.

## Dependencies ##
Embedded dependencies:

- [EASTL](https://github.com/electronicarts/EASTL)
- [linalg](https://github.com/john-chapman/linalg)
- [LodePNG](http://lodev.org/lodepng/)
- [Miniz](https://github.com/richgel999/miniz)
- [RapidJSON](http://rapidjson.org/)
- [stb](https://github.com/nothings/stb)
- [tinyexr](https://github.com/syoyo/tinyexr)

## Change Log ##
- `2019-07-03 (v0.20):` File: fixed setData/appendData when _data is nullptr.
- `2019-03-31 (v0.19):` String: removed internal use of `strncpy` to support strings which contain internal null chars.
- `2018-09-15 (v0.18):` Compile fixes for GNU. Removed `Ini`. Renamed `static_initializer` -> `StaticInitializer`.
- `2018-09-07 (v0.17):` Improved `static_initializer` to allow use of private static functions for init/shutdown. 
- `2018-09-07 (v0.16):` `FileSystem` supports arbitrary search paths (roots), removed `RootType` enum.
- `2018-07-12 (v0.15):` Rewrite of the `Json` implementation.
- `2018-04-17 (v0.14):` `File::Read` sleep/retry on sharing violation (Windows).
- `2018-04-01 (v0.13):` Memory alloc/free API via `APT_` macros.
- `2018-03-31 (v0.12):` FileSystem notifications API. Path manipulation API changes, `FileSystem::PathStr` -> `apt::PathStr`.
- `2018-03-27 (v0.11):` DateTime conversions between local and UTC.
- `2018-02-22 (v0.10):` Deprecated macros APT_ALIGNOF, APT_ALIGNAS, APT_THREAD_LOCAL. Minor fixes/cleaning.
- `2018-02-18 (v0.09):` Json: Direct array access overloads of `setValue()`, `getValue()`, bug fixes.
- `2018-01-21 (v0.08):` FileSystem: "null-separated string" interfaces now use std::initializer_list.
- `2018-01-14 (v0.07):` Type traits + tag dispatch for math utility functions. Moved Min, Max, Clamp, Saturate into math.h.
- `2018-01-06 (v0.06):` Rand API.
- `2017-11-14 (v0.05):` Replaced GLM with linalg.
- `2017-10-07 (v0.04):` Compression API.
- `2017-10-03 (v0.03):` New serialization API, replaced JsonSerializer -> SerializerJson.
- `2017-09-09 (v0.02):` FileSystem::ListFiles/ListDirs take a null-separated list of filter pattern strings.
- `2017-07-26 (v0.01):` Renamed def.h -> apt.h, added version and change log, new premake script.
