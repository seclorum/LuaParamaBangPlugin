/*
 * PluginEditor - implement a very simple PluginEditor which can be used by the Lua VM
 */
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
    LuaPluginProcessor& luaProcessor;
    juce::AudioProcessorValueTreeState& apvts;
    juce::Slider volumeSlider, channelSlider;
    juce::Label volumeLabel, channelLabel;

    // Added: Slider attachments for two-way binding
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> channelAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LuaPluginEditor)
};
