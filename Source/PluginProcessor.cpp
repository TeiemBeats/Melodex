#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 for (auto name : {"Volume","Sound Shapes","Phaser","Reverb","Attack","Decay","Sustain","Echo","Delay"})
  l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name,1),name,0.0f,1.0f, juce::String(name)=="Volume"?0.75f:0.3f));
 return l;
}
MelodexProcessor::MelodexProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMS",makeLayout()){}
void MelodexProcessor::prepareToPlay(double sr,int){sampleRate=sr;delayL.assign((size_t)(sr*2),0);delayR.assign((size_t)(sr*2),0);delayPos=0;for(auto&v:voices)v={};}
void MelodexProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;audio.clear();
 auto val=[this](const char* n){return params.getRawParameterValue(n)->load();};
 float volume=val("Volume"),shape=val("Sound Shapes"),attack=val("Attack"),decay=val("Decay"),sustain=val("Sustain"),echo=val("Echo"),delay=val("Delay"),reverb=val("Reverb"),phaser=val("Phaser");
 juce::MidiBuffer::Iterator it(midi);juce::MidiMessage msg;int at=0;bool has=it.getNextEvent(msg,at);
 for(int i=0;i<audio.getNumSamples();++i){
  while(has&&at<=i){if(msg.isNoteOn()){Voice* v=nullptr;for(auto&x:voices)if(x.note<0){v=&x;break;}if(!v)v=&voices[0];*v={msg.getNoteNumber(),0,msg.getFloatVelocity(),0,false};}else if(msg.isNoteOff()){for(auto&x:voices)if(x.note==msg.getNoteNumber())x.releasing=true;}else if(msg.isAllNotesOff()||msg.isAllSoundOff()){for(auto&x:voices)x.releasing=true;}has=it.getNextEvent(msg,at);}
  float sample=0;
  for(auto&v:voices){if(v.note<0)continue;double hz=juce::MidiMessage::getMidiNoteInHertz(v.note);double step=juce::MathConstants<double>::twoPi*hz/sampleRate;
   double a=0.002+attack*2.0,d=0.015+decay*2.0;float target=v.releasing?0.0f:(v.env<0.99f?1.0f:juce::jmap(sustain,0.05f,1.0f));double rate=v.releasing?0.03+decay*0.7:(v.env<target?a:d);
   v.env+=(target-v.env)*(float)(1.0/(juce::jmax(1.0,rate*sampleRate)));
   if(v.releasing&&v.env<0.0002f){v.note=-1;continue;}
   float sine=(float)std::sin(v.phase),saw=(float)(2.0*(v.phase/juce::MathConstants<double>::twoPi)-1.0),tri=(float)(2.0/juce::MathConstants<double>::pi*std::asin(sine));
   sample+=(sine*(1-shape)+saw*shape*0.5f+tri*shape*0.5f)*v.env*v.velocity*0.18f;
   v.phase+=step;if(v.phase>=juce::MathConstants<double>::twoPi)v.phase-=juce::MathConstants<double>::twoPi;
  }
  float modulation=1.0f-phaser*0.22f*(1.0f+(float)std::sin(i/sampleRate*4.0));sample*=modulation;
  size_t n=delayL.size();float left=sample,right=sample;
  if(n){size_t tap=(delayPos+n-(size_t)((0.12+delay*0.7)*sampleRate)%n)%n;float dl=delayL[tap],dr=delayR[tap];left+=dl*(echo*0.55f+reverb*0.2f);right+=dr*(echo*0.55f+reverb*0.2f);delayL[delayPos]=sample+dl*(echo*0.45f+reverb*0.25f);delayR[delayPos]=sample+dr*(echo*0.45f+reverb*0.25f);delayPos=(delayPos+1)%n;}
  audio.setSample(0,i,juce::jlimit(-1.0f,1.0f,left*volume));audio.setSample(1,i,juce::jlimit(-1.0f,1.0f,right*volume));
 }
}
void MelodexProcessor::getStateInformation(juce::MemoryBlock& dest){auto state=params.copyState();state.setProperty("preset",preset,nullptr);if(auto xml=state.createXml())copyXmlToBinary(*xml,dest);}
void MelodexProcessor::setStateInformation(const void* data,int size){if(auto xml=getXmlFromBinary(data,size)){auto state=juce::ValueTree::fromXml(*xml);if(state.isValid()){preset=(int)state.getProperty("preset",0);params.replaceState(state);}}}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new MelodexProcessor();}
