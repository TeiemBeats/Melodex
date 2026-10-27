#include "PluginEditor.h"
namespace {
juce::Colour accent(int i) {const juce::uint32 colours[]={0xff69d6ef,0xffffc06a,0xff64de95,0xffed74b0,0xffff8d6e,0xffc0a1fc};return juce::Colour(colours[i%6]);}
void text(juce::Graphics& g,const juce::String& s,juce::Rectangle<int> r,float size,juce::Colour c,juce::Justification j=juce::Justification::centredLeft) {
    g.setColour(c);g.setFont(size);g.drawText(s,r,j);
}
}
void MetalLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider& s) {
    auto r=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(8);
    float side=juce::jmin(r.getWidth(),r.getHeight());r=r.withSizeKeepingCentre(side,side);
    g.setColour(juce::Colour(0xff09151a));g.fillEllipse(r.translated(0,3));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff75868c),r.getX(),r.getY(),juce::Colour(0xff121f24),r.getRight(),r.getBottom(),false));g.fillEllipse(r);
    auto inner=r.reduced(6);g.setGradientFill(juce::ColourGradient(juce::Colour(0xffedf3f3),inner.getX(),inner.getY(),juce::Colour(0xff829398),inner.getRight(),inner.getBottom(),false));g.fillEllipse(inner);
    g.setColour(juce::Colour(0xff3e5057));g.drawEllipse(inner,1.3f);
    juce::Path arc;arc.addCentredArc(r.getCentreX(),r.getCentreY(),r.getWidth()*0.5f-2,r.getHeight()*0.5f-2,0,start,start+value*(end-start),true);
    g.setColour(s.findColour(juce::Slider::rotarySliderFillColourId));g.strokePath(arc,juce::PathStrokeType(3));
    float angle=start+value*(end-start),rad=inner.getWidth()*0.35f;
    auto c=inner.getCentre();g.setColour(juce::Colour(0xff20353d));
    g.drawLine(c.x+std::sin(angle)*rad*0.6f,c.y-std::cos(angle)*rad*0.6f,c.x+std::sin(angle)*rad,c.y-std::cos(angle)*rad,3);
}
MasteringEditor::MasteringEditor(MasteringProcessor& p):AudioProcessorEditor(p),processor(p) {
    setLookAndFeel(&look);
    look.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffc8d7db));
    look.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    look.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff293d45));
    look.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff92602d));
    for(int i=0;i<am13::count;++i) {
        auto& k=knobs[i];addAndMakeVisible(k);k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,20);k.setColour(juce::Slider::rotarySliderFillColourId,accent(i));
        k.setTooltip(juce::String(am13::names[i])+" - drag to adjust; double-click to reset");
        k.setDoubleClickReturnValue(true,am13::defaults[i]);
        if(i==9||i==12)k.setTextValueSuffix(" dB");else k.setTextValueSuffix(" %");
        links[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,am13::ids[i],k);
        addAndMakeVisible(labels[i]);labels[i].setText(juce::String(am13::names[i]).toUpperCase(),juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);labels[i].setColour(juce::Label::textColourId,accent(i));labels[i].setFont(11.f);
    }
    for(auto* s:{&input,&ceiling}) {addAndMakeVisible(*s);s->setSliderStyle(juce::Slider::LinearHorizontal);s->setTextBoxStyle(juce::Slider::TextBoxRight,false,70,22);s->setTextValueSuffix(" dB");}
    inputLink=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"input",input);
    ceilingLink=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,"ceiling",ceiling);
    addAndMakeVisible(bypass);bypass.setClickingTogglesState(true);bypassLink=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bypass);
    addAndMakeVisible(analyze);analyze.onClick=[this]{status.setText(processor.suggest()?"Suggestion applied - compare with bypass, then adjust to taste.":"Play your track for a few seconds, then analyze again.",juce::dontSendNotification);presets.setSelectedId(0,juce::dontSendNotification);};
    addAndMakeVisible(reset);reset.onClick=[this]{processor.preset(0);presets.setSelectedId(1,juce::dontSendNotification);status.setText("Controls reset.",juce::dontSendNotification);};
    addAndMakeVisible(presets);presets.addItemList({"Default / neutral","Gentle master","Beat punch","Open & bright"},1);presets.setSelectedId(1);presets.onChange=[this]{processor.preset(presets.getSelectedId()-1);status.setText("Preset applied. Settings are saved with your session.",juce::dontSendNotification);};
    addAndMakeVisible(status);status.setFont(12.f);status.setColour(juce::Label::textColourId,juce::Colour(0xffa4b7bd));status.setText("Play 5-10 seconds of a loud section, then Analyze & Suggest. Level-based analysis; no trained AI.",juce::dontSendNotification);
    setSize(1100,750);startTimerHz(30);
}
MasteringEditor::~MasteringEditor(){stopTimer();setLookAndFeel(nullptr);}
void MasteringEditor::resized() {
    presets.setBounds(570,25,230,30);bypass.setBounds(930,25,140,30);reset.setBounds(815,25,100,30);
    analyze.setBounds(835,132,230,42);input.setBounds(880,201,185,28);ceiling.setBounds(880,246,185,28);
    status.setBounds(30,703,1040,25);
    for(int i=0;i<am13::count;++i) {
        int row=i<7?0:1,col=i<7?i:i-7,num=row==0?7:6;
        int width=1040/num,x=30+col*width,y=393+row*151;
        knobs[i].setBounds(x+12,y+19,width-24,119);labels[i].setBounds(x,y,width,21);
    }
}
void MasteringEditor::timerCallback() {
    inDb=juce::Decibels::gainToDecibels(processor.inputPeak.load(),-100.f);outDb=juce::Decibels::gainToDecibels(processor.outputPeak.load(),-100.f);
    std::move(history.begin()+1,history.end(),history.begin());history.back()=juce::jlimit(0.f,1.f,(outDb+60)/60);
    repaint();
}
void MasteringEditor::paint(juce::Graphics& g) {
    const auto dark=juce::Colour(0xff102027),white=juce::Colour(0xffe2ecee);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe0e4e4),0,0,juce::Colour(0xff829298),1100,750,false));g.fillAll();
    text(g,"AI MASTERING 13",{28,16,520,30},25,dark);text(g,"BUSY WORKS BEATS  /  CUSTOM CONCEPT",{30,48,490,18},10,juce::Colour(0xff45575d));
    g.setColour(dark);g.fillRoundedRectangle(15,82,1070,277,10);g.fillRoundedRectangle(15,375,1070,316,10);g.fillRoundedRectangle(15,697,1070,36,6);
    text(g,"MASTER",{32,87,160,24},11,accent(0));text(g,"STEREO  /  64-BIT VST3 TARGET",{740,87,325,24},10,juce::Colour(0xffa2b6bd),juce::Justification::centredRight);
    auto panel=juce::Rectangle<float>(30,121,775,218);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff173c49),30,121,juce::Colour(0xff081419),805,339,false));g.fillRoundedRectangle(panel,6);
    for(int i=0;i<5;++i){g.setColour(juce::Colour(0xff29434c));g.drawHorizontalLine(154+i*36,40,795);text(g,juce::String(-i*12)+" dBFS",{43,137+i*36,80,18},9,juce::Colour(0xff7d9aa5));}
    juce::Path curve;curve.startNewSubPath(115,320-history[0]*166);
    for(int i=1;i<120;++i)curve.lineTo(115+float(i)*5.63f,320-history[i]*166);
    g.setColour(accent(0));g.strokePath(curve,juce::PathStrokeType(2));text(g,"OUTPUT PEAK HISTORY  /  ~4 SECONDS",{44,124,480,20},10,white);
    text(g,"INPUT",{835,183,100,17},10,white);text(g,"CEILING",{835,230,100,17},10,white);
    text(g,"IN  "+juce::String(inDb,1)+" dBFS",{835,289,230,20},12,accent(0));text(g,"OUT  "+juce::String(outDb,1)+" dBFS",{835,313,230,20},12,outDb>0?juce::Colours::red:accent(3));
    text(g,"SIGNAL PATH  /  FIXED ORDER  /  ALL CONTROLS AUTOMATABLE",{30,357,1040,18},9,dark);
    g.setColour(juce::Colour(0xff29424b));g.drawHorizontalLine(539,30,1070);
}
