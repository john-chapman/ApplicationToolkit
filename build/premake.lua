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
		ApplicationTools("../")
	group ""
		
	project "ApplicationTools_Tests"
		kind "ConsoleApp"
		language "C++"
		targetdir "../bin"
		uuid "DC3DA4C6-C837-CD18-B1A4-63299D3D3385"
		
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
			
		links { "ApplicationTools" }
		filter { "platforms:Win*" }
			links { "shlwapi" }
