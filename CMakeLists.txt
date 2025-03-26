cmake_minimum_required(VERSION 3.15)
project(LuaParamaBangPlugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
 juce 
 GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
 GIT_TAG master
)
FetchContent_MakeAvailable(juce)

# Fetch and build Lua 5.3
FetchContent_Declare(
 lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.3.6
)
FetchContent_MakeAvailable(lua)

# Lua source files (adjusted for walterschell/Lua repo structure)
set(LUA_SRC
 ${lua_SOURCE_DIR}/src/lapi.c
 ${lua_SOURCE_DIR}/src/lauxlib.c
 ${lua_SOURCE_DIR}/src/lbaselib.c
 ${lua_SOURCE_DIR}/src/lbitlib.c
 ${lua_SOURCE_DIR}/src/lcode.c
 ${lua_SOURCE_DIR}/src/lcorolib.c
 ${lua_SOURCE_DIR}/src/lctype.c
 ${lua_SOURCE_DIR}/src/ldblib.c
 ${lua_SOURCE_DIR}/src/ldebug.c
 ${lua_SOURCE_DIR}/src/ldo.c
 ${lua_SOURCE_DIR}/src/ldump.c
 ${lua_SOURCE_DIR}/src/lfunc.c
 ${lua_SOURCE_DIR}/src/lgc.c
 ${lua_SOURCE_DIR}/src/linit.c
 ${lua_SOURCE_DIR}/src/liolib.c
 ${lua_SOURCE_DIR}/src/llex.c
 ${lua_SOURCE_DIR}/src/lmathlib.c
 ${lua_SOURCE_DIR}/src/lmem.c
 ${lua_SOURCE_DIR}/src/loadlib.c
 ${lua_SOURCE_DIR}/src/lobject.c
 ${lua_SOURCE_DIR}/src/lopcodes.c
 ${lua_SOURCE_DIR}/src/loslib.c
 ${lua_SOURCE_DIR}/src/lparser.c
 ${lua_SOURCE_DIR}/src/lstate.c
 ${lua_SOURCE_DIR}/src/lstring.c
 ${lua_SOURCE_DIR}/src/lstrlib.c
 ${lua_SOURCE_DIR}/src/ltable.c
 ${lua_SOURCE_DIR}/src/ltablib.c
 ${lua_SOURCE_DIR}/src/ltm.c
 ${lua_SOURCE_DIR}/src/lundump.c
 ${lua_SOURCE_DIR}/src/lutf8lib.c
 ${lua_SOURCE_DIR}/src/lvm.c
 ${lua_SOURCE_DIR}/src/lzio.c
)

# Build Lua as a static library
add_library(lua STATIC ${LUA_SRC})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/src)
target_compile_definitions(lua PRIVATE LUA_USE_C89)

# macOS specific settings
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
 target_compile_definitions(lua PRIVATE
 LUA_USE_MACOSX
 LUA_USE_DLOPEN
 )
 set_target_properties(lua PROPERTIES
 VISIBILITY_INLINES_HIDDEN OFF
 C_VISIBILITY_PRESET default
 )
endif()

set_target_properties(lua PROPERTIES
 C_STANDARD 11
 C_STANDARD_REQUIRED ON
 POSITION_INDEPENDENT_CODE ON
)

# Add plugin
juce_add_plugin(LuaParamaBangPlugin
 COMPANY_NAME "warblaydrInc"
 IS_SYNTH FALSE
 NEEDS_MIDI_INPUT TRUE
 NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE
 EDITOR_WANTS_KEYBOARD_FOCUS FALSE
 COPY_PLUGIN_AFTER_BUILD TRUE
 PLUGIN_MANUFACTURER_CODE WRBL
 PLUGIN_CODE LPBP
 FORMATS AU VST3 Standalone
 PRODUCT_NAME "LuaParamaBangPlugin"
)

# Source files
target_sources(LuaParamaBangPlugin
 PRIVATE
 Source/PluginProcessor.cpp
 Source/PluginEditor.cpp
)

# Include directories
target_include_directories(LuaParamaBangPlugin
 PRIVATE
 Source
 ${lua_SOURCE_DIR}/src
 )

# Link libraries with proper macOS handling
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
 target_link_libraries(LuaParamaBangPlugin
 PRIVATE
 juce::juce_audio_utils
 juce::juce_audio_processors
 PUBLIC
 -Wl,-force_load,$<TARGET_FILE:lua>
 juce::juce_recommended_config_flags
 juce::juce_recommended_lto_flags
 juce::juce_recommended_warning_flags
 )
else()
 target_link_libraries(LuaParamaBangPlugin
 PRIVATE
 juce::juce_audio_utils
 juce::juce_audio_processors
 PUBLIC
 lua
 juce::juce_recommended_config_flags
 juce::juce_recommended_lto_flags
 juce::juce_recommended_warning_flags
 )
endif()

# Compile definitions
target_compile_definitions(LuaParamaBangPlugin
 PUBLIC
 JUCE_WEB_BROWSER=0
 JUCE_USE_CURL=0
 JUCE_VST3_CAN_REPLACE_VST2=0
)

# Ensure JUCE modules are properly included
target_include_directories(LuaParamaBangPlugin PRIVATE ${juce_SOURCE_DIR}/modules)
