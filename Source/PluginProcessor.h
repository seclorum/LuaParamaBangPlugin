/*
 * PluginProcessor.h - Header for LuaPluginProcessor
 */
#ifndef PLUGINPROCESSOR_H
#define PLUGINPROCESSOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "LuaInterface.h"

class LuaPluginProcessor : public juce::AudioProcessor,
                           public LuaInterface,
                           public juce::AudioProcessorValueTreeState::Listener,
                           public juce::AudioProcessorParameter::Listener {
public:
    LuaPluginProcessor();
    ~LuaPluginProcessor() override;

    // Listener interface methods
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    // Additional method for Lua integration
    void parameterChanged(const juce::String& parameterID, float newValue);


    // AudioProcessor required methods
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() const { return 1; }
    int getCurrentProgram() const { return 1; }
    void setCurrentProgram(int index) { }
    const juce::String getProgramName(int index) const {return ""; }

    int getNumPrograms() { return 1; }

    int getCurrentProgram() { return 1; }

    void changeProgramName(int index, const juce::String& newName) override;
    bool hasEditor() const override;

    const juce::String getProgramName (int index) { return ""; }

    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorEditor* createEditor() override;
    juce::String getLuaScript() const;

private:
    juce::AudioProcessorValueTreeState apvts;
    static const juce::String defaultLuaScript;

};

#endif // PLUGINPROCESSOR_H