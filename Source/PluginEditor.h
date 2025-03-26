#pragma once
#include "PluginProcessor.h"

class LuaPluginEditor : public juce::AudioProcessorEditor,
                       public juce::AudioProcessorValueTreeState::Listener
{
public:
    LuaPluginEditor(LuaPluginProcessor&, juce::AudioProcessorValueTreeState&);
    ~LuaPluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    LuaPluginProcessor& luaProcessor;  // Renamed to avoid shadowing
    juce::AudioProcessorValueTreeState& apvts;
    juce::Slider volumeSlider, channelSlider;
    juce::Label volumeLabel, channelLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LuaPluginEditor)
};
