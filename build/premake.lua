--APT_LOG_CALLBACK_ONLY = true
dofile("ApplicationTools_premake.lua")

workspace "ApplicationTools"
	location(_ACTION)
	configurations { "Debug", "Release" }
	platforms { "Win64" }
	flags { "C++11", "StaticRuntime" }
	filter { "platforms:Win64" }
		system "windows"
		architecture "x86_64"

	group "libs"
		ApplicationTools_Project(
			"../",    -- _libRoot
			"../lib"  -- _targetDir
			)
	group ""

	project "ApplicationTools_Tests"
		kind "ConsoleApp"
		language "C++"
		targetdir "../bin"

		filter { "configurations:debug" }
			targetsuffix "_debug"
			symbols "On"
			optimize "Off"

		filter { "configurations:release" }
			symbols "Off"
			optimize "Full"

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
