#include "PluginEditor.h"

LuaPluginEditor::LuaPluginEditor(LuaPluginProcessor& p, juce::AudioProcessorValueTreeState& vts)
        : AudioProcessorEditor(&p), luaProcessor(p), apvts(vts)
{
    volumeLabel.setText("Volume", juce::dontSendNotification);
    addAndMakeVisible(volumeLabel);
    volumeSlider.setRange(0, 127, 1);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(volumeSlider);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "volume", volumeSlider);

    channelLabel.setText("Channel", juce::dontSendNotification);
    addAndMakeVisible(channelLabel);
    channelSlider.setRange(0, 127, 1);
    channelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    channelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(channelSlider);
    channelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "channel", channelSlider);

    apvts.addParameterListener("volume", this);
    apvts.addParameterListener("channel", this);

    setSize(400, 200);
}

LuaPluginEditor::~LuaPluginEditor()
{
    apvts.removeParameterListener("volume", this);
    apvts.removeParameterListener("channel", this);
}

void LuaPluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void LuaPluginEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    volumeLabel.setBounds(area.removeFromTop(20));
    volumeSlider.setBounds(area.removeFromTop(50));
    area.removeFromTop(20);
    channelLabel.setBounds(area.removeFromTop(20));
    channelSlider.setBounds(area.removeFromTop(50));
}

void LuaPluginEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::Logger::writeToLog("Editor: parameterChanged called with ID: " + parameterID + ", value: " + String(newValue));
}