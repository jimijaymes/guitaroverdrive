#include "PluginEditor.h"

namespace {
const juce::Colour background(0xff141b22), panel(0xff202b35), accent(0xffe6b46b), text(0xfff2e8d8);
const char* modNames[11][4]={
 {"Ruetz gain-network change","Gain EQ / filter","Fat low end","Clean blend"},
 {"Tone-stack bypass","Mid response","Creamy / bass","Flat mids"},
 {"Bass+ feedback filter","808-style voicing","Extended bass","Bright tone"},
 {"C6 filter change","Bass feedback mod","Extended bass","Bright tone"},
 {"Bass coupling","Presence","Clip density","Clean blend"},
 {"Bass coupling","Presence","Clip density","Clean blend"},
 {"Phat bass","Presence","Clip density","Clean blend"},
 {"Bass coupling","Presence","Clip density","Clean blend"},
 {"Bass coupling","Presence","Clip density","Clean blend"},
 {"Bass coupling","Presence","Clip density","Clean blend"},
 {"Bass coupling","Presence","Clip density","Clean blend"}
};
}
GuitarOverdriveEditor::GuitarOverdriveEditor(GuitarOverdriveProcessor& p):AudioProcessorEditor(p),processor(p)
{
 const char* ids[]={"input","drive","tone","level","mix","modAmount","modTone","modBass","transformerDrive","inputHP","inputLP","outputHP","outputLP","tcBoostGain","boostMidFreq","boostMidGain","pedalBass","pedalTreble"};
 const char* labels[]={"INPUT GAIN (dB)","DISTORTION","TONE / FILTER","PEDAL LEVEL (dB)","DRY / WET MIX","MOD AMOUNT","MOD TONE","MOD BASS","TRANSFORMER DRIVE","INPUT HP (Hz)","INPUT LP (Hz)","OUTPUT HP (Hz)","OUTPUT LP (Hz)","BOOST GAIN (dB)","BOOST MID FREQ","BOOST MID GAIN","PEDAL BASS","PEDAL TREBLE"};
 for(size_t i=0;i<knobs.size();++i){
  auto& k=knobs[i];k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,20);
  k.slider.setColour(juce::Slider::rotarySliderFillColourId,accent);
  addAndMakeVisible(k.slider);k.label.setText(labels[i],juce::dontSendNotification);
  k.label.setJustificationType(juce::Justification::centred);k.label.setMinimumHorizontalScale(0.68f);k.label.setColour(juce::Label::textColourId,text);
  addAndMakeVisible(k.label);
  k.attach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,ids[i],k.slider);
 }
 for(int i=0;i<4;++i){auto& k=modKnobs[(size_t)i];k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,20);k.slider.setColour(juce::Slider::rotarySliderFillColourId,accent);addAndMakeVisible(k.slider);k.label.setJustificationType(juce::Justification::centred);k.label.setMinimumHorizontalScale(.68f);k.label.setColour(juce::Label::textColourId,text);addAndMakeVisible(k.label);k.attach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,"modControl"+juce::String(i+1),k.slider);}
 const char* cids[]={"pedal","muffVersion","feedbackCap","clipping","transformer","oversampling","tcBoostPosition","boostModel","dualMode","modCircuit"};
 const char* names[]={"PEDAL MODEL","BIG MUFF VERSION","FEEDBACK CAPACITOR","CLIPPING DIODES","TRANSFORMER MODEL","OVERSAMPLING","BOOST ROUTING","BOOST CIRCUIT","PEDAL MODE","CIRCUIT MOD"};
 const juce::StringArray opts[]={
 {"RAT","Big Muff","Tube Screamer","Boss SD-1","DOD Grunge FX69","DOD Death Metal FX86","Boss BD-2","Boss DS-1","Boss OD-2","Boss HM-2","Boss SD-2"},
 {"Triangle","Ram's Head"},{"Stock (1x)","Larger (1.5x)","Smaller (0.68x)"},
 {"Follow Design","LED Pair","Germanium Pair","Asymmetric Si","MOSFET-inspired","No Diodes","Schottky Pair","High-Vf Silicon / LED (Asymmetric)","Low-Vf Silicon Pair","High-Vf Silicon Pair","2x Silicon / 1x LED (Asymmetric)","Half-Wave Silicon (Positive Only)"},
 {"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"},
 {"1x","2x","4x","8x"},{"Off","Pre Pedal","Post Pedal"},{"TC Integrated Preamp","Boss FA-1"},
 {"Normal / Crunch","Turbo / Lead"},{"Stock","Mod A","Mod B","Mod C"}};
 for(size_t i=0;i<choices.size();++i){
  auto& c=choices[i];c.label.setText(names[i],juce::dontSendNotification);
  c.label.setColour(juce::Label::textColourId,text);addAndMakeVisible(c.label);
  c.box.addItemList(opts[i],1);addAndMakeVisible(c.box);
  c.attach=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,cids[i],c.box);
 }
 for(int i=0;i<4;++i){
  auto& m=mods[(size_t)i];addAndMakeVisible(m);
  m.setColour(juce::ToggleButton::textColourId,text);
  modAttachments[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"mod"+juce::String(i+1),m);
  m.onClick=[this]{updateContext();};
 }
 pedalBypass.setButtonText("BYPASS PEDAL");addAndMakeVisible(pedalBypass);
 pedalBypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"pedalBypass",pedalBypass);
 boostMidOn.setButtonText("MID EQ ON");addAndMakeVisible(boostMidOn);
 boostMidAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"boostMidOn",boostMidOn);
 choices[0].box.onChange=[this]{updateContext();};
 choices[6].box.onChange=[this]{updateContext();};
 choices[9].box.onChange=[this]{updateContext();};
 boostMidOn.onClick=[this]{updateContext();};
 setSize(1160,850);
 updateContext();
}
void GuitarOverdriveEditor::placeKnob(int index,juce::Rectangle<int> b)
{
 auto& k=knobs[(size_t)index];k.label.setBounds(b.removeFromTop(23));
 k.slider.setBounds(b);
}
void GuitarOverdriveEditor::placeChoice(int index,juce::Rectangle<int> b)
{
 auto& c=choices[(size_t)index];c.label.setBounds(b.removeFromTop(22));b.removeFromTop(3);c.box.setBounds(b.removeFromTop(30));
}
void GuitarOverdriveEditor::updateContext()
{
 const int pedal=juce::jlimit(0,10,choices[0].box.getSelectedItemIndex());
 const bool muff=pedal==1, ts=pedal==2,sd=pedal==3, newer=pedal>=4;
 const bool dual=pedal==8||pedal==10;
 const bool circuit=pedal==6||pedal==7||pedal==9;
 auto showChoice=[this](int i,bool show){choices[(size_t)i].box.setVisible(show);choices[(size_t)i].label.setVisible(show);};
 auto showKnob=[this](int i,bool show){knobs[(size_t)i].slider.setVisible(show);knobs[(size_t)i].label.setVisible(show);};
 showChoice(1,muff);showChoice(2,ts||sd);showChoice(8,dual);showChoice(9,circuit);
 // The circuit selector is an existing four-way parameter. Its visible names follow the selected pedal.
 if(circuit){
  const juce::StringArray names=pedal==7?juce::StringArray{"Stock","Keeley Seeing Eye","Keeley Ultra","Ultra + bass"}:
   pedal==6?juce::StringArray{"Stock","Phat 68nF","Phat 82nF","Phat + asymmetric"}:
   juce::StringArray{"Stock","Bass response","Presence response","Low-tuned EQ"};
  auto& c=choices[9].box;
  if(c.getNumItems()!=names.size() || c.getItemText(1)!=names[1]){
   const int selected=c.getSelectedId();c.clear(juce::dontSendNotification);
   c.addItemList(names,1);c.setSelectedId(juce::jlimit(1,4,selected),juce::dontSendNotification);
  }
 }
 for(int i=0;i<4;++i)mods[(size_t)i].setButtonText(modNames[pedal][i]);
 // Every enabled modification owns its own parameter and dedicated control.
 const bool usesControl[11][4]={
  {true,false,true,true},{false,true,true,false},{true,false,true,true},
  {true,true,true,true},{true,true,true,false},{true,true,true,false},
  {true,true,true,false},{true,true,true,false},{true,true,true,false},
  {true,true,true,false},{true,true,true,false}
 };
 const char* controlLabels[11][4]={
  {"RUETZ AMOUNT","","FAT AMOUNT","CLEAN BLEND"},
  {"","MID RESPONSE","BASS RESPONSE",""},
  {"BASS+ AMOUNT","","EXTENDED BASS","BRIGHTNESS"},
  {"C6 FILTER","BASS MOD","EXTENDED BASS","BRIGHTNESS"},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY",""},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY",""},
  {"PHAT AMOUNT","PRESENCE","CLIP DENSITY",""},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY",""},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY",""},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY",""},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY",""}
 };
 for(int i=0;i<4;++i){
  const bool show=mods[(size_t)i].getToggleState()&&usesControl[pedal][i];
  modKnobs[(size_t)i].slider.setVisible(show);
  modKnobs[(size_t)i].label.setVisible(show);
  modKnobs[(size_t)i].label.setText(controlLabels[pedal][i],juce::dontSendNotification);
 }
 for(int i=5;i<=7;++i){knobs[(size_t)i].slider.setVisible(false);knobs[(size_t)i].label.setVisible(false);}
 const bool graphicEq=pedal==4||pedal==5||pedal==9;
 showKnob(2,!graphicEq);
 showKnob(16,graphicEq);showKnob(17,graphicEq);
 knobs[16].label.setText("LOW EQ",juce::dontSendNotification);
 knobs[17].label.setText("HIGH EQ",juce::dontSendNotification);
 const bool boost=choices[6].box.getSelectedItemIndex()>0;
 showChoice(7,boost);showKnob(13,boost);boostMidOn.setVisible(boost);
 const bool mid=boost&&boostMidOn.getToggleState();
 showKnob(14,mid);showKnob(15,mid);
 resized();repaint();
}
void GuitarOverdriveEditor::paint(juce::Graphics& g)
{
 g.fillAll(background);g.setColour(text);g.setFont(juce::Font(juce::FontOptions(28,juce::Font::bold)));
 g.drawText("GUITAR OVERDRIVE",24,12,700,42,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xffaebac3));g.setFont(13.f);
 g.drawText("11 PEDALS  /  CIRCUIT MODS  /  TRANSFORMER",26,54,850,20,juce::Justification::centredLeft);
 auto panelDraw=[&](juce::Rectangle<int> r,const juce::String& title){
  g.setColour(panel);g.fillRoundedRectangle(r.toFloat(),12.f);
  g.setColour(accent);g.setFont(juce::Font(juce::FontOptions(16,juce::Font::bold)));
  g.drawText(title,r.getX()+18,r.getY()+12,r.getWidth()-36,24,juce::Justification::centredLeft);
 };
 panelDraw({20,88,550,340},"PEDAL  /  STOCK CONTROLS");
 panelDraw({585,88,555,340},"PEDAL MODIFICATIONS");
 panelDraw({20,443,550,385},"MASTER  /  FILTERS  /  BOOST");
 panelDraw({585,443,555,385},"TRANSFORMER  /  QUALITY");
}
void GuitarOverdriveEditor::resized()
{
 placeChoice(0,{38,137,500,58});
 placeChoice(1,{38,205,230,58});
 placeChoice(2,{286,205,250,58});
 placeChoice(8,{38,205,230,58});
 placeKnob(1,{38,278,122,130});
 placeKnob(2,{166,278,122,130});
 placeKnob(3,{422,278,122,130});
 placeKnob(16,{166,278,122,130});
 placeKnob(17,{294,278,122,130});
 pedalBypass.setBounds(410,385,145,28);
 for(int i=0;i<4;++i)mods[(size_t)i].setBounds(604+(i%2)*265,142+(i/2)*42,255,30);
 placeChoice(9,{605,242,246,58});
 placeChoice(3,{869,242,248,58});
 for(int i=0;i<4;++i){auto& k=modKnobs[(size_t)i];const int x=603+i*132;k.label.setBounds(x,309,124,23);k.slider.setBounds(x,332,124,88);}
 for(int i=5;i<=7;++i)placeKnob(i,{603+(i-5)*175,305,166,110});
 placeKnob(0,{38,498,120,125});
 placeKnob(4,{167,498,120,125});
 placeChoice(6,{310,496,240,58});
 placeChoice(7,{310,559,240,58});
 placeKnob(13,{38,636,125,125});
 boostMidOn.setBounds(310,633,210,28);
 placeKnob(14,{171,677,120,130});
 placeKnob(15,{310,677,120,130});
 placeChoice(4,{604,499,515,58});
 placeKnob(8,{607,579,160,145});
 placeChoice(5,{789,579,320,58});
 placeKnob(9,{604,705,125,110});
 placeKnob(10,{735,705,125,110});
 placeKnob(11,{866,705,125,110});
 placeKnob(12,{997,705,125,110});
}
