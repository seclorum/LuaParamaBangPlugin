#include "PluginProcessor.h"
#include "PluginEditor.h"


using namespace juce;

static const int PARAMETER_V1 = 1;

LuaPluginProcessor::LuaPluginProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                     .withOutput("Output", juce::AudioChannelSet::stereo())),
      apvts(*this, nullptr, "PARAMETERS", {
          std::make_unique<juce::AudioParameterInt>(ParameterID {"volume", PARAMETER_V1}, "Volume", 0, 127, 100),
          std::make_unique<juce::AudioParameterInt>(ParameterID {"channel", PARAMETER_V1}, "Channel", 0, 127, 0)
      })
{
    L = luaL_newstate();
    luaL_openlibs(L);

    // Store processor instance in Lua registry
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");

    lua_pushcfunction(L, luaGetParam);
    lua_setglobal(L, "getParam");
    lua_pushcfunction(L, luaSetParam);
    lua_setglobal(L, "setParam");

    luaL_dostring(L, R"(
        function paramChanged(id, value)
            print("Parameter changed: " .. id .. " = " .. value)
        end
        
        function processBlockEnter(numSamples)
            local vol = getParam("volume") / 127
            print("Processing block with volume: " .. vol)
        end
        
        function processBlockExit(numSamples)
            -- Cleanup if needed
        end
    )");
}

LuaPluginProcessor::~LuaPluginProcessor()
{
    lua_close(L);
}

void LuaPluginProcessor::prepareToPlay(double, int) {}
void LuaPluginProcessor::releaseResources() {}

void LuaPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    lua_getglobal(L, "processBlockEnter");
    lua_pushinteger(L, buffer.getNumSamples());
    lua_call(L, 1, 0);

    float vol = apvts.getRawParameterValue("volume")->load() / 127.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain(ch, 0, buffer.getNumSamples(), vol);

    lua_getglobal(L, "processBlockExit");
    lua_pushinteger(L, buffer.getNumSamples());
    lua_call(L, 1, 0);
}

int LuaPluginProcessor::luaGetParam(lua_State* L)
{
    // Retrieve processor from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");
    auto* processor = static_cast<LuaPluginProcessor*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    const char* paramId = luaL_checkstring(L, 1);
    float value = processor->apvts.getRawParameterValue(paramId)->load();
    lua_pushnumber(L, value);
    return 1;
}

int LuaPluginProcessor::luaSetParam(lua_State* L)
{
    // Retrieve processor from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");
    auto* processor = static_cast<LuaPluginProcessor*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    const char* paramId = luaL_checkstring(L, 1);
    float value = luaL_checknumber(L, 2);
    *processor->apvts.getRawParameterValue(paramId) = value;
    return 0;
}

void LuaPluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LuaPluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* LuaPluginProcessor::createEditor()
{
    return new LuaPluginEditor(*this, apvts);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LuaPluginProcessor();
}
