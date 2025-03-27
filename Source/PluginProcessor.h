#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>


extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class LuaPluginProcessor : public juce::AudioProcessor
{
public:
    LuaPluginProcessor();
    ~LuaPluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LuaPlugin"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    lua_State* getLuaState() const { return L; }

private:
    juce::AudioProcessorValueTreeState apvts;
    lua_State* L;

    static int luaGetParam(lua_State* L);
    static int luaSetParam(lua_State* L);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LuaPluginProcessor)
};
