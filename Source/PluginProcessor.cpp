#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace {
float coeff(float hz,float sr){return std::exp(-juce::MathConstants<float>::twoPi*juce::jlimit(1.f,sr*.45f,hz)/sr);}
float gain(float db){return juce::Decibels::decibelsToGain(db);}
float diode(float x,float positive,float negative){
 // Continuous soft diode conduction: individual positive/negative thresholds.
 const float v=x>=0?positive:negative;
 const float knee=.13f*v;
 const float mag=std::abs(x);
 if(mag<v-knee)return x;
 const float clipped=v+knee*std::tanh((mag-v)/juce::jmax(.0001f,knee));
 return std::copysign(clipped,x);
}
// A smooth, voltage-dependent conduction branch for vintage silicon pairs.
// The threshold and slope depend on effective diode series resistance and
// surrounding transistor-stage loading; not a measured device SPICE model.
float vintageSilicon(float x,float threshold,float softness){
 const float mag=std::abs(x);
 const float onset=threshold-softness;
 if(mag<=onset)return x;
 const float excess=mag-onset;
 const float limited=onset+softness*std::log1p(excess/softness);
 return std::copysign(limited,x);
}
float clip(float x,int type);
// Feedback-network approximation: the diodes shunt gain around the active
// device instead of hard-limiting the signal at the output node. Solve the
// monotonic nonlinear transfer by bisection; bounded iterations and no state.
float feedbackClip(float excitation,int type,float feedbackStrength){
 if(type==5)return excitation;
 float lo=juce::jmin(-1.f,excitation);
 float hi=juce::jmax(1.f,excitation);
 for(int i=0;i<14;++i){
  const float mid=.5f*(lo+hi);
  const float residual=mid+feedbackStrength*(mid-clip(mid,type))-excitation;
  if(residual>0.f)hi=mid;else lo=mid;
 }
 return .5f*(lo+hi);
}
float clip(float x,int type){
 switch(type){
 case 1:return diode(x,1.7f,1.7f); // LED
 case 2:return diode(x,.32f,.32f); // germanium
 case 3:return diode(x,.65f,1.3f); // asymmetric
 case 4:return diode(x,1.05f,1.05f); // MOSFET-inspired threshold
 case 5:return x;
 case 6:return diode(x,.36f,.36f); // optional Schottky-like low-Vf
 case 7:return diode(x,2.35f,2.35f); // symmetric Si+LED series string each polarity
 case 8:return diode(x,.48f,.48f); // low-forward silicon alternative
 case 9:return diode(x,.82f,.82f); // higher-forward silicon alternative
 default:return diode(x,.65f,.65f);
 }
}
}
float GuitarOverdriveProcessor::OnePole::lp(float x,float hz,float sr){
 const float a=coeff(hz,sr);z=a*z+(1.f-a)*x;return z;
}
juce::AudioProcessorValueTreeState::ParameterLayout GuitarOverdriveProcessor::layout(){
 using F=juce::AudioParameterFloat;using C=juce::AudioParameterChoice;using B=juce::AudioParameterBool;
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto f=[&](const char* id,const char* label,float lo,float hi,float def){p.add(std::make_unique<F>(juce::ParameterID{id,1},label,juce::NormalisableRange<float>(lo,hi,.1f),def));};
 f("input","Input dB",-24,24,0);f("drive","Drive %",0,100,50);f("tone","Tone %",0,100,50);f("level","Level dB",-36,18,0);f("mix","Mix %",0,100,100);
 f("modAmount","Mod Amount %",0,100,50);f("modTone","Mod Tone %",0,100,50);f("modBass","Mod Bass %",0,100,50);
 f("transformerDrive","Transformer Drive dB",-24,24,0);
 f("tcBoostGain","Preamp Boost dB",0,24,12);
 f("boostMidFreq","Preamp Mid Frequency Hz",180,7200,1100);
 f("boostMidGain","Preamp Mid Gain dB",-15,15,0);
 f("inputHP","Input HP Hz",10,1500,20);f("inputLP","Input LP Hz",1000,20000,20000);
 f("outputHP","Output HP Hz",10,1500,20);f("outputLP","Output LP Hz",1000,20000,20000);
 p.add(std::make_unique<B>(juce::ParameterID{"boostMidOn",1},"Preamp Mid EQ Enabled",false));
 p.add(std::make_unique<B>(juce::ParameterID{"pedalBypass",1},"Distortion Pedal Bypass",false));
 p.add(std::make_unique<C>(juce::ParameterID{"boostModel",1},"Boost Model",juce::StringArray{"TC Integrated Preamp","Boss FA-1"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"tcBoostPosition",1},"TC Preamp Boost",juce::StringArray{"Off","Pre Pedal","Post Pedal"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"feedbackCap",1},"TS9 / SD-1 HPF Capacitor",juce::StringArray{"Stock (1x)","Larger (1.5x)","Smaller (0.68x)"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"muffVersion",1},"Big Muff Version",juce::StringArray{"Triangle","Ram's Head"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"pedal",1},"Pedal",juce::StringArray{"RAT","Big Muff","Tube Screamer","Boss SD-1"},0));
 for(int i=1;i<=4;++i){const auto id="mod"+juce::String(i);p.add(std::make_unique<B>(juce::ParameterID{id,1},"Mod "+juce::String(i),false));}
 p.add(std::make_unique<C>(juce::ParameterID{"clipping",1},"Clipping",juce::StringArray{"Follow Design","LED Pair","Germanium Pair","Asymmetric Si","MOSFET-inspired","No Diodes","Schottky Pair","Si + LED Series (Symmetric)","Low-Vf Silicon Pair","High-Vf Silicon Pair"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"transformer",1},"Transformer",juce::StringArray{"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"oversampling",1},"Oversampling",juce::StringArray{"1x","2x","4x","8x"},2));
 return p;
}
GuitarOverdriveProcessor::GuitarOverdriveProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"STATE",layout()){}
void GuitarOverdriveProcessor::prepareToPlay(double sampleRate,int block){
 sr=sampleRate;maxBlock=juce::jmax(1,block);states={};scratch.setSize(2,maxBlock);
 for(int i=0;i<4;++i){
  oversamplers[(size_t)i]=std::make_unique<juce::dsp::Oversampling<float>>(2,(size_t)i,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,false);
  oversamplers[(size_t)i]->initProcessing((size_t)maxBlock);oversamplers[(size_t)i]->reset();
 }
 fixedLatency=0;
 for(const auto& os:oversamplers)fixedLatency=juce::jmax(fixedLatency,(int)std::ceil(os->getLatencyInSamples()));
 delaySize=juce::jmax(2048,fixedLatency+maxBlock+8);
 for(auto& v:dryDelay)v.assign((size_t)delaySize,0.f);
 for(auto& v:wetDelay)v.assign((size_t)delaySize,0.f);
 write=0;setLatencySamples(fixedLatency);
}
float GuitarOverdriveProcessor::transformer(float x,int channel,float drive,float character,float rate){
 const int model=juce::jlimit(0,7,(int)parameters.getRawParameterValue("transformer")->load());
 if(model==0)return x;
 // Port of Saturator transformer: low/high loading, leaky flux integration,
 // magnetic nonlinear branch and independently normalised amplifier branch.
 const float hp[]={0,22,12,24,9,28,14,18};
 const float lp[]={0,19000,17500,21000,22000,15500,18500,19500};
 const float asym[]={0,.035f,.085f,.075f,.045f,.02f,.095f,.045f};
 const float magnetDrive[]={0,.65f,1.1f,.8f,1.3f,.7f,1.35f,1.0f};
 const float amplifierDrive[]={0,.25f,.9f,.8f,1.1f,.2f,1.0f,1.15f};
 auto& s=states[(size_t)channel];
 const float low=s.xfLow.lp(x,hp[model],rate);
 const float high=s.xfHigh.lp(x,lp[model],rate);
 const float ac=x-low;
 const float leak=coeff(hp[model]*.55f,rate);
 s.flux=leak*s.flux+(1.f-leak)*ac*(140.f/juce::jmax(10.f,hp[model]));
 const float field=s.flux*magnetDrive[model]*drive*.22f;
 const float coreDist=(std::tanh(field)-field)*(.04f+.15f*character);
 const float ampGain=drive*(.65f+amplifierDrive[model]*character);
 const float offset=asym[model];
 const float amp=(std::tanh(x*ampGain+offset)-std::tanh(offset))/juce::jmax(.1f,ampGain);
 const float blend=.14f+.36f*character;
 return (1.f-blend)*x+blend*amp+coreDist-(x-high)*(.015f+.065f*character);
}
float GuitarOverdriveProcessor::tcBoost(float x,int channel,float boostDb,float rate,bool pre,int model,bool midOn,float midHz,float midDb){
 auto& st=states[(size_t)channel];
 auto& hp=pre?st.boostPreHp:st.boostPostHp;
 auto& lp=pre?st.boostPreLp:st.boostPostLp;
 // Two distinct boost voicings: TC tighter input / extended treble;
 // FA-1 broader low end, softer upper bandwidth and output drive.
 const float coupled=hp.hp(x,model==0?38.f:16.f,rate);
 float shaped=lp.lp(coupled,model==0?17000.f:12500.f,rate);
 if(midOn && std::abs(midDb)>.001f){
  // TPT state-variable bell with broad 1073-inspired proportional-Q response.
  // Gain-dependent bandwidth: broader at small boosts/cuts, slightly narrower
  // at larger settings. Not a recreation of the 1073 inductor circuit.
  const float q=.72f+.032f*std::abs(midDb);
  const float g=std::tan(juce::MathConstants<float>::pi*juce::jlimit(100.f,rate*.40f,midHz)/rate);
  const float k=1.f/q;
  float& ic1=pre?st.midPre1:st.midPost1;
  float& ic2=pre?st.midPre2:st.midPost2;
  const float a1=1.f/(1.f+g*(g+k));
  const float v1=a1*(ic1+g*(shaped-ic2));
  const float v2=ic2+g*v1;
  ic1=2.f*v1-ic1;ic2=2.f*v2-ic2;
  const float a=std::pow(10.f,midDb/40.f);
  shaped+=(a-1.f/a)*k*v1;
 }
 const float driven=shaped*gain(boostDb);
 const float rail=model==0?9.f:6.5f;
 return rail*std::tanh(driven/rail);
}
float GuitarOverdriveProcessor::pedal(float x,int channel,float drive,float tone,float level,int model,int mod,float amount,float modTone,float modBass,float rate){
 auto& s=states[(size_t)channel];
 const int clipping=juce::jlimit(0,9,(int)parameters.getRawParameterValue("clipping")->load());
 const bool m1=parameters.getRawParameterValue("mod1")->load()>.5f;
 const bool m2=parameters.getRawParameterValue("mod2")->load()>.5f;
 const bool m3=parameters.getRawParameterValue("mod3")->load()>.5f;
 const bool m4=parameters.getRawParameterValue("mod4")->load()>.5f;
 float y=x;
 if(model==0){
  // RAT: frequency-selective high-gain op-amp followed by hard diodes to ground.
  const float ruetz=m1?amount:0.f;
  const float fat=m3?amount:0.f;
  const float pre=s.preBass.hp(x,45.f+135.f*(1.f-fat),rate);
  const float upper=s.preTop.lp(pre,juce::jmax(900.f,13000.f-7000.f*drive),rate);
  const float opGain=1.f+90.f*drive*drive*(1.f-.58f*ruetz);
  const float desired=upper*opGain;
  // LM308-style slew-limited stage, with op-amp supply rail approximation.
  const float maxStep=juce::jmax(.00001f,300000.f/rate/9.f);
  s.opamp+=juce::jlimit(-maxStep,maxStep,desired-s.opamp);
  const float op=juce::jlimit(-4.f,4.f,s.opamp);
  const int diodeType=clipping;
  y=clip(op,diodeType);
  // RAT Filter rotates clockwise towards darker sound.
  const float cutoff=350.f*std::pow(50.f,1.f-tone);
  y=s.ratTone.lp(y,cutoff,rate);
  if(m4)y=(1.f-.5f*amount)*y+.5f*amount*x;
  y*=.60f;
 }else if(model==1){
  // Big Muff: two cascaded transistor gain/diode clipping stages, then
  // a passive bass/treble blend with a characteristic mid scoop.
  // Triangle and Ram's Head both used silicon diode pairs. Component
  // substitutions occurred across production; values here are approximations.
  const bool ram=parameters.getRawParameterValue("muffVersion")->load()>.5f;
  const float bassBoost=m3?modBass:0.f;
  const float pre=s.preBass.hp(x,(ram?57.f:38.f)-20.f*bassBoost,rate);
  const float booster=s.preTop.lp(pre,ram?9200.f:12500.f,rate)*(ram?1.75f:2.05f);
  const float a=s.muffOne.lp(booster,ram?4900.f:6700.f,rate)
      *(ram?2.8f+11.f*drive:3.2f+13.f*drive);
  const float first=clipping==0?vintageSilicon(a,ram?.68f:.64f,ram?.22f:.18f):feedbackClip(a,clipping,ram?3.4f:3.0f);
  const float b=s.muffTwo.lp(first,ram?3700.f:5200.f,rate)
      *(ram?2.7f+8.5f*drive:3.0f+10.f*drive);
  const float second=clipping==0?vintageSilicon(b,ram?.66f:.63f,ram?.20f:.17f):feedbackClip(b,clipping,ram?3.8f:3.2f);
  const float low=s.muffToneLow.lp(second,(ram?365.f:470.f)+220.f*modBass,rate);
  const float high=second-s.muffToneHigh.lp(second,(ram?1050.f:820.f)+800.f*modTone,rate);
  const float scoop=m2?(.15f+.85f*amount):(m4?.15f:1.f);
  const float scooped=low*(1.f-tone)+high*tone;
  const float flat=.55f*second+.45f*scooped;
  y=scoop*scooped+(1.f-scoop)*flat;
  if(m1)y=second;
  if(m3)y=.68f*y+.32f*std::tanh(second*1.5f);
  y*=.8f;
 }else{
  // Tube Screamer and SD-1: feedback-loop diode clipping, unlike RAT.
  // The input bass corner shapes which frequencies reach the feedback clipper.
  const bool sd=model==3;
  const int cap=juce::jlimit(0,2,(int)parameters.getRawParameterValue("feedbackCap")->load());
  const float capRatio=cap==1?1.5f:(cap==2?.68f:1.f);
  float bassCorner=720.f/capRatio;
  if(m1 && !sd)bassCorner=(720.f-490.f*amount)/capRatio; // TS bass+
  if(m2 && sd)bassCorner=(720.f-480.f*amount)/capRatio; // SD-1 bass
  if(m3)bassCorner=juce::jmin(bassCorner,(720.f-350.f*modBass)/capRatio);
  const float filtered=(sd?s.sdPre:s.tsPre).hp(x,bassCorner,rate);
  const float amplified=filtered*(1.5f+18.f*drive*drive);
  // Stock SD-1 has asymmetrical clipping, stock TS symmetric.
  int diodeType=clipping;
  if(clipping==0 && sd)diodeType=3;
  // Clipping is exclusively selected by the independent global diode menu.
  
  // Soft feedback limiting retains more signal dynamics than RAT hard clip.
  const float clipped=feedbackClip(amplified,diodeType,sd?5.2f:4.3f);
  const float clippedGain=clipped/juce::jmax(.25f,1.f+drive*1.5f);
  y=.22f*x+.78f*clippedGain;
  if(m2&&!sd)y=.27f*x+.73f*clippedGain; // TS808-style response
  const float cutoff=850.f*std::pow(12.f,tone)*(m4?(1.f+.5f*modTone):1.f);
  y=s.tsTone.lp(y,cutoff,rate);
  if(m1&&sd)y=s.preMid.lp(y,8500.f-3500.f*amount,rate); // C6 mod
  y*=1.25f;
 }
 return y*gain(level);
}
void GuitarOverdriveProcessor::processChunk(juce::AudioBuffer<float>& buffer,int count,int channels,int osIndex){
 const auto v=[&](const char* id){return parameters.getRawParameterValue(id)->load();};
 const float in=gain(v("input")),level=v("level"),drive=v("drive")*.01f,tone=v("tone")*.01f,mix=v("mix")*.01f;
 const float amount=v("modAmount")*.01f,modTone=v("modTone")*.01f,modBass=v("modBass")*.01f;
 const float xfDrive=gain(v("transformerDrive")),ih=v("inputHP"),il=v("inputLP"),oh=v("outputHP"),ol=v("outputLP");
 const int boostPosition=juce::jlimit(0,2,(int)v("tcBoostPosition"));
 const int boostModel=juce::jlimit(0,1,(int)v("boostModel"));
 const bool boostMidOn=v("boostMidOn")>.5f;
 const bool bypassPedal=v("pedalBypass")>.5f;
 const float boostMidHz=v("boostMidFreq"),boostMidDb=v("boostMidGain");
 const float boostDb=v("tcBoostGain");
 const int model=juce::jlimit(0,3,(int)v("pedal")),mod=0; // Legacy argument retained; mods are independent booleans.
 for(int c=channels;c<2;++c)scratch.clear(c,0,count);
 for(int c=0;c<channels;++c){const float* src=buffer.getReadPointer(c);float* dst=scratch.getWritePointer(c);for(int i=0;i<count;++i)dst[i]=src[i]*in;}
 auto& os=*oversamplers[(size_t)osIndex];
 auto block=juce::dsp::AudioBlock<float>(scratch).getSubBlock(0,(size_t)count);
 juce::dsp::AudioBlock<const float> inputBlock(block);
 auto up=os.processSamplesUp(inputBlock);
 const float rate=(float)sr*(float)os.getOversamplingFactor();
 for(int c=0;c<channels;++c){auto* samples=up.getChannelPointer((size_t)c);auto& s=states[(size_t)c];
  for(size_t i=0;i<up.getNumSamples();++i){
   float x=s.inputHp.hp(samples[i],ih,rate);
   x=s.inputLp.lp(x,il,rate);
   if(boostPosition==1)x=tcBoost(x,c,boostDb,rate,true,boostModel,boostMidOn,boostMidHz,boostMidDb);
   if(!bypassPedal)x=pedal(x,c,drive,tone,level,model,mod,amount,modTone,modBass,rate);
   if(boostPosition==2)x=tcBoost(x,c,boostDb,rate,false,boostModel,boostMidOn,boostMidHz,boostMidDb);
   x=transformer(x,c,xfDrive,.5f,rate);
   x=s.outputHp.hp(x,oh,rate);
   samples[i]=s.outputLp.lp(x,ol,rate);
  }
 }
 os.processSamplesDown(block);
 const int wetPadding=juce::jlimit(0,delaySize-1,fixedLatency-(int)std::round(os.getLatencyInSamples()));
 float peakValue=0;
 for(int c=0;c<channels;++c){auto* dst=buffer.getWritePointer(c);const float* wet=scratch.getReadPointer(c);
  auto& dryRing=dryDelay[(size_t)c];auto& wetRing=wetDelay[(size_t)c];
  for(int i=0;i<count;++i){
   const int pos=(write+i)%delaySize;
   const int dryRead=(pos+delaySize-fixedLatency)%delaySize;
   const int wetRead=(pos+delaySize-wetPadding)%delaySize;
   const float dry=fixedLatency==0?dst[i]*in:dryRing[(size_t)dryRead];
   const float aligned=wetPadding==0?wet[i]:wetRing[(size_t)wetRead];
   dryRing[(size_t)pos]=dst[i]*in;
   wetRing[(size_t)pos]=wet[i];
   dst[i]=dry*(1.f-mix)+aligned*mix;
   peakValue=juce::jmax(peakValue,std::abs(dst[i]));
  }
 }
 write=(write+count)%delaySize;
 peak.store(juce::Decibels::gainToDecibels(juce::jmax(peakValue,1e-7f),-90.f));
}
void GuitarOverdriveProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard;
 const int n=buffer.getNumSamples(),channels=juce::jmin(2,buffer.getNumChannels());
 if(n<=0||channels<=0)return;
 for(int c=channels;c<buffer.getNumChannels();++c)buffer.clear(c,0,n);
 const int os=juce::jlimit(0,3,(int)parameters.getRawParameterValue("oversampling")->load());
 for(int pos=0;pos<n;pos+=maxBlock){
  const int count=juce::jmin(maxBlock,n-pos);
  float* ptrs[2]={buffer.getWritePointer(0,pos),channels>1?buffer.getWritePointer(1,pos):nullptr};
  juce::AudioBuffer<float> chunk(ptrs,channels,count);
  processChunk(chunk,count,channels,os);
 }
}
juce::AudioProcessorEditor* GuitarOverdriveProcessor::createEditor(){return new GuitarOverdriveEditor(*this);}
void GuitarOverdriveProcessor::getStateInformation(juce::MemoryBlock& b){auto xml=parameters.copyState().createXml();copyXmlToBinary(*xml,b);}
void GuitarOverdriveProcessor::setStateInformation(const void* data,int bytes){if(auto xml=getXmlFromBinary(data,bytes))if(xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new GuitarOverdriveProcessor();}
