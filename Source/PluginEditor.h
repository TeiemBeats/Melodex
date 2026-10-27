#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class RoyalLook : public juce::LookAndFeel_V4 {
public:
 RoyalLook();
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class MelodexEditor : public juce::AudioProcessorEditor, private juce::ListBoxModel, private juce::Timer {
public:
 explicit MelodexEditor(MelodexProcessor&);
 ~MelodexEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 MelodexProcessor& processor;
 RoyalLook look;
 juce::ComboBox category;
 juce::TextEditor search;
 juce::ListBox browser;
 juce::TextButton previous {"<"},next {">"};
 juce::Label current;
 juce::MidiKeyboardComponent keyboard;
 std::vector<int> visible;
 std::array<juce::Slider,12> knobs;
 std::array<juce::Label,12> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 int shown=-1;
 void refresh();
 void timerCallback() override;
 int getNumRows() override {return int(visible.size());}
 void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
 void selectedRowsChanged(int) override;
};
