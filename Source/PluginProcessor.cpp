/*
 * PluginProcessor.cpp - Implementation of LuaPluginProcessor with all abstract methods implemented
 */
#include "PluginProcessor.h"
#include "PluginEditor.h" // Assuming this exists

using namespace juce;

static const int PARAMETER_V1 = 1;

LuaPluginProcessor::LuaPluginProcessor()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                 .withOutput("Output", juce::AudioChannelSet::stereo())),
          apvts(*this, nullptr, "PARAMETERS", {
                  std::make_unique<juce::AudioParameterInt>(ParameterID {"volume", PARAMETER_V1}, "Volume", 0, 127, 100),
                  std::make_unique<juce::AudioParameterInt>(ParameterID {"channel", PARAMETER_V1}, "Channel", 0, 127, 0)
          }),
          L(nullptr) // Initialize L to nullptr
{
    L = luaL_newstate();
    if (!L)
    {
        juce::Logger::writeToLog("Fatal: Failed to create Lua state");
        return;
    }
    luaL_openlibs(L);

    {
        juce::ScopedLock lock(luaLock); // Protect Lua initialization
        lua_pushlightuserdata(L, this);
        lua_setfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");

        lua_pushcfunction(L, luaGetParam);
        lua_setglobal(L, "getParam");
        lua_pushcfunction(L, luaSetParam);
        lua_setglobal(L, "setParam");

        if (luaL_dostring(L, R"(
			lastVol = 0
			lutTrans = {}
			for i = 0, 127 do
				lutTrans[i] = i / 127
			end

            function paramChanged(id, value)
                print("lua:Parameter changed: " .. id .. " = " .. value)
				if (id == "channel") then
					print("lua:would set volume to", lutTrans[value])
					setParam("volume", lutTrans[value])
				end
            end

            function processBlockEnter(numSamples)
                local vol = getParam("volume")
				if (vol ~= lastVol) then
                	print("lua:Processing block with volume: " .. vol)
					lastVol = vol
				end
            end

            function processBlockExit(numSamples)
                -- Cleanup if needed
            end
        )") != LUA_OK)
        {
            juce::Logger::writeToLog("Lua init error: " + String(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
        else
        {
            juce::Logger::writeToLog("Lua script loaded successfully");
        }
    }

    apvts.addParameterListener("volume", this);
    apvts.addParameterListener("channel", this);
    juce::Logger::writeToLog("Parameter listeners added for volume and channel");
}

LuaPluginProcessor::~LuaPluginProcessor() {
    apvts.removeParameterListener("volume", this);
    apvts.removeParameterListener("channel", this);
}

void LuaPluginProcessor::parameterChanged(const String& parameterID, float newValue) {
    juce::Logger::writeToLog("parameterChanged called with ID: " + parameterID + ", value: " + String(newValue));
    callLuaFunction("paramChanged", 2, parameterID.toRawUTF8(), newValue);
}

void LuaPluginProcessor::parameterValueChanged(int parameterIndex, float newValue) {
    juce::Logger::writeToLog("parameterValueChanged called with index: " + String(parameterIndex) + ", value: " + String(newValue));
    String paramID;
    switch (parameterIndex) {
        case 0: paramID = "volume"; break;
        case 1: paramID = "channel"; break;
        default: return;
    }
    parameterChanged(paramID, newValue);
}

void LuaPluginProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting) {
    juce::Logger::writeToLog("parameterGestureChanged called: " + String(parameterIndex) + ", starting: " + String(gestureIsStarting ? "TRUE" : "FALSE"));
}

void LuaPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::Logger::writeToLog("prepareToPlay called with sampleRate: " + String(sampleRate) + ", blockSize: " + String(samplesPerBlock));
}

void LuaPluginProcessor::releaseResources() {
    juce::Logger::writeToLog("releaseResources called");
}

void LuaPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    // !J! For testing only:
#if 0
    static bool firstBlock = true;
    if (firstBlock)
    {
        juce::ScopedLock lock(luaLock); // Protect test Lua call
        lua_getglobal(L, "setParam");
        lua_pushstring(L, "volume");
        lua_pushnumber(L, 60); // Lua sets volume to 60
        if (lua_pcall(L, 2, 0, 0) != LUA_OK)
        {
            juce::Logger::writeToLog("Lua error in setParam test: " + String(lua_tostring(L, -1)));
            lua_pop(L, 1);
        }
        firstBlock = false;
    }
#endif

    callLuaFunction("processBlockEnter", 1, buffer.getNumSamples());

    float vol = apvts.getRawParameterValue("volume")->load() / 127.0f;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain(ch, 0, buffer.getNumSamples(), vol);

    callLuaFunction("processBlockExit", 1, buffer.getNumSamples());
}

int LuaPluginProcessor::luaGetParam(lua_State* L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");
    auto* processor = static_cast<LuaPluginProcessor*>(lua_touserdata(L, -1));
    if (!processor)
    {
        juce::Logger::writeToLog("Error: LuaPluginProcessor instance not found in registry.");
        return 0;
    }
    lua_pop(L, 1);

    const char* paramId = luaL_checkstring(L, 1);
    juce::ScopedLock lock(processor->luaLock); // Protect parameter access
    if (auto* param = processor->apvts.getRawParameterValue(paramId))
    {
        float value = param->load();
        lua_pushnumber(L, value);
        return 1;
    }
    juce::Logger::writeToLog("Error: Invalid parameter ID in luaGetParam: " + String(paramId));
    lua_pushnumber(L, 0.0f); // Default value
    return 1;
}

int LuaPluginProcessor::luaSetParam(lua_State* L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "LuaPluginProcessor");
    auto* processor = static_cast<LuaPluginProcessor*>(lua_touserdata(L, -1));
    if (!processor)
    {
        juce::Logger::writeToLog("Error: LuaPluginProcessor instance not found in registry.");
        return 0;
    }
    lua_pop(L, 1);

    const char* paramId = luaL_checkstring(L, 1);
    float value = luaL_checknumber(L, 2);

    {
        juce::ScopedLock lock(processor->luaLock); // Protect parameter setting
        if (auto* param = processor->apvts.getParameter(paramId))
        {
            param->setValueNotifyingHost(value);
            juce::Logger::writeToLog("luaSetParam set " + String(paramId) + " to " + String(value));
        }
    }
    return 0;
}

void LuaPluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LuaPluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* LuaPluginProcessor::createEditor() {
    return new LuaPluginEditor(*this, apvts);
}

const juce::String LuaPluginProcessor::getName() const {
    return "LuaPlugin";
}

bool LuaPluginProcessor::acceptsMidi() const {
    return true;
}

bool LuaPluginProcessor::producesMidi() const {
    return false;
}

double LuaPluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

void LuaPluginProcessor::changeProgramName(int, const juce::String&) {}

bool LuaPluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new LuaPluginProcessor(); // Line 99: Should now work
}
