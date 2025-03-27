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

# Verify lua_SOURCE_DIR is populated
if(NOT lua_SOURCE_DIR)
    message(FATAL_ERROR "lua_SOURCE_DIR is not set. FetchContent failed to retrieve Lua.")
endif()
message(STATUS "lua_SOURCE_DIR is set to: ${lua_SOURCE_DIR}")

# Check if a key Lua source file exists
if(NOT EXISTS "${lua_SOURCE_DIR}/lapi.c")
    message(FATAL_ERROR "Lua source file ${lua_SOURCE_DIR}/lapi.c not found. Check repository and tag.")
endif()
message(STATUS "Found Lua source file: ${lua_SOURCE_DIR}/lapi.c")

# Lua source files
set(LUA_SRC
        ${lua_SOURCE_DIR}/lapi.c
        ${lua_SOURCE_DIR}/lauxlib.c
        ${lua_SOURCE_DIR}/lbaselib.c
        ${lua_SOURCE_DIR}/lbitlib.c
        ${lua_SOURCE_DIR}/lcode.c
        ${lua_SOURCE_DIR}/lcorolib.c
        ${lua_SOURCE_DIR}/lctype.c
        ${lua_SOURCE_DIR}/ldblib.c
        ${lua_SOURCE_DIR}/ldebug.c
        ${lua_SOURCE_DIR}/ldo.c
        ${lua_SOURCE_DIR}/ldump.c
        ${lua_SOURCE_DIR}/lfunc.c
        ${lua_SOURCE_DIR}/lgc.c
        ${lua_SOURCE_DIR}/linit.c
        ${lua_SOURCE_DIR}/liolib.c
        ${lua_SOURCE_DIR}/llex.c
        ${lua_SOURCE_DIR}/lmathlib.c
        ${lua_SOURCE_DIR}/lmem.c
        ${lua_SOURCE_DIR}/loadlib.c
        ${lua_SOURCE_DIR}/lobject.c
        ${lua_SOURCE_DIR}/lopcodes.c
        ${lua_SOURCE_DIR}/loslib.c
        ${lua_SOURCE_DIR}/lparser.c
        ${lua_SOURCE_DIR}/lstate.c
        ${lua_SOURCE_DIR}/lstring.c
        ${lua_SOURCE_DIR}/lstrlib.c
        ${lua_SOURCE_DIR}/ltable.c
        ${lua_SOURCE_DIR}/ltablib.c
        ${lua_SOURCE_DIR}/ltm.c
        ${lua_SOURCE_DIR}/lundump.c
        ${lua_SOURCE_DIR}/lutf8lib.c
        ${lua_SOURCE_DIR}/lvm.c
        ${lua_SOURCE_DIR}/lzio.c
)

# Build Lua as a static library
add_library(lua STATIC ${LUA_SRC})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})
target_compile_definitions(lua PRIVATE LUA_USE_C89)
set_target_properties(lua PROPERTIES LINKER_LANGUAGE C)

# macOS specific settings for Lua
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(lua PRIVATE
            LUA_USE_MACOSX
#            LUA_USE_DLOPEN
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
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Create an intermediate static library combining plugin code and Lua
add_library(LuaParamaBangPluginCore STATIC
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)
target_include_directories(LuaParamaBangPluginCore
        PRIVATE
        Source
        ${lua_SOURCE_DIR}
        ${juce_SOURCE_DIR}/modules
)
target_link_libraries(LuaParamaBangPluginCore
        PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        PUBLIC
        $<$<PLATFORM_ID:Darwin>:-Wl,-force_load,$<TARGET_FILE:lua>>
        $<$<NOT:$<PLATFORM_ID:Darwin>>:lua>
)
target_compile_definitions(LuaParamaBangPluginCore
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
# Ensure lua is built before LuaParamaBangPluginCore
add_dependencies(LuaParamaBangPluginCore lua)

# Add plugin, linking to the combined core library
juce_add_plugin(LuaParamaBangPlugin
        COMPANY_NAME "github.com/seclorum"
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

# Include directories for the plugin
target_include_directories(LuaParamaBangPlugin
        PRIVATE
        Source
        ${lua_SOURCE_DIR}
        ${juce_SOURCE_DIR}/modules
)

# Link the plugin to the core library and JUCE recommended flags
target_link_libraries(LuaParamaBangPlugin
        PRIVATE
        LuaParamaBangPluginCore
        juce::juce_audio_utils
        juce::juce_audio_devices
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Compile definitions for the plugin
target_compile_definitions(LuaParamaBangPlugin
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)