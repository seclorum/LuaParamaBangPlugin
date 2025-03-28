//
// Created by Jay Vaughan on 28.03.25.
//
/*
 * LuaInterface.h - Base class for managing Lua VM and common functionality, fixed for Lua 5.3
 */
#ifndef LUAINTERFACE_H
#define LUAINTERFACE_H

#include <juce_audio_processors/juce_audio_processors.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
};

// Base class for Lua integration, designed to be extensible
class LuaInterface {
protected:
    lua_State* L;                            // Lua VM state
    juce::CriticalSection luaLock;           // Thread safety for Lua access
    juce::AudioProcessorValueTreeState* apvts; // Pointer to the processor's APVTS

public:
    LuaInterface() : L(nullptr), apvts(nullptr) {
        // Initialize Lua state with proper Lua 5.3 headers
        L = luaL_newstate();
        if (!L) {
            juce::Logger::writeToLog("Fatal: Failed to create Lua state");
            return;
        }
        luaL_openlibs(L); // Open standard libraries
    }

    virtual ~LuaInterface() {
        // Clean up Lua state
        if (L) {
            lua_close(L);
            L = nullptr;
        }
    }

    // Initialize Lua with processor instance and APVTS
    void initializeLua(juce::AudioProcessor* processor, juce::AudioProcessorValueTreeState* apvtsPtr) {
        juce::ScopedLock lock(luaLock);
        if (!processor || !apvtsPtr) {
            juce::Logger::writeToLog("Error: Null processor or APVTS passed to initializeLua");
            return;
        }
        lua_pushlightuserdata(L, processor);
        lua_setfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");

        // Store the APVTS pointer
        apvts = apvtsPtr;

        // Register C functions
        lua_pushcfunction(L, &LuaInterface::luaGetParam);
        lua_setglobal(L, "getParam");
        lua_pushcfunction(L, &LuaInterface::luaSetParam);
        lua_setglobal(L, "setParam");
    }

    // Load a Lua script, virtual for extensibility
    virtual bool loadScript(const char* script) {
        juce::ScopedLock lock(luaLock);
        if (!script) {
            juce::Logger::writeToLog("Error: Null script passed to loadScript");
            return false;
        }
        if (luaL_dostring(L, script) != 0) {
            const char* err = lua_tostring(L, -1);
            juce::Logger::writeToLog("Lua init error: " + juce::String(err ? err : "Unknown error"));
            lua_pop(L, 1);
            return false;
        }
        juce::Logger::writeToLog("Lua script loaded successfully");
        return true;
    }

    // Call a Lua function with specified arguments
    void callLuaFunction(const char* funcName, int numArgs, ...) {
        juce::ScopedLock lock(luaLock);
        if (!funcName) {
            juce::Logger::writeToLog("Error: Null function name in callLuaFunction");
            return;
        }
        lua_getglobal(L, funcName);
        if (lua_isfunction(L, -1)) {
            va_list args;
            va_start(args, numArgs);
            for (int i = 0; i < numArgs; ++i) {
                lua_pushnumber(L, static_cast<lua_Number>(va_arg(args, double)));
            }
            va_end(args);
            if (lua_pcall(L, numArgs, 0, 0) != 0) {
                const char* err = lua_tostring(L, -1);
                juce::Logger::writeToLog("Lua error in " + juce::String(funcName) + ": " + juce::String(err ? err : "Unknown error"));
                lua_pop(L, 1);
            }
        } else {
            juce::Logger::writeToLog(juce::String(funcName) + " not found or not a function");
            lua_pop(L, 1);
        }
    }

    // Static Lua C function for getting parameter values
    static int luaGetParam(lua_State* L) {
        if (lua_gettop(L) < 1) {
            lua_pushstring(L, "luaGetParam: Parameter ID required");
            lua_error(L);
            return 0;
        }

        lua_getfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");
        auto* processor = static_cast<juce::AudioProcessor*>(lua_touserdata(L, -1));
        if (!processor) {
            juce::Logger::writeToLog("Error: LuaPluginProcessor instance not found in registry.");
            lua_pushnumber(L, 0.0f);
            return 1;
        }
        lua_pop(L, 1);

        const char* paramId = luaL_checkstring(L, 1);
        // Access APVTS through the LuaInterface instance
        LuaInterface* luaInterface = dynamic_cast<LuaInterface*>(processor);
        if (!luaInterface || !luaInterface->apvts) {
            juce::Logger::writeToLog("Error: Processor does not inherit from LuaInterface or APVTS not initialized");
            lua_pushnumber(L, 0.0f);
            return 1;
        }
        juce::ScopedLock lock(luaInterface->luaLock);
        if (auto* param = luaInterface->apvts->getRawParameterValue(paramId)) {
            float value = param->load();
            lua_pushnumber(L, static_cast<lua_Number>(value));
            return 1;
        }
        juce::Logger::writeToLog("Error: Invalid parameter ID in luaGetParam: " + juce::String(paramId));
        lua_pushnumber(L, 0.0f);
        return 1;
    }

    // Static Lua C function for setting parameter values
    static int luaSetParam(lua_State* L) {
        if (lua_gettop(L) < 2) {
            lua_pushstring(L, "luaSetParam: Parameter ID and value required");
            lua_error(L);
            return 0;
        }

        lua_getfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");
        auto* processor = static_cast<juce::AudioProcessor*>(lua_touserdata(L, -1));
        if (!processor) {
            juce::Logger::writeToLog("Error: LuaPluginProcessor instance not found in registry.");
            return 0;
        }
        lua_pop(L, 1);

        const char* paramId = luaL_checkstring(L, 1);
        lua_Number value = luaL_checknumber(L, 2);
        LuaInterface* luaInterface = dynamic_cast<LuaInterface*>(processor);
        if (!luaInterface || !luaInterface->apvts) {
            juce::Logger::writeToLog("Error: Processor does not inherit from LuaInterface or APVTS not initialized");
            return 0;
        }
        juce::ScopedLock lock(luaInterface->luaLock);
        if (auto* param = luaInterface->apvts->getParameter(paramId)) {
            param->setValueNotifyingHost(static_cast<float>(value) / 127.0f);
            juce::Logger::writeToLog("luaSetParam set " + juce::String(paramId) + " to " + juce::String(value));
        }
        return 0;
    }
};

#endif // LUAINTERFACE_H