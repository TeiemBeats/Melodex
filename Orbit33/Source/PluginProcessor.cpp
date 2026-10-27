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
 add("Detune",0,40,2); add("Engine",0,7,1);
 add("Engine B",0,7,0);add("Shape B",0,1,.45f);add("Detune B",0,40,8);
 add("Blend",0,1,.35f);add("Octave B",-2,2,0);add("Resonance",0,.9f,.15f);
 add("LFO Rate",.05f,12,.65f,.4f);add("LFO Depth",0,1,.15f);
 add("LFO Target",0,2,0);add("Pan",-1,1,0);add("Transpose",-24,24,0);
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
 float velocity=0, lowpass=0, lowpass2=0;
 double lfo=0;
 explicit Voice(juce::AudioProcessorValueTreeState& p):params(p){}
 float value(const char* id) const{return params.getRawParameterValue(id)->load();}
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<Sound*>(s)!=nullptr;}
 void startNote(int note,float vel,juce::SynthesiserSound*,int pitch) override {
  frequency=juce::MidiMessage::getMidiNoteInHertz(note); velocity=vel;phase=phaseB=0;lfo=0;lowpass=lowpass2=0;
  envelope.setSampleRate(getSampleRate());envelope.reset();envelope.noteOn();pitchWheelMoved(pitch);
 }
 float bend=1;
 void pitchWheelMoved(int wheel) override {bend=std::pow(2.f,((wheel-8192)/8192.f)*2.f/12.f);}
 void controllerMoved(int,int) override {}
 void stopNote(float,bool tail) override {if(tail)envelope.noteOff();else{envelope.reset();clearCurrentNote();}}
 void renderNextBlock(juce::AudioBuffer<float>& b,int start,int count) override {
  juce::ADSR::Parameters requested {value("Attack"),value("Decay"),value("Sustain"),value("Release")};
  if(requested.attack!=applied.attack || requested.decay!=applied.decay || requested.sustain!=applied.sustain || requested.release!=applied.release){applied=requested;envelope.setParameters(applied);}
  const auto sr=getSampleRate(); const auto hz=frequency*bend*std::pow(2.,std::round(value("Transpose"))/12.);
  auto engine=juce::jlimit(0,7,juce::roundToInt(value("Engine"))); auto shape=value("Sound Shapes");
  const auto drift=std::pow(2.,value("Detune")/1200.);
  const auto hzB=hz*std::pow(2.,std::round(value("Octave B"))+value("Detune B")/1200.);
  const int engineB=juce::jlimit(0,7,juce::roundToInt(value("Engine B")));
  const float shapeB=value("Shape B"),blend=value("Blend"),res=value("Resonance"),depth=value("LFO Depth");
  const int target=juce::roundToInt(value("LFO Target"));
  const auto lfoStep=value("LFO Rate")/sr;
  const auto gainA=std::cos(blend*juce::MathConstants<float>::halfPi),gainB=std::sin(blend*juce::MathConstants<float>::halfPi);
  const float pan=value("Pan");
  const auto coefficient=1.f-float(std::exp(-juce::MathConstants<double>::twoPi*std::min(double(value("Tone")),sr*.4)/sr));
  for(int i=0;i<count;++i){
   float modulation=float(std::sin(lfo*juce::MathConstants<double>::twoPi));lfo+=lfoStep;lfo-=std::floor(lfo);
   float s=orbit33::oscillator(engine,phase,shape,hz*drift,sr)*gainA+orbit33::oscillator(engineB,phaseB,shapeB,hzB,sr)*gainB;
   phase+=hz*drift/sr;phase-=std::floor(phase);phaseB+=hzB/sr;phaseB-=std::floor(phaseB);
   const auto c=juce::jlimit(.001f,.95f,coefficient*(target==0?1+modulation*depth*.8f:1));
   lowpass+=c*(s-res*lowpass2-lowpass);lowpass2+=c*(lowpass-lowpass2);
   s=lowpass2*envelope.getNextSample()*velocity*.14f*(target==1?1-depth*.5f*(1+modulation):1);
   const float position=juce::jlimit(-1.f,1.f,pan+(target==2?modulation*depth:0));
   b.addSample(0,start+i,s*std::sqrt((1-position)*.5f));b.addSample(1,start+i,s*std::sqrt((1+position)*.5f));
   if(!envelope.isActive()){clearCurrentNote();break;}
  }
 }
};
}
Orbit33Processor::Orbit33Processor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMS",layout()) {
 for(int i=0;i<16;++i)synth.addVoice(new Voice(params));synth.addSound(new Sound());setCurrentProgram(0);
}
void Orbit33Processor::setCurrentProgram(int index){
 index=juce::jlimit(0,13999,index);auto p=orbit33::patch(index);
 auto set=[this](const char* id,float v){auto* a=params.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));};
 set("Engine",float(p.engine));set("Sound Shapes",p.shape);set("Attack",p.attack);set("Decay",p.decay);
 set("Sustain",p.sustain);set("Release",p.release);set("Tone",p.tone);set("Detune",p.detune);
 set("Reverb",p.reverb);set("Echo",p.echo);set("Delay",p.delay);set("Phaser",p.motion);
 set("Engine B",float((p.engine+1+(index%7))%8));set("Shape B",std::min(1.f,p.shape*.7f+.2f));
 set("Detune B",std::min(40.f,p.detune+4));set("Blend",.15f+.06f*((index%700)/100));
 set("Octave B",index/700==8?-1.f:0.f);set("Resonance",.08f+.06f*((index%700)/100));
 set("LFO Rate",.2f+.12f*((index%100)/10));set("LFO Depth",p.motion+.05f);set("LFO Target",0);
 set("Transpose",0);set("Pan",0);preset.store(index);
}
void Orbit33Processor::prepareToPlay(double sr,int){
 rate=sr;synth.setCurrentPlaybackSampleRate(sr);synth.allNotesOff(0,false);keyboard.reset();
 delayBuffer.setSize(2,int(sr)+1);delayBuffer.clear();delayPosition=0;motionPhase=0;
 reverb.setSampleRate(sr);reverb.reset();volume.reset(sr,.03);volume.setCurrentAndTargetValue(params.getRawParameterValue("Volume")->load());
}
void Orbit33Processor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
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
void Orbit33Processor::getStateInformation(juce::MemoryBlock& dest){auto s=params.copyState();s.setProperty("preset",preset.load(),nullptr);s.setProperty("schema",2,nullptr);if(auto xml=s.createXml())copyXmlToBinary(*xml,dest);}
void Orbit33Processor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size)){auto s=juce::ValueTree::fromXml(*xml);
 if(s.hasType(params.state.getType())){preset.store(juce::jlimit(0,13999,int(s.getProperty("preset",0))));params.replaceState(s);}}
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Orbit33Processor();}
