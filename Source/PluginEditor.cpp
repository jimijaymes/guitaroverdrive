#include "PluginEditor.h"
GuitarOverdriveEditor::GuitarOverdriveEditor(GuitarOverdriveProcessor& p):AudioProcessorEditor(p),processor(p){
 const char* ids[]={"input","drive","tone","level","mix","modAmount","modTone","modBass","transformerDrive","inputHP","inputLP","outputHP","outputLP","tcBoostGain","boostMidFreq","boostMidGain","pedalBass","pedalTreble"};
 const char* labels[]={"INPUT dB","DISTORTION","TONE / FILTER","LEVEL dB","MIX %","MOD DEPTH","MOD TONE","MOD BASS","XFMR DRIVE dB","INPUT HP Hz","INPUT LP Hz","OUTPUT HP Hz","OUTPUT LP Hz","BOOST dB","MID FREQ Hz","MID GAIN dB","PEDAL BASS %","PEDAL TREBLE %"};
 for(size_t i=0;i<knobs.size();++i){auto& k=knobs[i];k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,20);addAndMakeVisible(k.slider);k.label.setText(labels[i],juce::dontSendNotification);k.label.setJustificationType(juce::Justification::centred);addAndMakeVisible(k.label);k.attach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,ids[i],k.slider);}
 const char* cids[]={"pedal","muffVersion","feedbackCap","clipping","transformer","oversampling","tcBoostPosition","boostModel","dualMode"};
 const char* names[]={"PEDAL MODEL","BIG MUFF VERSION","TS9 / SD-1 HPF CAP","DIODE / CLIPPING","SATURATOR TRANSFORMER","OVERSAMPLING","BOOST ROUTING","BOOST CIRCUIT","OD-2 / SD-2 MODE"};
 const juce::StringArray opts[]={
 {"RAT","Big Muff","Tube Screamer","Boss SD-1","DOD Grunge FX69","DOD Death Metal FX86","Boss BD-2","Boss DS-1","Boss OD-2","Boss HM-2","Boss SD-2"},
 {"Triangle","Ram's Head"},
 {"Stock (1x)","Larger (1.5x)","Smaller (0.68x)"},
 {"Follow Design","LED Pair","Germanium Pair","Asymmetric Si","MOSFET-inspired","No Diodes","Schottky Pair","Si + LED Series (Symmetric)","Low-Vf Silicon Pair","High-Vf Silicon Pair"},
 {"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"},
 {"1x","2x","4x","8x"},
 {"Off","Pre Pedal","Post Pedal"},
 {"TC Integrated Preamp","Boss FA-1"},
 {"Normal / Crunch","Turbo / Lead"}};
 for(int i=0;i<4;++i){auto& m=mods[(size_t)i];m.setButtonText("MOD "+juce::String(i+1));addAndMakeVisible(m);modAttachments[(size_t)i]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"mod"+juce::String(i+1),m);}
 for(size_t i=0;i<choices.size();++i){auto& c=choices[i];c.label.setText(names[i],juce::dontSendNotification);addAndMakeVisible(c.label);for(int j=0;j<opts[i].size();++j)c.box.addItem(opts[i][j],j+1);addAndMakeVisible(c.box);c.attach=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,cids[i],c.box);}
 pedalBypass.setButtonText("BYPASS DISTORTION PEDAL");addAndMakeVisible(pedalBypass);
 pedalBypassAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"pedalBypass",pedalBypass);
 boostMidOn.setButtonText("MID EQ ON");addAndMakeVisible(boostMidOn);
 boostMidAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"boostMidOn",boostMidOn);
 choices[0].box.onChange=[this]{const int pedal=choices[0].box.getSelectedItemIndex();
  choices[1].box.setEnabled(pedal==1);choices[1].label.setEnabled(pedal==1);
  choices[2].box.setEnabled(pedal==2||pedal==3);choices[2].label.setEnabled(pedal==2||pedal==3);
  choices[8].box.setEnabled(pedal==8||pedal==10);choices[8].label.setEnabled(pedal==8||pedal==10);
  const char* names[11][4]={{"RUETZ","GAIN EQ / FILTER","FAT","CLEAN BLEND"},{"TONE BYPASS","MID CONTROL","CREAMY / BASS","FLAT MIDS"},{"BASS+","808 VOICING","EXTENDED BASS","BRIGHT TONE"},{"C6 FILTER","BASS MOD","EXTENDED BASS","BRIGHT TONE"},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY","CLEAN BLEND"},
  {"BASS COUPLING","PRESENCE","CLIP DENSITY","CLEAN BLEND"},
  {"PHAT BASS","PRESENCE","CLIP DENSITY","CLEAN BLEND"},
  {"BASS MOD","PRESENCE","CLIP DENSITY","CLEAN BLEND"},
  {"BASS MOD","PRESENCE","CLIP DENSITY","CLEAN BLEND"},
  {"BASS MOD","PRESENCE","CLIP DENSITY","CLEAN BLEND"},
  {"BASS MOD","PRESENCE","CLIP DENSITY","CLEAN BLEND"}};
  for(int i=0;i<4;++i)mods[(size_t)i].setButtonText(names[juce::jlimit(0,10,pedal)][i]);};
 choices[0].box.onChange();
 setSize(1060,1030);
}
void GuitarOverdriveEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff161c23));g.setColour(juce::Colour(0xfff3e4c5));g.setFont(juce::Font(juce::FontOptions(30,juce::Font::bold)));g.drawText("GUITAR OVERDRIVE",24,18,620,45,juce::Justification::centredLeft);
 g.setFont(juce::Font(juce::FontOptions(13)));g.drawText("ELEVEN PEDAL MODELS  /  MODS  /  TRANSFORMER COLOUR",26,65,800,24,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xfff3e4c5));g.setFont(juce::Font(juce::FontOptions(13)));g.drawText("STACKABLE MODS 1–4 (PEDAL-SPECIFIC) — CLIPPING ALWAYS INDEPENDENT",26,704,960,20,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xffcf9e5c));for(int y:{105,352,505})g.drawLine(24.f,(float)y,1036.f,(float)y,1.5f);
}
void GuitarOverdriveEditor::resized(){
 for(int i=0;i<6;++i){auto& c=choices[(size_t)i];const int x=25+(i%3)*340;const int y=112+(i/3)*62;c.label.setBounds(x,y,325,22);c.box.setBounds(x,y+23,325,30);}
 choices[6].label.setBounds(275,745,300,22);choices[6].box.setBounds(275,768,300,30);
 choices[7].label.setBounds(600,745,325,22);choices[7].box.setBounds(600,768,325,30);
 choices[8].label.setBounds(25,925,325,22);choices[8].box.setBounds(25,948,325,30);
 pedalBypass.setBounds(25,835,235,28);boostMidOn.setBounds(275,835,155,28);
 for(int i=0;i<4;++i)mods[(size_t)i].setBounds(25+i*250,660,235,28);
 for(int i=0;i<5;++i){auto& k=knobs[(size_t)i];int x=25+i*207;k.slider.setBounds(x,245,175,80);k.label.setBounds(x,324,175,19);}
 for(int i=5;i<9;++i){auto& k=knobs[(size_t)i];int x=25+(i-5)*207;k.slider.setBounds(x,365,175,110);k.label.setBounds(x,475,175,22);}
 for(int i=9;i<13;++i){auto& k=knobs[(size_t)i];int x=25+(i-9)*207;k.slider.setBounds(x,520,175,110);k.label.setBounds(x,630,175,22);}
 knobs[13].slider.setBounds(25,735,175,65);knobs[13].label.setBounds(25,805,175,20);
 knobs[14].slider.setBounds(480,820,175,65);knobs[14].label.setBounds(480,885,175,20);
 knobs[15].slider.setBounds(685,820,175,65);knobs[15].label.setBounds(685,885,175,20);
 knobs[16].slider.setBounds(405,925,175,65);knobs[16].label.setBounds(405,995,175,20);
 knobs[17].slider.setBounds(625,925,175,65);knobs[17].label.setBounds(625,995,175,20);
}
