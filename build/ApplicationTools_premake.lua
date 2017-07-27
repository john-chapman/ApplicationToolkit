-- Options (define as globals prior to dofile(ApplicationTools_premake.lua)
-- APT_LOG_CALLBACK_ONLY   - disable APT_LOG* writing to stdout/stderr by default

local APT_UUID        = "6ADD11F4-56D6-3046-7F08-16CB6B601052"
local SRC_DIR         = "/src/"
local ALL_SRC_DIR     = SRC_DIR .. "all/"
local ALL_EXTERN_DIR  = ALL_SRC_DIR .. "extern/"
local WIN_SRC_DIR     = SRC_DIR .. "win/"
local WIN_EXTERN_DIR  = WIN_SRC_DIR .. "extern/"

local function ApplicationTools_Common(_libRoot)
	SRC_DIR         = _libRoot .. SRC_DIR
	ALL_SRC_DIR     = _libRoot .. ALL_SRC_DIR
	ALL_EXTERN_DIR  = _libRoot .. ALL_EXTERN_DIR
	WIN_SRC_DIR     = _libRoot .. WIN_SRC_DIR
	WIN_EXTERN_DIR  = _libRoot .. WIN_EXTERN_DIR

	defines { "EA_COMPILER_NO_EXCEPTIONS" }
	rtti "Off"
	exceptionhandling "Off"

	filter { "configurations:debug" }
		defines { "APT_DEBUG" }

	filter { "action:vs*" }
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
		buildoptions { "/EHsc" }
		characterset "MBCS" -- force Win32 API to use *A variants (i.e. can pass char* for strings)

	includedirs({
		ALL_SRC_DIR,
		ALL_EXTERN_DIR,
		})
	filter { "platforms:Win*" }
		includedirs({
			WIN_SRC_DIR,
			WIN_EXTERN_DIR,
			})
end

function ApplicationTools_Project(_libRoot, _targetDir)
	_libRoot   = _libRoot or ""
	_targetDir = _targetDir or "../lib"

	ApplicationTools_Common(_libRoot)

	project "ApplicationTools"
		kind "StaticLib"
		language "C++"
		targetdir(_targetDir)
		uuid(APT_UUID)

		filter { "configurations:debug" }
			targetsuffix "_debug"
			symbols "On"
			optimize "Off"

		filter { "configurations:release" }
			symbols "Off"
			optimize "Full"

		vpaths({
			["*"]        = ALL_SRC_DIR .. "apt/**",
			["extern/*"] = ALL_EXTERN_DIR .. "**",
			["win"]      = WIN_SRC_DIR .. "apt/**",
			})

		files({
			ALL_SRC_DIR    .. "**.h",
			ALL_SRC_DIR    .. "**.hpp",
			ALL_SRC_DIR    .. "**.c",
			ALL_SRC_DIR    .. "**.cpp",
			ALL_EXTERN_DIR .. "**.c",
			ALL_EXTERN_DIR .. "**.cpp",
			ALL_EXTERN_DIR .. "**.natvis",
			})
		removefiles({
			ALL_EXTERN_DIR .. "glm/**",
			ALL_EXTERN_DIR .. "rapidjson/**",
			})
		filter { "platforms:Win*" }
			files({
				WIN_SRC_DIR    .. "**.h",
				WIN_SRC_DIR    .. "**.hpp",
				WIN_SRC_DIR    .. "**.c",
				WIN_SRC_DIR    .. "**.cpp",
				WIN_EXTERN_DIR .. "**.c",
				WIN_EXTERN_DIR .. "**.cpp",
				})

		if (APT_LOG_CALLBACK_ONLY or false) then defines { "APT_LOG_CALLBACK_ONLY" } end
end

function ApplicationTools_ProjectExternal(_libRoot)
	_libRoot = _libRoot or ""

	ApplicationTools_Common(_libRoot)

	externalproject "ApplicationTools"
		location(_libRoot .. "build/" .. _ACTION)
		uuid(APT_UUID)
		kind "StaticLib"
		language "C++"
end

function ApplicationTools_Link()
	links { "ApplicationTools" }

	filter { "platforms:Win*" }
		links { "shlwapi" }
end
