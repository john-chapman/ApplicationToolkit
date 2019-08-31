--[[
	Usage #1: External Project File
	-------------------------------
	To use one of the prebuilt project files, call dofile() at the top of your premake script:

		dofile("extern/ApplicationTools/build/ApplicationTools_premake.lua")

	Then call ApplicationTools_ProjectExternal() inside your workspace declaration:

		workspace "MyWorkspace"
			ApplicationTools_ProjectExternal("extern/ApplicationTools")

	Finally, for each project which needs to link ApplicationTools:

		project "MyProject"
			ApplicationTools_Link()

	This is the least flexible of the two options but has the advantage of being able to update ApplicationTools without rebuilding your project files.

	Usage #2: Local Project File
	----------------------------
	To customize the project, call dofile() at the top of your premake script:

		dofile("extern/ApplicationTools/build/ApplicationTools_premake.lua")

	Then call ApplicationTools_Project() inside your workspace declaration:

		workspace "MyWorkspace"
			ApplicationTools_Project(
				"extern/ApplicationTools", -- lib root
				"build/lib",               -- build output location
				{                          -- config map
					APT_LOG_CALLBAG_ONLY = 1,
				})

	See config.h for more info about configuring the library.
	
	Finally, for each project which needs to link ApplicationTools:

		project "MyProject"
			ApplicationTools_Link()

	This option provides the most flexibility, but don't forget to rebuild your project files after updating.
--]]

local APT_UUID          = "6ADD11F4-56D6-3046-7F08-16CB6B601052"

local SRC_DIR           = "/src/"
local ALL_SRC_DIR       = SRC_DIR       .. "all/"
local ALL_EXTERN_DIR    = ALL_SRC_DIR   .. "extern/"
local WIN_SRC_DIR       = SRC_DIR       .. "win/"
local WIN_EXTERN_DIR    = WIN_SRC_DIR   .. "extern/"

local function ApplicationTools_SetPaths(_root)
	SRC_DIR         = _root .. SRC_DIR
	ALL_SRC_DIR     = _root .. ALL_SRC_DIR
	ALL_EXTERN_DIR  = _root .. ALL_EXTERN_DIR
	WIN_SRC_DIR     = _root .. WIN_SRC_DIR
	WIN_EXTERN_DIR  = _root .. WIN_EXTERN_DIR
end


local function ApplicationTools_ProjectCommon()
	kind       "StaticLib"
	language   "C++"
	cppdialect "C++14"
	uuid       "6ADD11F4-56D6-3046-7F08-16CB6B601052"
end

local function ApplicationTools_Globals()
	project "*"

	defines { "EA_COMPILER_NO_EXCEPTIONS" }
	rtti "Off"
	exceptionhandling "Off"

	filter { "configurations:debug" }
		defines { "APT_DEBUG" }
	filter {}

	filter { "action:vs*" }
		defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
		buildoptions { "/EHsc" }
		characterset "MBCS" -- force Win32 API to use *A variants (i.e. can pass char* for strings)
	filter {}

	includedirs({
		ALL_SRC_DIR,
		ALL_EXTERN_DIR,
		})
	filter { "platforms:Win*" }
		includedirs({
			WIN_SRC_DIR,
			WIN_EXTERN_DIR,
			})
	filter {}
end

function ApplicationTools_Project(_root, _targetDir, _config)
	_root      = _root or ""
	_targetDir = _targetDir or "../lib"
	_config    = _config or {}

	ApplicationTools_SetPaths(_root)

	project "ApplicationTools"
		targetdir(_targetDir)
		ApplicationTools_ProjectCommon()

		vpaths({
			["*"]        = ALL_SRC_DIR    .. "apt/**",
			["extern/*"] = ALL_EXTERN_DIR .. "**",
			["win"]      = WIN_SRC_DIR    .. "apt/**",
			})

		files({
			ALL_SRC_DIR    .. "**.h",
			ALL_SRC_DIR    .. "**.hpp",
			ALL_SRC_DIR    .. "**.c",
			ALL_SRC_DIR    .. "**.cpp",
			ALL_EXTERN_DIR .. "**.h",
			ALL_EXTERN_DIR .. "**.hpp",
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
		filter {}

		for k,v in pairs(_config) do
			if v then
				defines { tostring(k) .. "=" .. tostring(v) }
			end
		end

	ApplicationTools_Globals()
end

function ApplicationTools_ProjectExternal(_root)
	_root = _root or ""

	ApplicationTools_SetPaths(_root)

	externalproject "ApplicationTools"
		location(_root .. "/build/" .. _ACTION)
		ApplicationTools_ProjectCommon()

	ApplicationTools_Globals(_root)
end

function ApplicationTools_Link()
	links { "ApplicationTools" }

	filter { "platforms:Win*" }
		links { "shlwapi" }
	filter {}
end
