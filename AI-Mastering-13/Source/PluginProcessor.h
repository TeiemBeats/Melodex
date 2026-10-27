#pragma once
#include <JuceHeader.h>
#include "MasteringDSP.h"
class MasteringProcessor : public juce::AudioProcessor {
public:
    MasteringProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "AI Mastering 13"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    void preset(int);
    bool suggest();
    juce::AudioProcessorValueTreeState state;
    std::atomic<float> inputPeak{0}, outputPeak{0}, inputRms{0}, crest{0};
    std::atomic<int> blocks{0};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    void set(const char*,float);
    am13::Engine engine;
    std::array<std::atomic<float>*,am13::count> params{};
    std::atomic<float> *input=nullptr,*ceil=nullptr,*bypass=nullptr;
    float rmsMemory=0,peakMemory=0;
};
