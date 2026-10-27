#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class OrbitLook : public juce::LookAndFeel_V4 {
public:
 OrbitLook();
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class Orbit33Editor : public juce::AudioProcessorEditor, private juce::ListBoxModel, private juce::Timer {
public:
 explicit Orbit33Editor(Orbit33Processor&);
 ~Orbit33Editor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
 void selectLayer(int);
private:
 Orbit33Processor& processor;
 OrbitLook look;
 juce::ComboBox category,engine,target;
 juce::TextEditor search;
 juce::ListBox browser;
 juce::TextButton previous {"<"},next {">"},random {"RANDOM"},layerA {"LAYER A"},layerB {"LAYER B"};
 juce::Label current;
 juce::MidiKeyboardComponent keyboard;
 std::vector<int> visible;
 std::array<juce::Slider,18> knobs;
 std::array<juce::Label,18> labels;
 using Attach=juce::AudioProcessorValueTreeState::SliderAttachment;
 std::array<std::unique_ptr<Attach>,18> attachments;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> engineAttachment,targetAttachment;
 int shown=-1,layer=0;
 void refresh();
 void timerCallback() override;
 int getNumRows() override {return int(visible.size());}
 void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
 void selectedRowsChanged(int) override;
};
