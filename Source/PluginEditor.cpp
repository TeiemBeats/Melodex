#include "PluginEditor.h"
namespace { const juce::Colour royal(0xff4169e1), black(0xff080a10), panel(0xff101621), ink(0xffeaf0ff), muted(0xff8d9bb6); }
RoyalLook::RoyalLook(){
 setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
 setColour(juce::ComboBox::backgroundColourId,panel);setColour(juce::ComboBox::outlineColourId,royal.withAlpha(.5f));setColour(juce::ComboBox::textColourId,ink);
 setColour(juce::PopupMenu::backgroundColourId,panel);setColour(juce::PopupMenu::highlightedBackgroundColourId,royal);
 setColour(juce::TextButton::buttonColourId,panel);setColour(juce::TextButton::textColourOffId,ink);
 setColour(juce::TextEditor::backgroundColourId,black);setColour(juce::TextEditor::textColourId,ink);setColour(juce::TextEditor::outlineColourId,royal.withAlpha(.4f));
}
void RoyalLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float v,float start,float end,juce::Slider&){
 auto b=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(9);auto size=juce::jmin(b.getWidth(),b.getHeight());
 auto c=b.getCentre();float radius=size*.5f;juce::Path rail,arc;
 rail.addCentredArc(c.x,c.y,radius,radius,0,start,end,true);arc.addCentredArc(c.x,c.y,radius,radius,0,start,start+v*(end-start),true);
 g.setColour(juce::Colour(0xff25314a));g.strokePath(rail,juce::PathStrokeType(3));g.setColour(royal);g.strokePath(arc,juce::PathStrokeType(3));
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff35405a),c.x,c.y-radius,black,c.x,c.y+radius,false));g.fillEllipse(c.x-radius+7,c.y-radius+7,size-14,size-14);
 auto a=start+v*(end-start);g.setColour(ink);g.drawLine(c.x+std::sin(a)*radius*.38f,c.y-std::cos(a)*radius*.38f,c.x+std::sin(a)*radius*.65f,c.y-std::cos(a)*radius*.65f,2);
}
MelodexEditor::MelodexEditor(MelodexProcessor& p):AudioProcessorEditor(&p),processor(p),keyboard(p.keyboard,juce::MidiKeyboardComponent::horizontalKeyboard){
 setLookAndFeel(&look);setSize(1060,720);
 category.addItem("All instruments  /  2,000",1);for(int i=0;i<20;++i)category.addItem(melodex::categories[i],i+2);
 category.setSelectedId(1);category.onChange=[this]{refresh();};addAndMakeVisible(category);
 search.setTextToShowWhenEmpty("Search sounds...",muted);search.onTextChange=[this]{refresh();};addAndMakeVisible(search);
 browser.setModel(this);browser.setRowHeight(32);browser.setColour(juce::ListBox::backgroundColourId,black);addAndMakeVisible(browser);
 for(auto* b:{&previous,&next})addAndMakeVisible(*b);
 previous.onClick=[this]{processor.setCurrentProgram((processor.getCurrentProgram()+1999)%2000);};next.onClick=[this]{processor.setCurrentProgram((processor.getCurrentProgram()+1)%2000);};
 current.setColour(juce::Label::textColourId,ink);current.setFont(juce::Font(21.f,juce::Font::bold));addAndMakeVisible(current);
 const char* ids[]={"Volume","Sound Shapes","Tone","Detune","Reverb","Echo","Attack","Decay","Sustain","Release","Delay","Phaser"};
 const char* titles[]={"OUTPUT","CHARACTER","TONE / Hz","DETUNE / ct","REVERB","ECHO","ATTACK / s","DECAY / s","SUSTAIN","RELEASE / s","DELAY / s","MOTION"};
 for(int i=0;i<12;++i){auto& k=knobs[i];k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,20);k.setNumDecimalPlacesToDisplay(i==2?0:2);k.setDoubleClickReturnValue(true,processor.params.getRawParameterValue(ids[i])->load());addAndMakeVisible(k);
 labels[i].setText(titles[i],juce::dontSendNotification);labels[i].setColour(juce::Label::textColourId,muted);labels[i].setFont(12.f);labels[i].setJustificationType(juce::Justification::centred);addAndMakeVisible(labels[i]);
 attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.params,ids[i],k));}
 keyboard.setAvailableRange(36,96);keyboard.setLowestVisibleKey(48);keyboard.setKeyWidth(24);keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,royal.withAlpha(.8f));addAndMakeVisible(keyboard);
 refresh();timerCallback();startTimerHz(12);
}
MelodexEditor::~MelodexEditor(){stopTimer();browser.setModel(nullptr);setLookAndFeel(nullptr);}
void MelodexEditor::refresh(){
 visible.clear();int family=category.getSelectedId()-2;auto query=search.getText().trim();
 for(int i=0;i<2000;++i)if((family<0||i/100==family)&&processor.getProgramName(i).containsIgnoreCase(query))visible.push_back(i);
 browser.updateContent();shown=-1;timerCallback();repaint();
}
void MelodexEditor::timerCallback(){int i=processor.getCurrentProgram();if(i==shown)return;shown=i;current.setText(processor.getProgramName(i),juce::dontSendNotification);
 browser.deselectAllRows();for(int r=0;r<int(visible.size());++r)if(visible[r]==i){browser.selectRow(r);break;}repaint();}
