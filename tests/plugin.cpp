#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
 juce::ScopedJuceInitialiser_GUI init;
 MelodexProcessor p;p.prepareToPlay(48000,512);
 check(p.getNumPrograms()==2000,"program count");p.setCurrentProgram(1709);
 auto tone=p.params.getRawParameterValue("Tone")->load();
 juce::MemoryBlock state;p.getStateInformation(state);p.setCurrentProgram(4);p.setStateInformation(state.getData(),int(state.getSize()));
 check(p.getCurrentProgram()==1709,"program recall");check(std::abs(p.params.getRawParameterValue("Tone")->load()-tone)<.1f,"parameter recall");
 juce::AudioBuffer<float> buffer(2,512);juce::MidiBuffer midi;midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),256);p.processBlock(buffer,midi);
 check(buffer.getMagnitude(0,256)==0,"sample accurate onset");check(buffer.getMagnitude(256,256)>0,"note produces audio");
 midi.addEvent(juce::MidiMessage::noteOff(1,60),0);p.processBlock(buffer,midi);
 float magnitude=0;
 for(int n=0;n<1200;++n){p.processBlock(buffer,midi);magnitude=buffer.getMagnitude(0,512);check(std::isfinite(magnitude)&&magnitude<=1,"bounded audio");}
 check(magnitude<.001f,"note release and effects decay");
 p.setCurrentProgram(0);
 std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
 auto snapshot=editor->createComponentSnapshot(editor->getLocalBounds());
 juce::File file=juce::File::getCurrentWorkingDirectory().getChildFile("Melodex-interface.png");file.deleteFile();
 if(auto stream=file.createOutputStream()){juce::PNGImageFormat png;check(png.writeImageToStream(snapshot,*stream),"screenshot export");}
 std::cout<<"PASS: MIDI timing, sound, state recall, release, bounded output; interface captured\n";
}
