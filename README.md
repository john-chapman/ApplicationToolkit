Common application tools:

- Platform/compiler/architecture defines + related macros.
- Assert/verify macros (+ callback for app-specific behavior).
- Time functions.
- Hash functions (FNV1a).
- File + file system tools.
- Common file format load/parse (image files, JSON, INI).
- Misc useful base classes for common idioms (factory, static initializer, etc.).

Currently only supporting Windows, but the project is designed to be cross-platform.

## Usage ##
The project is intended to be compiled as a static lib. A [premake](https://premake.github.io/) script is provided in `build/` (requires premake5).

## Dependencies ##
Committed dependencies:

- [EASTL](https://github.com/electronicarts/EASTL)
- [GLM](https://github.com/g-truc/glm)
- [LodePNG](http://lodev.org/lodepng/)
- [RapidJSON](http://rapidjson.org/)
- [stb](https://github.com/nothings/stb)