void MelodexEditor::selectedRowsChanged(int row){if(row>=0&&row<int(visible.size())&&processor.getCurrentProgram()!=visible[row])processor.setCurrentProgram(visible[row]);}
void MelodexEditor::paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool selected){
 if(row<0||row>=int(visible.size()))return;
 if(selected){g.setColour(royal.withAlpha(.26f));g.fillRect(0,0,w,h);g.setColour(royal);g.fillRect(0,0,3,h);}
 auto full=processor.getProgramName(visible[row]);g.setColour(selected?ink:muted);g.setFont(14.f);g.drawText(full.fromFirstOccurrenceOf(" / ",false,false),14,0,w-20,h,juce::Justification::centredLeft);
}
void MelodexEditor::paint(juce::Graphics& g){
 g.fillAll(black);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff1b2c59),0.f,0.f,black,1060.f,180.f,false));g.fillRect(0,0,getWidth(),100);
 g.setColour(royal);g.fillRect(0,0,getWidth(),4);g.setFont(38.f);g.setColour(ink);g.drawText("MELODEX",26,17,340,48,juce::Justification::centredLeft);
 g.setFont(13.f);g.setColour(muted);g.drawText("By Mark33s Beats Labs",28,65,400,22,juce::Justification::centredLeft);
 g.setColour(royal);g.setFont(14.f);g.drawText("FACTORY COLLECTION   /   2,000 SOUNDS",650,30,382,24,juce::Justification::centredRight);
 g.setColour(muted);g.setFont(11.f);g.drawText("POLYPHONIC SYNTHESIS  •  MIDI IN  •  VST3",650,58,382,20,juce::Justification::centredRight);
 g.setColour(panel);g.fillRoundedRectangle(18,112,280,470,8);g.fillRoundedRectangle(312,112,730,470,8);
 g.setColour(royal.withAlpha(.45f));g.drawRoundedRectangle(312,112,730,78,8,1);
 g.setColour(muted);g.setFont(11.f);g.drawText("SOUND LIBRARY",32,122,250,24,juce::Justification::centredLeft);
 g.drawText("SELECTED INSTRUMENT",328,122,600,18,juce::Justification::centredLeft);
 g.drawText("SOUND & SPACE",330,206,680,20,juce::Justification::centredLeft);g.drawText("AMPLITUDE & MOVEMENT",330,382,680,20,juce::Justification::centredLeft);
 g.setColour(royal.withAlpha(.3f));g.drawHorizontalLine(370,330,1020);
 g.setColour(muted);g.drawText(juce::String(visible.size())+" presets  /  20 instrument families",32,550,250,20,juce::Justification::centredLeft);
 if(visible.empty())g.drawText("No matching sounds",32,240,250,26,juce::Justification::centred);
 g.setFont(11.f);g.drawText("PLAY YOUR MIDI KEYBOARD OR CLICK THE KEYS",24,594,1012,22,juce::Justification::centredLeft);
 g.setColour(royal);g.drawText("MARK33S BEATS LABS   /   MELODEX 0.2",24,691,1012,18,juce::Justification::centredRight);
}
void MelodexEditor::resized(){
 category.setBounds(32,154,252,32);search.setBounds(32,194,252,32);browser.setBounds(26,238,264,300);
 current.setBounds(326,146,615,38);previous.setBounds(952,141,34,34);next.setBounds(994,141,34,34);
 for(int i=0;i<12;++i){int x=326+(i%6)*116,y=i<6?234:410;knobs[i].setBounds(x,y,108,112);labels[i].setBounds(x,y+116,108,22);}
 keyboard.setBounds(24,622,1012,62);
}
juce::AudioProcessorEditor* MelodexProcessor::createEditor(){return new MelodexEditor(*this);}
