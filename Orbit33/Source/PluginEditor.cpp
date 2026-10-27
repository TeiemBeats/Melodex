#include "PluginEditor.h"
namespace {const juce::Colour blue(0xff4169e1),cyan(0xff78d9fa),ink(0xffe4e9f5),muted(0xff9caac2),black(0xff080c15),panel(0xff171f30);}
OrbitLook::OrbitLook(){
 setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
 setColour(juce::Slider::trackColourId,cyan);setColour(juce::Slider::thumbColourId,cyan);
 setColour(juce::ComboBox::backgroundColourId,black);setColour(juce::ComboBox::textColourId,cyan);setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff56637d));
 setColour(juce::PopupMenu::backgroundColourId,panel);setColour(juce::PopupMenu::highlightedBackgroundColourId,blue);
 setColour(juce::TextButton::buttonColourId,juce::Colour(0xff303b53));setColour(juce::TextButton::buttonOnColourId,blue);setColour(juce::TextButton::textColourOffId,ink);
 setColour(juce::TextEditor::backgroundColourId,black);setColour(juce::TextEditor::textColourId,ink);setColour(juce::TextEditor::outlineColourId,blue);
}
void OrbitLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float v,float start,float end,juce::Slider&){
 auto size=float(juce::jmin(w,h))-16;auto cx=x+w*.5f,cy=y+h*.5f,rad=size*.5f;
 juce::Path rail,arc;rail.addCentredArc(cx,cy,rad,rad,0,start,end,true);arc.addCentredArc(cx,cy,rad,rad,0,start,start+v*(end-start),true);
 g.setColour(black);g.strokePath(rail,juce::PathStrokeType(3));g.setColour(cyan);g.strokePath(arc,juce::PathStrokeType(2));
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff818b9f),cx,cy-rad,juce::Colour(0xff141b2a),cx,cy+rad,false));g.fillEllipse(cx-rad+5,cy-rad+5,size-10,size-10);
 g.setColour(juce::Colour(0xff101726));g.drawEllipse(cx-rad+5,cy-rad+5,size-10,size-10,1);
 auto a=start+v*(end-start);g.setColour(ink);g.drawLine(cx+std::sin(a)*rad*.2f,cy-std::cos(a)*rad*.2f,cx+std::sin(a)*rad*.62f,cy-std::cos(a)*rad*.62f,2);
}
Orbit33Editor::Orbit33Editor(Orbit33Processor& p):AudioProcessorEditor(&p),processor(p),keyboard(p.keyboard,juce::MidiKeyboardComponent::horizontalKeyboard){
 setLookAndFeel(&look);setSize(1160,860);
 category.addItem("ALL INSTRUMENTS / 14,000",1);for(int i=0;i<20;++i)category.addItem(orbit33::categories[i],i+2);category.setSelectedId(1);category.onChange=[this]{refresh();};addAndMakeVisible(category);
 search.setTextToShowWhenEmpty("Search patches...",muted);search.onTextChange=[this]{refresh();};addAndMakeVisible(search);
 browser.setModel(this);browser.setRowHeight(31);browser.setColour(juce::ListBox::backgroundColourId,black);addAndMakeVisible(browser);
 for(auto* b:{&previous,&next,&random,&layerA,&layerB})addAndMakeVisible(*b);
 previous.onClick=[this]{processor.setCurrentProgram((processor.getCurrentProgram()+13999)%14000);};next.onClick=[this]{processor.setCurrentProgram((processor.getCurrentProgram()+1)%14000);};
 random.onClick=[this]{if(!visible.empty())processor.setCurrentProgram(visible[size_t(juce::Random::getSystemRandom().nextInt(int(visible.size())))]);};
 layerA.onClick=[this]{selectLayer(0);};layerB.onClick=[this]{selectLayer(1);};
 current.setColour(juce::Label::textColourId,cyan);current.setFont(juce::Font(21.f,juce::Font::bold));current.setJustificationType(juce::Justification::centred);addAndMakeVisible(current);
 const char* engines[]={"Analog Harmonics","Electric Spectrum","Tonewheel Organ","Harmonic Bells","Plucked Harmonics","Sub / Pure","Vocal Formants","Rich Ensemble"};
 for(int i=0;i<8;++i)engine.addItem(engines[i],i+1);addAndMakeVisible(engine);
 target.addItem("LFO > Filter",1);target.addItem("LFO > Amplitude",2);target.addItem("LFO > Pan",3);addAndMakeVisible(target);
 targetAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.params,"LFO Target",target);
 const char* ids[]={"LFO Rate","LFO Depth","Transpose","Pan","Sound Shapes","Detune","Blend","Octave B","Tone","Resonance","Attack","Decay","Sustain","Release","Reverb","Echo","Delay","Volume"};
 const char* names[]={"RATE / Hz","DEPTH","TRANSPOSE","PAN","CHARACTER","DETUNE / ct","A / B BLEND","B OCTAVE","CUTOFF / Hz","RESONANCE","ATTACK","DECAY","SUSTAIN","RELEASE","REVERB","ECHO","TIME / s","MASTER"};
 for(int i=0;i<18;++i){auto& k=knobs[i];k.setSliderStyle(i>=10&&i<=13?juce::Slider::LinearVertical:juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,19);addAndMakeVisible(k);
 labels[i].setText(names[i],juce::dontSendNotification);labels[i].setColour(juce::Label::textColourId,muted);labels[i].setFont(10.f);labels[i].setJustificationType(juce::Justification::centred);addAndMakeVisible(labels[i]);
 attachments[i]=std::make_unique<Attach>(processor.params,ids[i],k);
 k.textFromValueFunction=[i](double v){return juce::String((i==2||i==7)?std::round(v):v,(i==2||i==7||i==8)?0:((i==10||i==11||i==13)?3:2));};k.updateText();}
 keyboard.setAvailableRange(36,96);keyboard.setLowestVisibleKey(48);keyboard.setKeyWidth(24);keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId,blue.withAlpha(.8f));addAndMakeVisible(keyboard);
 selectLayer(0);refresh();timerCallback();startTimerHz(12);
}
Orbit33Editor::~Orbit33Editor(){stopTimer();browser.setModel(nullptr);setLookAndFeel(nullptr);}
void Orbit33Editor::selectLayer(int value){
 layer=value;layerA.setToggleState(layer==0,juce::dontSendNotification);layerB.setToggleState(layer==1,juce::dontSendNotification);
 engineAttachment.reset();attachments[4].reset();attachments[5].reset();
 engineAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.params,layer==0?"Engine":"Engine B",engine);
 attachments[4]=std::make_unique<Attach>(processor.params,layer==0?"Sound Shapes":"Shape B",knobs[4]);attachments[5]=std::make_unique<Attach>(processor.params,layer==0?"Detune":"Detune B",knobs[5]);
 for(int i:{4,5}){knobs[i].textFromValueFunction=[](double v){return juce::String(v,2);};knobs[i].updateText();}repaint();
}
void Orbit33Editor::refresh(){visible.clear();int family=category.getSelectedId()-2;auto query=search.getText().trim();for(int i=0;i<14000;++i)if((family<0||i/700==family)&&processor.getProgramName(i).containsIgnoreCase(query))visible.push_back(i);browser.updateContent();shown=-1;timerCallback();repaint();}
void Orbit33Editor::timerCallback(){auto i=processor.getCurrentProgram();if(i!=shown){shown=i;current.setText(processor.getProgramName(i),juce::dontSendNotification);browser.deselectAllRows();for(int r=0;r<int(visible.size());++r)if(visible[r]==i){browser.selectRow(r);break;}}repaint(550,328,290,80);}
void Orbit33Editor::selectedRowsChanged(int row){if(row>=0&&row<int(visible.size())&&processor.getCurrentProgram()!=visible[row])processor.setCurrentProgram(visible[row]);}
void Orbit33Editor::paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool selected){if(row<0||row>=int(visible.size()))return;if(selected){g.setColour(blue);g.fillRect(0,0,w,h);}g.setColour(selected?ink:cyan);g.setFont(12.f);g.drawText(processor.getProgramName(visible[row]).fromFirstOccurrenceOf(" / ",false,false),9,0,w-18,h,juce::Justification::centredLeft);}
void Orbit33Editor::paint(juce::Graphics& g){
 g.fillAll(black);
 auto metal=[&](juce::Rectangle<int> r){g.setGradientFill(juce::ColourGradient(juce::Colour(0xff48516b),float(r.getX()),float(r.getY()),juce::Colour(0xff1d263c),float(r.getX()),float(r.getBottom()),false));g.fillRoundedRectangle(r.toFloat(),7);g.setColour(juce::Colour(0xff65708a));g.drawRoundedRectangle(r.toFloat().reduced(.5f),7,1);};
 metal({6,6,252,848});metal({270,6,884,64});
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff344fa0),280,78,juce::Colour(0xff131a38),280,636,false));g.fillRoundedRectangle(270,78,884,558,10);
 g.setColour(ink);g.setFont(17.f);g.drawText("PATCH BROWSER",24,20,220,24,juce::Justification::centredLeft);
 g.setColour(muted);g.setFont(11.f);g.drawText("ORIGINAL FACTORY COLLECTION",24,49,222,20,juce::Justification::centredLeft);
 g.setColour(cyan);g.setFont(12.f);g.drawText("MARK33S BEATS LABS",290,24,330,25,juce::Justification::centredLeft);
 g.setColour(muted);g.drawText("DUAL LAYER SYNTH  /  16 VOICES  /  MIDI",680,24,450,25,juce::Justification::centredRight);
 g.setColour(ink);g.setFont(46.f);g.drawText("O R B I T 3 3",340,84,750,65,juce::Justification::centred);
 g.setColour(muted);g.setFont(11.f);g.drawText("By Mark33s Beats Labs  /  Original Power Synth",350,145,720,20,juce::Justification::centred);
 g.setColour(black);g.fillRoundedRectangle(324,177,714,42,5);
 metal({292,266,842,350});
 g.setColour(juce::Colour(0xff1a2338));g.drawVerticalLine(530,266,616);g.drawVerticalLine(842,266,616);
 auto caption=[&](const char* text,int x,int y,int w){g.setColour(ink);g.setFont(11.f);g.drawText(text,x,y,w,24,juce::Justification::centred);};
 caption("MODULATION",300,270,222);caption(layer==0?"OSCILLATOR / LAYER A":"OSCILLATOR / LAYER B",540,270,292);caption("FILTER",852,270,272);caption("AMP ENVELOPE / seconds",852,417,272);
 g.setColour(black);g.fillRoundedRectangle(550,334,274,68,5);
 juce::Path wave;int eng=juce::roundToInt(processor.params.getRawParameterValue(layer==0?"Engine":"Engine B")->load());float shape=processor.params.getRawParameterValue(layer==0?"Sound Shapes":"Shape B")->load();
 for(int i=0;i<258;++i){auto s=orbit33::oscillator(eng,i/128.,shape,220,48000);float x=558.f+i,y=368.f-s*25; if(i==0)wave.startNewSubPath(x,y);else wave.lineTo(x,y);}
 g.setColour(cyan);g.strokePath(wave,juce::PathStrokeType(1.5f));
 metal({270,646,884,116});caption("SPACE & OUTPUT",278,650,155);
 g.setColour(muted);g.setFont(11.f);g.drawText(juce::String(visible.size())+" patches / 20 families",24,730,222,22,juce::Justification::centredLeft);
 g.drawText("14,000 procedural variations",24,756,222,22,juce::Justification::centredLeft);g.drawText("SYNTH ENGINE / NO SAMPLES",24,798,222,24,juce::Justification::centredLeft);
 if(visible.empty())g.drawText("No matching patches",24,230,222,30,juce::Justification::centred);
 g.setColour(cyan);g.setFont(10.f);g.drawText("ORBIT33 0.1  /  MARK33S BEATS LABS",280,831,864,22,juce::Justification::centredRight);
}
void Orbit33Editor::resized(){
 category.setBounds(20,82,224,34);search.setBounds(20,126,224,32);browser.setBounds(18,174,228,544);random.setBounds(22,685,220,28);browser.setBounds(18,174,228,500);
 current.setBounds(330,180,700,36);previous.setBounds(282,180,34,34);next.setBounds(1047,180,34,34);
 layerA.setBounds(556,230,128,27);layerB.setBounds(690,230,128,27);engine.setBounds(550,301,274,27);target.setBounds(304,301,214,27);
 auto knob=[&](int i,int x,int y,int w,int h){knobs[i].setBounds(x,y,w,h);labels[i].setBounds(x,y+h,w,20);};
 for(int i=0;i<4;++i)knob(i,304+(i%2)*108,338+(i/2)*125,102,90);
 for(int i=4;i<8;++i)knob(i,564+((i-4)%2)*134,410+((i-4)/2)*95,112,73);
 knob(8,861,302,122,94);knob(9,995,302,122,94);
 for(int i=10;i<14;++i)knob(i,855+(i-10)*67,448,64,128);
 for(int i=14;i<18;++i)knob(i,448+(i-14)*165,650,122,86);
 keyboard.setBounds(278,774,868,54);
}
juce::AudioProcessorEditor* Orbit33Processor::createEditor(){return new Orbit33Editor(*this);}
