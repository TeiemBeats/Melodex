#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>
#include <vector>

class MelodexEditor : public juce::AudioProcessorEditor {
public:
 explicit MelodexEditor(MelodexProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 MelodexProcessor& processor;
 juce::ComboBox presetBox;
 std::array<std::unique_ptr<juce::Slider>,9> knobs;
 std::array<std::unique_ptr<juce::Label>,9> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
};
