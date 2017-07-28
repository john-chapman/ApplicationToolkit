Common application tools:

- Platform/compiler/architecture defines + related macros.
- Assert macros (+ callback for app-specific behavior).
- Logging macros (+ callback for app-specific behavior).
- Time functions.
- Hash functions (FNV1a).
- File + file system tools.
- Common file format load/parse (image files, JSON, INI).
- Misc useful base classes for common idioms (factory, static initializer, etc.).

## Usage ##
The project is intended to be compiled as a static lib. A [premake](https://premake.github.io/) script is provided in `build/` (requires premake5). See [build/ApplicationTools_premake.lua](https://github.com/john-chapman/ApplicationTools/blob/master/build/ApplicationTools_premake.lua) for details.

## Dependencies ##
Committed dependencies:

- [EASTL](https://github.com/electronicarts/EASTL)
- [GLM](https://github.com/g-truc/glm)
- [LodePNG](http://lodev.org/lodepng/)
- [RapidJSON](http://rapidjson.org/)
- [stb](https://github.com/nothings/stb)

## Change Log ##
- `2017-07-26 (v1.01):` Renamed def.h -> apt.h, added version and change log, new premake script.