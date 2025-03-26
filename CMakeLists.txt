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

# Lua source files (corrected for lua/lua repo structure)
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
        ${lua_SOURCE_DIR}
)

# Link libraries
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

# On macOS, modify the shared code target's link command to force load lua
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_custom_command(TARGET LuaParamaBangPlugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Modifying link flags for LuaParamaBangPlugin_SharedCode"
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/CMakeFiles/LuaParamaBangPlugin_SharedCode.dir/link.txt
            ${CMAKE_BINARY_DIR}/CMakeFiles/LuaParamaBangPlugin_SharedCode.dir/link.txt.bak
        COMMAND sed -i '' 's|-lLuaParamaBangPlugin_SharedCode|-Wl,-force_load,${CMAKE_BINARY_DIR}/liblua.a|' ${CMAKE_BINARY_DIR}/CMakeFiles/LuaParamaBangPlugin_SharedCode.dir/link.txt
        COMMENT "Forcing Lua symbols into LuaParamaBangPlugin_SharedCode"
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
