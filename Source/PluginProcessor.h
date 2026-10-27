#pragma once
#include <JuceHeader.h>
#include "PresetBank.h"
class MelodexProcessor : public juce::AudioProcessor {
public:
 MelodexProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout& l) const override { return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() && l.getMainInputChannelSet().isDisabled(); }
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "MELODEX"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 12.; }
 int getNumPrograms() override { return melodex::presetCount; }
 int getCurrentProgram() override { return preset.load(); }
 void setCurrentProgram(int) override;
 const juce::String getProgramName(int i) override { return melodex::name(i); }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState params;
 juce::MidiKeyboardState keyboard;
private:
 std::atomic<int> preset {0};
 juce::Synthesiser synth;
 juce::Reverb reverb;
 juce::AudioBuffer<float> delayBuffer;
 int delayPosition=0;
 double rate=44100, motionPhase=0;
 juce::SmoothedValue<float> volume;
};
