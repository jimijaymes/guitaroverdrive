#include "PluginEditor.h"
GuitarOverdriveEditor::GuitarOverdriveEditor(GuitarOverdriveProcessor& p):AudioProcessorEditor(p),processor(p){
 const char* ids[]={"input","drive","tone","level","mix","modAmount","modTone","modBass","transformerDrive","inputHP","inputLP","outputHP","outputLP"};
 const char* labels[]={"INPUT dB","DISTORTION","TONE / FILTER","LEVEL dB","MIX %","MOD DEPTH","MOD TONE","MOD BASS","XFMR DRIVE dB","INPUT HP Hz","INPUT LP Hz","OUTPUT HP Hz","OUTPUT LP Hz"};
 for(size_t i=0;i<knobs.size();++i){auto& k=knobs[i];k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,20);addAndMakeVisible(k.slider);k.label.setText(labels[i],juce::dontSendNotification);k.label.setJustificationType(juce::Justification::centred);addAndMakeVisible(k.label);k.attach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,ids[i],k.slider);}
 const char* cids[]={"pedal","muffVersion","clipping","transformer","oversampling"};
 const char* names[]={"PEDAL MODEL","BIG MUFF VERSION","DIODE / CLIPPING","SATURATOR TRANSFORMER","OVERSAMPLING"};
 const juce::StringArray opts[]={
 {"RAT","Big Muff","Tube Screamer","Boss SD-1"},
 {"Triangle","Ram's Head"},
 {"Stock","LED","Germanium","Asymmetric","MOSFET-inspired","No diodes","Schottky (optional)"},
 {"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"},
 {"1x","2x","4x","8x"}};
 for(int i=0;i<4;++i){auto& m=mods[(size_t)i];m.setButtonText("MOD "+juce::String(i+1));addAndMakeVisible(m);modAttachments[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"mod"+juce::String(i+1),m);}
 for(size_t i=0;i<choices.size();++i){auto& c=choices[i];c.label.setText(names[i],juce::dontSendNotification);addAndMakeVisible(c.label);for(int j=0;j<opts[i].size();++j)c.box.addItem(opts[i][j],j+1);addAndMakeVisible(c.box);c.attach=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,cids[i],c.box);}
 choices[0].box.onChange=[this]{const int pedal=choices[0].box.getSelectedItemIndex();
  choices[1].box.setEnabled(pedal==1);choices[1].label.setEnabled(pedal==1);
  const char* names[4][4]={{"RUETZ","GAIN EQ / FILTER","FAT","CLEAN BLEND"},{"TONE BYPASS","MID CONTROL","CREAMY / BASS","FLAT MIDS"},{"BASS+","808 VOICING","EXTENDED BASS","BRIGHT TONE"},{"C6 FILTER","BASS MOD","EXTENDED BASS","BRIGHT TONE"}};
  for(int i=0;i<4;++i)mods[(size_t)i].setButtonText(names[juce::jlimit(0,3,pedal)][i]);};
 choices[0].box.onChange();
 setSize(1060,760);
}
void GuitarOverdriveEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff161c23));g.setColour(juce::Colour(0xfff3e4c5));g.setFont(juce::Font(juce::FontOptions(30,juce::Font::bold)));g.drawText("GUITAR OVERDRIVE",24,18,620,45,juce::Justification::centredLeft);
 g.setFont(juce::Font(juce::FontOptions(13)));g.drawText("FOUR CIRCUIT FAMILIES  /  MODS  /  TRANSFORMER COLOUR",26,65,800,24,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xfff3e4c5));g.setFont(juce::Font(juce::FontOptions(13)));g.drawText("STACKABLE MODS 1–4 (PEDAL-SPECIFIC) — CLIPPING ALWAYS INDEPENDENT",26,695,960,20,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xffcf9e5c));for(int y:{105,292,482})g.drawLine(24.f,(float)y,1036.f,(float)y,1.5f);
}
void GuitarOverdriveEditor::resized(){
 for(int i=0;i<5;++i){auto& c=choices[(size_t)i];const int x=25+i*207;c.label.setBounds(x,115,195,24);c.box.setBounds(x,143,195,32);}
 for(int i=0;i<4;++i)mods[(size_t)i].setBounds(25+i*250,660,235,28);
 for(int i=0;i<5;++i){auto& k=knobs[(size_t)i];int x=25+i*207;k.slider.setBounds(x,190,175,80);k.label.setBounds(x,269,175,19);}
 for(int i=5;i<9;++i){auto& k=knobs[(size_t)i];int x=25+(i-5)*207;k.slider.setBounds(x,310,175,125);k.label.setBounds(x,438,175,22);}
 for(int i=9;i<13;++i){auto& k=knobs[(size_t)i];int x=25+(i-9)*207;k.slider.setBounds(x,500,175,125);k.label.setBounds(x,628,175,22);}
}
