//
// Created by Jay Vaughan on 28.03.25.
//
/*
 * LuaTimerInterface.h - Extends LuaInterface with JUCE Timer integration
 */
#ifndef LUATIMERINTERFACE_H
#define LUATIMERINTERFACE_H

#include "LuaInterface.h"

class LuaTimerInterface : public LuaInterface, public juce::Timer {
public:
    LuaTimerInterface() {
        // Start timer with a default interval (e.g., 100ms)
        startTimer(100);
    }

    // Override to load script with timer-related Lua functions
    bool loadScript(const char* script) override {
        const char* timerScript = R"(
            function onTimer()
                print("Timer tick: " .. os.clock())
            end
        )";
        juce::ScopedLock lock(luaLock);
        if (luaL_dostring(L, timerScript) != LUA_OK || luaL_dostring(L, script) != LUA_OK) {
            juce::Logger::writeToLog("Lua init error: " + juce::String(lua_tostring(L, -1)));
            lua_pop(L, 1);
            return false;
        }
        juce::Logger::writeToLog("Lua script with timer support loaded successfully");
        return true;
    }

    // JUCE Timer callback interfacing with Lua
    void timerCallback() override {
        juce::ScopedLock lock(luaLock);
        lua_getglobal(L, "onTimer");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                const char* err = lua_tostring(L, -1);
                juce::Logger::writeToLog("Lua error in onTimer: " + juce::String(err ? err : "Unknown error"));
                lua_pop(L, 1);
            }
        } else {
            lua_pop(L, 1); // Pop nil or non-function value
        }
    }
};

#endif // LUATIMERINTERFACE_H
