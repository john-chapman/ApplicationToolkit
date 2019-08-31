dofile "ApplicationTools_premake.lua"

workspace "ApplicationTools"
	location(_ACTION)
	platforms { "Win64" }
	staticruntime "On"
	filter { "platforms:Win64" }
		system "windows"
		architecture "x86_64"
	filter {}

	configurations { "Debug", "Release" }
	filter { "configurations:Debug" }
		targetsuffix "_debug"
		symbols "On"
		optimize "Off"
	filter {}
	filter { "configurations:Release" }
		symbols "Off"
		optimize "Full"
	filter {}
	
	group "libs"
		ApplicationTools_Project(
			"../",    -- _root
			"../lib", -- _targetDir
			{
				--APT_LOG_CALLBACK_ONLY = 1,
			})
	group ""
	
	project "ApplicationTools_Tests"
		kind "ConsoleApp"
		language "C++"
		targetdir "../bin"

		local TESTS_DIR         = "../tests/"
		local TESTS_EXTERN_DIR  = TESTS_DIR .. "extern/"
		includedirs { TESTS_DIR, TESTS_EXTERN_DIR }
		files({
			TESTS_DIR .. "**.h",
			TESTS_DIR .. "**.hpp",
			TESTS_DIR .. "**.c",
			TESTS_DIR .. "**.cpp",
			})
		removefiles({
			TESTS_EXTERN_DIR .. "**.h",
			TESTS_EXTERN_DIR .. "**.hpp",
			})

		ApplicationTools_Link()
