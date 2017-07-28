--[[
	Usage #1: External Project File
	-------------------------------
	To use one of the bundled project files, call dofile() at the top of your premake script:
		
		dofile("extern/ApplicationTools/build/ApplicationTools_premake.lua")
		
	The call ApplicationTools_ProjectExternal() inside your workspace declaration:
		
		workspace "MyWorkspace"
			function ApplicationTools_External("extern/ApplicationTools")
			
	Finally, for each project which needs to link ApplicationTools:
		
		project "MyProject"
			ApplicationTools_Link()
			
	This is the least flexible of the two options but has the advantage of being able to update ApplicationTools without rebuilding your project files.
	
	Usage #2: Local Project File
	----------------------------
	To customize the project, specify config options (see the list below), then call dofile() at the top of your premake script:
		
		APT_LOG_CALLBACK_ONLY = true
		dofile("extern/ApplicationTools/build/ApplicationTools_premake.lua")
		
	Then call ApplicationTools_Project() inside your workspace declaration:
		
		workspace "MyWorkspace"
			ApplicationTools_Project(
				"extern/ApplicationTools", -- lib root
				"build/lib"                -- build output location
				)
				
	Finally, for each project which needs to link ApplicationTools:
		project "MyProject"
			ApplicationTools_Link()
	
	This option provides the most flexibility, but don't forget to rebuild your project files after updating.

	Config Options
	--------------
	APT_LOG_CALLBACK_ONLY   - Disable APT_LOG* writing to stdout/stderr (default: false).
--]]

local APT_UUID        = "6ADD11F4-56D6-3046-7F08-16CB6B601052"
local SRC_DIR         = "/src/"
local ALL_SRC_DIR     = SRC_DIR .. "all/"
local ALL_EXTERN_DIR  = ALL_SRC_DIR .. "extern/"
local WIN_SRC_DIR     = SRC_DIR .. "win/"
local WIN_EXTERN_DIR  = WIN_SRC_DIR .. "extern/"

local function ApplicationTools_Common(_root)
	SRC_DIR         = _root .. SRC_DIR
	ALL_SRC_DIR     = _root .. ALL_SRC_DIR
	ALL_EXTERN_DIR  = _root .. ALL_EXTERN_DIR
	WIN_SRC_DIR     = _root .. WIN_SRC_DIR
	WIN_EXTERN_DIR  = _root .. WIN_EXTERN_DIR

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

function ApplicationTools_Project(_root, _targetDir)
	_root   = _root or ""
	_targetDir = _targetDir or "../lib"

	ApplicationTools_Common(_root)

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

function ApplicationTools_ProjectExternal(_root)
	_root = _root or ""

	ApplicationTools_Common(_root)

	externalproject "ApplicationTools"
		location(_root .. "build/" .. _ACTION)
		uuid(APT_UUID)
		kind "StaticLib"
		language "C++"
end

function ApplicationTools_Link()
	links { "ApplicationTools" }

	filter { "platforms:Win*" }
		links { "shlwapi" }
end
