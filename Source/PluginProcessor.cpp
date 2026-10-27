#include "PluginProcessor.h"
#include "PluginEditor.h"
namespace {
using AP=juce::AudioParameterFloat;
juce::AudioProcessorValueTreeState::ParameterLayout layout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 auto add=[&](const char* id,float lo,float hi,float def,float skew=1.f){
  l.add(std::make_unique<AP>(juce::ParameterID(id,1),id,juce::NormalisableRange<float>(lo,hi,0.f,skew),def));
 };
 add("Volume",0,1,.65f); add("Sound Shapes",0,1,.25f); add("Phaser",0,1,0);
 add("Reverb",0,1,.15f); add("Attack",.001f,4,.003f,.35f);
 add("Decay",.02f,4,.8f,.4f); add("Sustain",0,1,.15f);
 add("Echo",0,.8f,.04f); add("Delay",.05f,.9f,.3f);
 add("Release",.02f,5,.4f,.4f); add("Tone",120,16000,6000,.3f);
 add("Detune",0,30,2); add("Engine",0,7,1);
 return l;
}
struct Sound : juce::SynthesiserSound {
 bool appliesToNote(int) override{return true;}
 bool appliesToChannel(int) override{return true;}
};
struct Voice : juce::SynthesiserVoice {
 juce::AudioProcessorValueTreeState& params;
 juce::ADSR envelope;
 juce::ADSR::Parameters applied {-1,-1,-1,-1};
 double phase=0, phaseB=0, frequency=440;
 float velocity=0, lowpass=0;
 explicit Voice(juce::AudioProcessorValueTreeState& p):params(p){}
 float value(const char* id) const{return params.getRawParameterValue(id)->load();}
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<Sound*>(s)!=nullptr;}
 void startNote(int note,float vel,juce::SynthesiserSound*,int pitch) override {
  frequency=juce::MidiMessage::getMidiNoteInHertz(note); velocity=vel;phase=phaseB=0;lowpass=0;
  envelope.setSampleRate(getSampleRate());envelope.reset();envelope.noteOn();pitchWheelMoved(pitch);
 }
 float bend=1;
 void pitchWheelMoved(int wheel) override {bend=std::pow(2.f,((wheel-8192)/8192.f)*2.f/12.f);}
 void controllerMoved(int,int) override {}
 void stopNote(float,bool tail) override {if(tail)envelope.noteOff();else{envelope.reset();clearCurrentNote();}}
 void renderNextBlock(juce::AudioBuffer<float>& b,int start,int count) override {
  juce::ADSR::Parameters requested {value("Attack"),value("Decay"),value("Sustain"),value("Release")};
  if(requested.attack!=applied.attack || requested.decay!=applied.decay || requested.sustain!=applied.sustain || requested.release!=applied.release){applied=requested;envelope.setParameters(applied);}
  const auto sr=getSampleRate(); const auto hz=frequency*bend;
  auto engine=juce::jlimit(0,7,juce::roundToInt(value("Engine"))); auto shape=value("Sound Shapes");
  const auto drift=std::pow(2.,value("Detune")/1200.);
  const auto coefficient=1.f-float(std::exp(-juce::MathConstants<double>::twoPi*std::min(double(value("Tone")),sr*.4)/sr));
  for(int i=0;i<count;++i){
   float s=(melodex::oscillator(engine,phase,shape,hz,sr)+melodex::oscillator(engine,phaseB,shape,hz*drift,sr))*.5f;
   phase+=hz/sr;phase-=std::floor(phase);phaseB+=hz*drift/sr;phaseB-=std::floor(phaseB);
   lowpass+=coefficient*(s-lowpass); s=lowpass*envelope.getNextSample()*velocity*.16f;
   for(int ch=0;ch<b.getNumChannels();++ch)b.addSample(ch,start+i,s);
   if(!envelope.isActive()){clearCurrentNote();break;}
  }
 }
};
}
MelodexProcessor::MelodexProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMS",layout()) {
 for(int i=0;i<16;++i)synth.addVoice(new Voice(params));synth.addSound(new Sound());setCurrentProgram(0);
}
void MelodexProcessor::setCurrentProgram(int index){
 index=juce::jlimit(0,1999,index);auto p=melodex::patch(index);
 auto set=[this](const char* id,float v){auto* a=params.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));};
 set("Engine",float(p.engine));set("Sound Shapes",p.shape);set("Attack",p.attack);set("Decay",p.decay);
 set("Sustain",p.sustain);set("Release",p.release);set("Tone",p.tone);set("Detune",p.detune);
 set("Reverb",p.reverb);set("Echo",p.echo);set("Delay",p.delay);set("Phaser",p.motion);
 preset.store(index);
}
void MelodexProcessor::prepareToPlay(double sr,int){
 rate=sr;synth.setCurrentPlaybackSampleRate(sr);synth.allNotesOff(0,false);keyboard.reset();
 delayBuffer.setSize(2,int(sr)+1);delayBuffer.clear();delayPosition=0;motionPhase=0;
 reverb.setSampleRate(sr);reverb.reset();volume.reset(sr,.03);volume.setCurrentAndTargetValue(params.getRawParameterValue("Volume")->load());
}
void MelodexProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals guard;b.clear();keyboard.processNextMidiBuffer(midi,0,b.getNumSamples(),true);
 synth.renderNextBlock(b,midi,0,b.getNumSamples());midi.clear();
 auto val=[this](const char* id){return params.getRawParameterValue(id)->load();};
 float echo=val("Echo"),motion=val("Phaser");int n=delayBuffer.getNumSamples();
 int tap=juce::jlimit(1,n-1,int(val("Delay")*rate));
 for(int i=0;i<b.getNumSamples();++i){
  int read=(delayPosition+n-tap)%n;
  float dl=delayBuffer.getSample(0,read),dr=delayBuffer.getSample(1,read);
  float left=b.getSample(0,i),right=b.getSample(1,i);
  delayBuffer.setSample(0,delayPosition,left+dr*echo*.7f);delayBuffer.setSample(1,delayPosition,right+dl*echo*.7f);
  b.setSample(0,i,(left+dl*echo)*(1-motion*.25f*(1+float(std::sin(motionPhase)))));
  b.setSample(1,i,(right+dr*echo)*(1-motion*.25f*(1+float(std::cos(motionPhase)))));
  motionPhase+=juce::MathConstants<double>::twoPi*.65/rate;if(motionPhase>juce::MathConstants<double>::twoPi)motionPhase-=juce::MathConstants<double>::twoPi;
  delayPosition=(delayPosition+1)%n;
 }
 juce::Reverb::Parameters r;r.roomSize=.65f;r.damping=.45f;r.wetLevel=val("Reverb")*.4f;r.dryLevel=1;r.width=1;
 reverb.setParameters(r);reverb.processStereo(b.getWritePointer(0),b.getWritePointer(1),b.getNumSamples());
 volume.setTargetValue(val("Volume"));
 for(int i=0;i<b.getNumSamples();++i){float gain=volume.getNextValue();for(int ch=0;ch<2;++ch)b.setSample(ch,i,std::tanh(b.getSample(ch,i)*gain));}
}
void MelodexProcessor::getStateInformation(juce::MemoryBlock& dest){auto s=params.copyState();s.setProperty("preset",preset.load(),nullptr);s.setProperty("schema",2,nullptr);if(auto xml=s.createXml())copyXmlToBinary(*xml,dest);}
void MelodexProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size)){auto s=juce::ValueTree::fromXml(*xml);
 if(s.hasType(params.state.getType())){preset.store(juce::jlimit(0,1999,int(s.getProperty("preset",0))));params.replaceState(s);}}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new MelodexProcessor();}
