#include "PluginEditor.h"
MelodexEditor::MelodexEditor(MelodexProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(1100,560);
 for(int i=0;i<4000;++i)presetBox.addItem(processor.getProgramName(i),i+1);
 presetBox.setSelectedId(processor.preset+1,juce::dontSendNotification);
 presetBox.onChange=[this]{processor.setCurrentProgram(presetBox.getSelectedId()-1);};addAndMakeVisible(presetBox);
 int i=0;for(auto name:{"Volume","Sound Shapes","Phaser","Reverb","Attack","Decay","Sustain","Echo","Delay"}){
  knobs[i]=std::make_unique<juce::Slider>();knobs[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knobs[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,false,75,18);addAndMakeVisible(*knobs[i]);
  labels[i]=std::make_unique<juce::Label>();labels[i]->setText(name,juce::dontSendNotification);labels[i]->setJustificationType(juce::Justification::centred);labels[i]->setColour(juce::Label::textColourId,juce::Colours::cyan);addAndMakeVisible(*labels[i]);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.params,name,*knobs[i]));++i;
 }
}
void MelodexEditor::paint(juce::Graphics&g){g.fillAll(juce::Colour::fromRGB(2,13,25));g.setColour(juce::Colours::deepskyblue);g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8),15,2);g.setFont(56);g.drawFittedText("MELODEX",20,20,getWidth()-40,85,juce::Justification::centred,1);g.setFont(18);g.drawFittedText("MARKEES BEATS LABS  |  4,000 PARAMETRIC PRESET SLOTS",20,105,getWidth()-40,30,juce::Justification::centred,1);g.setColour(juce::Colours::aqua);g.drawLine(30,230,getWidth()-30,230,2);g.setFont(15);g.drawFittedText("MIDI INSTRUMENT  |  WINDOWS VST3  |  SYNTH ENGINE",25,485,getWidth()-50,35,juce::Justification::centred,1);}
void MelodexEditor::resized(){presetBox.setBounds(350,165,400,32);int w=(getWidth()-40)/9;for(int i=0;i<9;++i){int x=20+i*w;knobs[i]->setBounds(x+4,280,w-8,135);labels[i]->setBounds(x,420,w,25);}}
juce::AudioProcessorEditor* MelodexProcessor::createEditor(){return new MelodexEditor(*this);}
