#include "PluginProcessor.h"
#include "PluginEditor.h"
juce::AudioProcessorValueTreeState::ParameterLayout MasteringProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    for(int i=0;i<am13::count;++i) {
        float lo=0,hi=100;
        if(i==1||i==2||i==5) lo=-100;
        if(i==8) hi=200;
        if(i==9) hi=18;
        if(i==12) {lo=-24;hi=6;}
        l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{am13::ids[i],1},am13::names[i],juce::NormalisableRange<float>(lo,hi,0.1f),am13::defaults[i]));
    }
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"input",1},"Input dB",-24.f,12.f,0.f));
    l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"ceiling",1},"Ceiling dBFS",-12.f,-0.1f,-1.f));
    l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
    return l;
}
MasteringProcessor::MasteringProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"AM13",layout()) {
    for(int i=0;i<am13::count;++i) params[i]=state.getRawParameterValue(am13::ids[i]);
    input=state.getRawParameterValue("input"); ceil=state.getRawParameterValue("ceiling"); bypass=state.getRawParameterValue("bypass");
}
void MasteringProcessor::prepareToPlay(double sr,int) {engine.prepare(sr);rmsMemory=peakMemory=0;blocks=0;}
bool MasteringProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    return l.getMainInputChannelSet()==l.getMainOutputChannelSet() && (l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()||l.getMainOutputChannelSet()==juce::AudioChannelSet::mono());
}
void MasteringProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
    juce::ScopedNoDenormals guard;
    std::array<float,am13::count> p{}; for(int i=0;i<am13::count;++i) p[i]=params[i]->load();
    const float in=input->load(), ceiling=ceil->load(); const bool off=bypass->load()>0.5f;
    const int n=b.getNumSamples(); if(n==0) return;
    auto* l=b.getWritePointer(0); auto* r=b.getNumChannels()>1?b.getWritePointer(1):nullptr;
    float ip=0,op=0; double sum=0;
    for(int i=0;i<n;++i) {
        float a=l[i],c=r?r[i]:a;
        ip=std::max(ip,std::max(std::abs(a),std::abs(c))); sum+=(double(a)*a+double(c)*c)*0.5;
        engine.sample(a,c,p,in,ceiling,off);
        l[i]=a; if(r) r[i]=c;
        op=std::max(op,std::max(std::abs(a),std::abs(c)));
    }
    float decay=std::exp(-float(n)/float(getSampleRate()*2.0));
    rmsMemory=decay*rmsMemory+(1-decay)*float(sum/n); peakMemory=std::max(ip,peakMemory*decay);
    inputPeak.store(ip);outputPeak.store(op); inputRms.store(std::sqrt(rmsMemory));
    crest.store(20*std::log10(std::max(peakMemory,1.e-9f)/std::max(std::sqrt(rmsMemory),1.e-9f))); blocks.fetch_add(1);
}
void MasteringProcessor::getStateInformation(juce::MemoryBlock& b) {auto xml=state.copyState().createXml();copyXmlToBinary(*xml,b);}
void MasteringProcessor::setStateInformation(const void* d,int n) {auto xml=getXmlFromBinary(d,n);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorParameter* MasteringProcessor::getBypassParameter() const {return state.getParameter("bypass");}
void MasteringProcessor::set(const char* id,float v) {auto* p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}
void MasteringProcessor::preset(int index) {
    for(int i=0;i<am13::count;++i)set(am13::ids[i],am13::defaults[i]);
    set("input",0);set("ceiling",-1);
    if(index==1) {set("clean",80);set("compress",20);set("bass",10);set("loud",3);}
    if(index==2) {set("clean",90);set("compress",45);set("shape",20);set("bass",20);set("analog",25);set("loud",6);}
    if(index==3) {set("clean",65);set("tone",20);set("dynamic",30);set("width",115);set("excite",15);set("loud",3);}
}
bool MasteringProcessor::suggest() {
    float rms=inputRms.load(); if(blocks.load()<20||rms<0.0001f)return false;
    preset(1);set("compress",crest.load()>12?35.f:15.f);
    set("loud",juce::jlimit(0.f,9.f,-16.f-juce::Decibels::gainToDecibels(rms)));
    return true;
}
juce::AudioProcessorEditor* MasteringProcessor::createEditor(){return new MasteringEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new MasteringProcessor();}
