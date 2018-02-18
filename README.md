Common application tools:

- Platform/compiler/architecture defines + related macros.
- Assert macros (+ callback for app-specific behavior).
- Logging macros (+ callback for app-specific behavior).
- Math types/functions.
- Time functions.
- Hash functions (FNV1a).
- Compression functions.
- File + file system tools.
- Common file format load/parse (image files, JSON, INI).
- Misc useful base/template classes for common idioms (factory, static initializer, etc.).

## Usage ##
The project is intended to be compiled as a static lib. A [premake](https://premake.github.io/) script is provided in `build/` (requires premake5). See [build/ApplicationTools_premake.lua](https://github.com/john-chapman/ApplicationTools/blob/master/build/ApplicationTools_premake.lua) for details.

## Dependencies ##
Embedded dependencies:

- [EASTL](https://github.com/electronicarts/EASTL)
- [linalg](https://github.com/john-chapman/linalg)
- [LodePNG](http://lodev.org/lodepng/)
- [Miniz](https://github.com/richgel999/miniz)
- [RapidJSON](http://rapidjson.org/)
- [stb](https://github.com/nothings/stb)

## Change Log ##
- `2018-02-18 (v0.09):` Json: Direct array access overloads of `setValue()`, `getValue()`, bug fixes.
- `2018-01-21 (v0.08):` FileSystem: "null-separated string" interfaces now use std::initializer_list.
- `2018-01-14 (v0.07):` Type traits + tag dispatch for math utility functions. Moved Min, Max, Clamp, Saturate into math.h.
- `2018-01-06 (v0.06):` Rand API.
- `2017-11-14 (v0.05):` Replaced GLM with linalg.
- `2017-10-07 (v0.04):` Compression API.
- `2017-10-03 (v0.03):` New serialization API, replaced JsonSerializer -> SerializerJson.
- `2017-09-09 (v0.02):` FileSystem::ListFiles/ListDirs take a null-separated list of filter pattern strings.
- `2017-07-26 (v0.01):` Renamed def.h -> apt.h, added version and change log, new premake script.