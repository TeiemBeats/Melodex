#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

class MelodexProcessor : public juce::AudioProcessor {
public:
 MelodexProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout& layouts) const override { return layouts.getMainOutputChannelSet()==juce::AudioChannelSet::stereo(); }
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "MELODEX"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 2.0; }
 int getNumPrograms() override { return 4000; }
 int getCurrentProgram() override { return preset; }
 void setCurrentProgram(int i) override { preset=juce::jlimit(0,3999,i); }
 const juce::String getProgramName(int i) override { return "MELODEX Preset " + juce::String(i+1); }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState params;
 int preset=0;
private:
 struct Voice { int note=-1; double phase=0; float velocity=0,env=0; bool releasing=false; };
 std::array<Voice,16> voices{};
 double sampleRate=44100;
 std::vector<float> delayL,delayR;
 size_t delayPos=0;
};
