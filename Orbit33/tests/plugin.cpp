#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <iostream>
#include <stdexcept>
#include <set>
#include <cstdint>
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
 juce::ScopedJuceInitialiser_GUI init;
 Orbit33Processor p;p.prepareToPlay(48000,512);
 check(p.getNumPrograms()==14000,"program count");p.setCurrentProgram(11709);
 auto tone=p.params.getRawParameterValue("Tone")->load();
 juce::MemoryBlock state;p.getStateInformation(state);p.setCurrentProgram(13999);p.setStateInformation(state.getData(),int(state.getSize()));
 check(p.getCurrentProgram()==11709,"program recall");check(std::abs(p.params.getRawParameterValue("Tone")->load()-tone)<.1f,"parameter recall");
 juce::AudioBuffer<float> buffer(2,512);juce::MidiBuffer midi;midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),256);p.processBlock(buffer,midi);
 check(buffer.getMagnitude(0,256)==0,"sample accurate onset");check(buffer.getMagnitude(256,256)>0,"note produces audio");
 midi.addEvent(juce::MidiMessage::noteOff(1,60),0);p.processBlock(buffer,midi);
 float magnitude=0;
 for(int n=0;n<1200;++n){p.processBlock(buffer,midi);magnitude=buffer.getMagnitude(0,512);check(std::isfinite(magnitude)&&magnitude<=1,"bounded audio");}
 check(magnitude<.001f,"note release and effects decay");
 p.setCurrentProgram(0);
 auto set=[&](const char* id,float v){auto* a=p.params.getParameter(id);a->setValueNotifyingHost(a->convertTo0to1(v));};
 auto render=[&](float blend){p.prepareToPlay(48000,512);set("Blend",blend);juce::MidiBuffer notes;notes.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);p.processBlock(buffer,notes);return juce::AudioBuffer<float>(buffer);};
 set("Reverb",0);set("Echo",0);auto a=render(0);auto b=render(1);float difference=0;
 for(int i=0;i<512;++i)difference+=std::abs(a.getSample(0,i)-b.getSample(0,i));check(difference>.001f,"independent layer sound");
 for(int family=0;family<20;++family){p.setCurrentProgram(family*700);p.prepareToPlay(48000,512);juce::MidiBuffer notes;notes.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);p.processBlock(buffer,notes);check(buffer.getMagnitude(0,512)>0,"family produces sound");}
 // Compare actual stereo audio from every factory patch under identical MIDI input.
 // Quantization to 16 bit avoids calling insignificant float differences distinct audio.
 std::set<std::uint64_t> fingerprints;
 for(int program=0;program<14000;++program){
  p.setCurrentProgram(program);p.prepareToPlay(22050,512);std::uint64_t hash=14695981039346656037ULL;
  juce::MidiBuffer notes;notes.addEvent(juce::MidiMessage::noteOn(1,60,.8f),0);
  double energy=0;
  for(int block=0;block<8;++block){p.processBlock(buffer,notes);
   for(int n=0;n<512;++n)for(int ch=0;ch<2;++ch){float sample=buffer.getSample(ch,n);check(std::isfinite(sample),"finite bank audio");energy+=sample*sample;
    auto quantized=std::int16_t(std::round(juce::jlimit(-1.f,1.f,sample)*32767));
    hash^=std::uint16_t(quantized);hash*=1099511628211ULL;
   }
  }
  check(energy>1.e-12,"non-silent factory patch");
  if(!fingerprints.insert(hash).second)throw std::runtime_error("Duplicate rendered factory audio: "+std::to_string(program));
 }
 std::cout<<"PASS: 14000 different 16-bit stereo audio fingerprints (C4, velocity .8, 4096 frames at 22050 Hz)\n";
 p.setCurrentProgram(0);
 std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
 dynamic_cast<Orbit33Editor*>(editor.get())->selectLayer(1);
 dynamic_cast<Orbit33Editor*>(editor.get())->selectLayer(0);
 auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
 juce::File file=juce::File::getCurrentWorkingDirectory().getChildFile("Orbit33-interface.png");file.deleteFile();
 if(auto stream=file.createOutputStream()){juce::PNGImageFormat png;check(png.writeImageToStream(snapshot,*stream),"screenshot export");}
 std::cout<<"PASS: MIDI timing, sound, state recall, release, bounded output; interface captured\n";
}
