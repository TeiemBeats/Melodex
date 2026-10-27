#pragma once
#include "PluginProcessor.h"
class MetalLook : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class MasteringEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit MasteringEditor(MasteringProcessor&);
    ~MasteringEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    MasteringProcessor& processor;
    MetalLook look;
    std::array<juce::Slider,am13::count> knobs;
    std::array<juce::Label,am13::count> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,am13::count> links;
    juce::Slider input,ceiling;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputLink,ceilingLink;
    juce::TextButton bypass{"BYPASS"}, analyze{"ANALYZE & SUGGEST"}, reset{"RESET"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassLink;
    juce::ComboBox presets;
    juce::Label status;
    std::array<float,120> history{};
    float inDb=-100,outDb=-100;
};
