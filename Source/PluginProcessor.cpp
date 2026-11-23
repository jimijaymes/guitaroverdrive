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
// Implicit diode-feedback stage: a monotonic exponential I/V branch.
// The scaling is calibrated in normalized audio units, not literal volts.
// Asymmetric mode uses one silicon junction in one direction and two in
// series in the other, as in the stock SD-1 feedback network.
float diodeFeedbackStage(float excitation,float strength,float positiveV,float negativeV)
{
 const float extent=juce::jmax(1.f,std::abs(excitation));
 float lo=-extent,hi=extent;
 for(int i=0;i<18;++i)
 {
  const float v=.5f*(lo+hi);
  const float threshold=v>=0.f?positiveV:negativeV;
  const float thermal=.055f*(threshold/.65f);
  const float excess=juce::jmax(0.f,std::abs(v)-threshold*.65f);
  const float conduction=thermal*std::expm1(juce::jmin(16.f,excess/thermal));
  const float residual=v+std::copysign(strength*conduction,v)-excitation;
  if(residual>0.f)hi=v;else lo=v;
 }
 return .5f*(lo+hi);
}
// Feedback-network approximation: the diodes shunt gain around the active
// device instead of hard-limiting the signal at the output node. Solve the
// monotonic nonlinear transfer by bisection; bounded iterations and no state.
float feedbackClip(float excitation,int type,float feedbackStrength){
 if(type==5)return excitation;
 // Feedback diode conduction is monotonic; bracket the actual solution.
 // The previous fixed [-1,+1] bracket saturated high-level inputs even
 // when the chosen diode threshold was much higher.
 const float extent=juce::jmax(1.f,std::abs(excitation));
 float lo=-extent,hi=extent;
 for(int i=0;i<20;++i){
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
 for(int i=1;i<=4;++i){const auto id="modControl"+juce::String(i);p.add(std::make_unique<F>(juce::ParameterID{id,1},"Mod "+juce::String(i)+" Control",juce::NormalisableRange<float>(0.f,100.f,.1f),50.f));}
 f("modAmount","Mod Amount %",0,100,50);f("modTone","Mod Tone %",0,100,50);f("modBass","Mod Bass %",0,100,50);
 f("transformerDrive","Transformer Drive dB",-24,24,0);
 f("tcBoostGain","Preamp Boost dB",0,24,12);
 f("boostMidFreq","Preamp Mid Frequency Hz",180,7200,1100);
 f("boostMidGain","Preamp Mid Gain dB",-15,15,0);
 f("pedalBass","New Pedal Bass %",0,100,50);f("pedalTreble","New Pedal Treble %",0,100,50);
 f("inputHP","Input HP Hz",10,1500,20);f("inputLP","Input LP Hz",1000,20000,20000);
 f("outputHP","Output HP Hz",10,1500,20);f("outputLP","Output LP Hz",1000,20000,20000);
 p.add(std::make_unique<B>(juce::ParameterID{"boostMidOn",1},"Preamp Mid EQ Enabled",false));
 p.add(std::make_unique<B>(juce::ParameterID{"pedalBypass",1},"Distortion Pedal Bypass",false));
 p.add(std::make_unique<C>(juce::ParameterID{"boostModel",1},"Boost Model",juce::StringArray{"TC Integrated Preamp","Boss FA-1"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"tcBoostPosition",1},"TC Preamp Boost",juce::StringArray{"Off","Pre Pedal","Post Pedal"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"modCircuit",1},"Verified Mod Circuit",juce::StringArray{"Stock","Mod A","Mod B","Mod C"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"dualMode",1},"OD-2 / SD-2 Mode",juce::StringArray{"Normal / Crunch","Turbo / Lead"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"feedbackCap",1},"TS9 / SD-1 HPF Capacitor",juce::StringArray{"Stock (1x)","Larger (1.5x)","Smaller (0.68x)"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"muffVersion",1},"Big Muff Version",juce::StringArray{"Triangle","Ram's Head"},0));
 p.add(std::make_unique<C>(juce::ParameterID{"pedal",1},"Pedal",juce::StringArray{"RAT","Big Muff","Tube Screamer","Boss SD-1","DOD FX69 Grunge","DOD FX86 Death Metal","Boss BD-2","Boss DS-1","Boss OD-2","Boss HM-2","Boss SD-2"},0));
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
 const int modCircuit=juce::jlimit(0,3,(int)parameters.getRawParameterValue("modCircuit")->load());
 const auto modValue=[this](int i){return parameters.getRawParameterValue("modControl"+juce::String(i))->load()*.01f;};
 const float a1=modValue(1),a2=modValue(2),a3=modValue(3),a4=modValue(4);
 float y=x;
 if(model==0){
  // RAT: frequency-selective high-gain op-amp followed by hard diodes to ground.
  const float ruetz=m1?a1:0.f;
  const float fat=m3?a3:0.f;
  // RAT gain network has two independent feedback high-pass branches:
  // 47R/2.2uF ~1.54 kHz and 560R/4.7uF ~60.5 Hz. Ruetz changes
  // the 47R leg; the 560R leg remains in circuit.
  const float hpFast=s.preBass.hp(x,1539.f*(1.f+12.f*ruetz),rate);
  const float hpSlow=s.preMid.hp(x,60.5f*(1.f-.65f*fat),rate);
  const float upper=s.preTop.lp(x,20000.f,rate);
  // Effective feedback gain rises nonlinearly with distortion control.
  // Bound the gain to avoid driving the op-amp into its rails at nearly
  // every knob position; the diode clipper remains a separate stage.
  const float opGain=1.f+juce::jmin(450.f,420.f*drive*drive)*(1.f-.58f*ruetz);
  const float desired=upper+hpFast*(opGain-1.f)*.55f+hpSlow*(opGain-1.f)*.45f;
  // LM308-style slew-limited stage, with op-amp supply rail approximation.
  // Slew limit in normalized audio units per sample (approximate,
  // not a literal LM308 voltage-to-digital conversion).
  // LM308 slew rate approximately 0.3 V/us. Treat one normalized unit
  // as one volt at the virtual-ground-referenced op-amp output.
  const float maxStep=300000.f/rate;
  const float slewLimited=s.opamp+juce::jlimit(-maxStep,maxStep,desired-s.opamp);
  // Gain-bandwidth limitation: at higher distortion gain the closed-loop
  // stage cannot follow the same high-frequency excursions.
  const float gbwHz=500000.f;
  const float closedLoopBandwidth=juce::jlimit(120.f,rate*.45f,gbwHz/juce::jmax(1.f,opGain));
  s.opamp=s.ratSlew.lp(slewLimited,closedLoopBandwidth,rate);
  const float op=juce::jlimit(-4.f,4.f,s.opamp);
  const int diodeType=clipping;
  y=clip(op,diodeType);
  // RAT Filter rotates clockwise towards darker sound.
  const float cutoff=350.f*std::pow(50.f,1.f-tone);
  y=s.ratTone.lp(y,cutoff,rate);
  if(m4)y=(1.f-.5f*a4)*y+.5f*a4*x;
  y*=.60f;
 }else if(model==1){
  // Big Muff: two cascaded transistor gain/diode clipping stages, then
  // a passive bass/treble blend with a characteristic mid scoop.
  // Triangle and Ram's Head both used silicon diode pairs. Component
  // substitutions occurred across production; values here are approximations.
  const bool ram=parameters.getRawParameterValue("muffVersion")->load()>.5f;
  const float bassBoost=m3?a3:0.f;
  const float pre=s.preBass.hp(x,(ram?57.f:38.f)-20.f*bassBoost,rate);
  const float booster=s.preTop.lp(pre,ram?9200.f:12500.f,rate)*(ram?1.75f:2.05f);
  const float a=s.muffOne.lp(booster,ram?4900.f:6700.f,rate)
      *(ram?2.8f+11.f*drive:3.2f+13.f*drive);
  const float first=clipping==0?vintageSilicon(a,ram?.68f:.64f,ram?.22f:.18f):feedbackClip(a,clipping,ram?3.4f:3.0f);
  const float b=s.muffTwo.lp(first,ram?3700.f:5200.f,rate)
      *(ram?2.7f+8.5f*drive:3.0f+10.f*drive);
  const float second=clipping==0?vintageSilicon(b,ram?.66f:.63f,ram?.20f:.17f):feedbackClip(b,clipping,ram?3.8f:3.2f);
  const float low=s.muffToneLow.lp(second,(ram?365.f:470.f)+220.f*(m3?a3:0.f),rate);
  const float high=second-s.muffToneHigh.lp(second,(ram?1050.f:820.f)+800.f*(m4?a4:0.f),rate);
  const float scoop=m2?(.15f+.85f*a2):(m4?.15f:1.f);
  const float scooped=low*(1.f-tone)+high*tone;
  const float flat=.55f*second+.45f*scooped;
  y=scoop*scooped+(1.f-scoop)*flat;
  if(m1)y=second;
  if(m3)y=.68f*y+.32f*std::tanh(second*1.5f);
  y*=.8f;
 }else if(model>=4){
  // First-pass family-specific circuit approximations. Stock voicings are
  // defaults; four independent switches add circuit-inspired modification
  // options, not claims of exact boutique component substitutions.
  const bool dual=parameters.getRawParameterValue("dualMode")->load()>.5f;
  const float bass=parameters.getRawParameterValue("pedalBass")->load()*.01f;
  const float treble=parameters.getRawParameterValue("pedalTreble")->load()*.01f;
  const float fat=m1?a1:0.f, bright=m2?a2:0.f;
  const float hpf[]={95.f,120.f,35.f,110.f,75.f,90.f,75.f};
  const float lpf[]={12500.f,10500.f,17000.f,11500.f,13500.f,12500.f,15500.f};
  const int idx=model-4;
  const float circuitBass=(model==7 && modCircuit>0)?0.70f:(model==6 && modCircuit>0?0.76f:1.f);
  const float pre=s.newInput.hp(x,hpf[idx]*(1.f-.65f*fat)*circuitBass,rate);
  const float filtered=s.newGain1.lp(pre,lpf[idx]*(1.f+.28f*bright),rate);
  float a=0.f,b=0.f;
  if(model==4){ // DOD FX69 Grunge: cascaded high gain, scooped active-like tone
   a=feedbackClip(filtered*(3.f+29.f*drive),clipping==0?3:clipping,3.2f);
   b=clip(s.newGain2.lp(a,8500.f,rate)*(2.5f+9.f*drive),clipping==0?0:clipping);
   const float low=s.newBass.lp(b,220.f+350.f*bass,rate);
   const float high=b-s.newTreble.lp(b,1200.f+4800.f*treble,rate);
   y=.9f*low*(.3f+1.4f*bass)+high*(.2f+1.6f*treble);
  }else if(model==5){ // DOD FX86 Death Metal: fixed heavy gain, two-band EQ
   a=feedbackClip(filtered*(8.f+23.f*drive),clipping==0?3:clipping,5.f);
   b=clip(s.newGain2.lp(a*(4.f+8.f*drive),7800.f,rate),clipping==0?0:clipping);
   const float low=s.newBass.lp(b,180.f+420.f*bass,rate);
   const float high=b-s.newTreble.lp(b,1800.f+3900.f*treble,rate);
   y=low*(.25f+1.5f*bass)+high*(.2f+1.6f*treble);
  }else if(model==6){ // BD-2: two cascaded nonlinear feedback gain sections
   a=feedbackClip(filtered*(1.2f+10.f*drive),clipping==0?(modCircuit>0?3:0):clipping,2.3f);
   b=feedbackClip(s.newGain2.lp(a,12500.f,rate)*(1.4f+5.f*drive),clipping==0?(modCircuit>0?3:0):clipping,2.8f);
   y=s.newTreble.lp(b,(modCircuit>0?850.f:1100.f)+12500.f*tone*(1.f+.4f*bright),rate);
   y=.14f*x+.86f*y;
  }else if(model==7){ // DS-1: component-derived modern DS-1A signal path
   // ElectroSmash DS-1 analysis: input 7.2 Hz, C2/R4 3.3 Hz,
   // C3/R5 33 Hz, C5/R10 23 Hz; Q2 ~56x (35 dB);
   // op-amp gain 1+VR1/4.7k (1..22.3), C8/R13 ~72 Hz,
   // R14/C10 7.2 kHz small-signal, tone LP 234 / HP 1063 Hz.
   // Filter loading, transistor I-V and op-amp dynamics are still approximated.
   const float input=s.dsInput.hp(x,7.2f,rate);
   const float coupled=s.dsCouple1.hp(input,3.3f,rate);
   const float bassCut=s.dsCouple2.hp(coupled,33.f*(1.f-.40f*fat),rate);
   // 9 V single-supply transistor Q2: asymmetric headroom and smooth knee.
   // Keeley mods do not all change the Q2 gain; retain stock unless
   // the selected component patch explicitly requires a change.
   const float q2Gain=56.f;
   const float q2Input=bassCut*q2Gain;
   const float q2= q2Input>=0.f?4.0f*std::tanh(q2Input/4.0f)
                               :3.0f*std::tanh(q2Input/3.0f);
   const float opInput=s.dsCouple3.hp(q2,23.f,rate);
   // Non-inverting stage: low frequencies pass with gain 1, high
   // frequencies receive the gain-dependent feedback boost.
   const float feedbackHigh=s.dsFeedbackHp.hp(opInput,72.f,rate);
   const float opGain=1.f+21.2766f*drive;
   const float opOut=s.dsOpLow.lp(opInput+(opGain-1.f)*feedbackHigh,
                                18000.f,rate);
   const float railLimited=juce::jlimit(-4.f,4.f,opOut);
   // D4/D5 shunt-to-ground clipper, AFTER R14/C10; diode override
   // replaces these devices rather than adding another clip stage.
   // Seeing Eye: silicon + red LED. Ultra: Si+LED series / red LED.
   const float diodeInput=s.dsClipLow.lp(railLimited,7200.f,rate);
   if(clipping==0 && modCircuit>0){
    const float pos=modCircuit>=2?2.35f:.65f;
    b=diode(diodeInput,pos,1.7f);
   }else b=clip(diodeInput,clipping);
   // Passive Big Muff-style two-branch tone network: 234 Hz LP and
   // 1063 Hz HP. Keep fixed corners; the pot blends their outputs.
   const float low=s.dsToneLow.lp(b,234.f,rate);
   const float high=s.dsToneHigh.hp(b,1063.f,rate);
   const float passiveBlend=(1.f-tone)*low+tone*high;
   // Approximate nominal 12 dB passive insertion loss.
   y=.25f*passiveBlend;
   y=s.dsOutput.hp(y,3.4f,rate);
  }else if(model==8){ // OD-2: normal and turbo use different gain distribution
   const float first=dual?2.8f+12.f*drive:1.5f+7.f*drive;
   a=feedbackClip(filtered*first,clipping==0?3:clipping,dual?4.2f:2.8f);
   b=dual?feedbackClip(s.newGain2.hp(a,140.f,rate)*(2.5f+8.f*drive),clipping==0?3:clipping,4.8f):a;
   y=s.newTreble.lp(.17f*x+.83f*b,900.f+11000.f*tone,rate);
  }else if(model==9){ // HM-2: high-gain stages plus resonant dual-band EQ
   a=feedbackClip(filtered*(3.f+24.f*drive),clipping==0?3:clipping,4.5f);
   b=clip(s.newGain2.lp(a*(2.5f+8.f*drive),6500.f,rate),clipping);
   // Stock HM-2 EQ: three gyrator-based series-resonant branches.
   // 87 Hz Low; 960 Hz + 1279 Hz jointly controlled by High.
   // TPT SVF bandpass approximation to the resonant branches, with
   // reciprocal boost/cut about 50% (rather than bass/treble shelves).
   // Mod C approximates the published Dunwich lower-tuned gyrator set.
   const bool lowTuned=modCircuit==3;
   const float frequencies[3]={lowTuned?62.48f:86.8f,
                               lowTuned?716.67f:958.5f,
                               lowTuned?1065.5f:1278.6f};
   const float controls[3]={bass,treble,treble};
   y=b;
   for(int band=0;band<3;++band){
    const float frequency=juce::jmin(frequencies[band],rate*.40f);
    const float q=band==0?2.5f:2.2f;
    const float g=std::tan(juce::MathConstants<float>::pi*frequency/rate);
    const float k=1.f/q;
    const float a1=1.f/(1.f+g*(g+k));
    float& ic1=s.hm2EqIc1[(size_t)band];
    float& ic2=s.hm2EqIc2[(size_t)band];
    const float v1=a1*(ic1+g*(b-ic2));
    const float v2=ic2+g*v1;
    ic1=2.f*v1-ic1;
    ic2=2.f*v2-ic2;
    const float db=(controls[band]-.5f)*(band==0?32.f:18.f);
    const float A=std::pow(10.f,db/40.f);
    y+=(A-1.f/A)*k*v1;
   }
  }else{ // SD-2: crunch and lead use distinct gain and clipping stages
   a=feedbackClip(filtered*(1.5f+(dual?14.f:8.f)*drive),clipping==0?3:clipping,dual?3.8f:2.7f);
   b=dual?feedbackClip(s.newGain2.hp(a,190.f,rate)*(2.f+9.f*drive),clipping==0?3:clipping,4.6f):a;
   y=s.newTreble.lp((dual?.10f:.22f)*x+(dual?.90f:.78f)*b,800.f+12500.f*tone,rate);
  }
  if(m3)y=.75f*y+.25f*std::tanh(y*(1.f+2.f*a3));
  if(m4)y=.82f*y+.18f*x; // optional clean blend
  y*=.55f;
 }else{
  // Tube Screamer and SD-1: feedback-loop diode clipping, unlike RAT.
  // The input bass corner shapes which frequencies reach the feedback clipper.
  const bool sd=model==3;
  const int cap=juce::jlimit(0,2,(int)parameters.getRawParameterValue("feedbackCap")->load());
  const float capRatio=cap==1?1.5f:(cap==2?.68f:1.f);
  float bassCorner=720.f/capRatio;
  if(m1 && !sd)bassCorner=(720.f-490.f*a1)/capRatio; // TS bass+
  if(m2 && sd)bassCorner=(720.f-480.f*a2)/capRatio; // SD-1 bass
  if(m3)bassCorner=juce::jmin(bassCorner,(720.f-350.f*a3)/capRatio);
  const float filtered=(sd?s.sdPre:s.tsPre).hp(x,bassCorner,rate);
  const float amplified=filtered*(1.5f+18.f*drive*drive);
  // Stock SD-1 has asymmetrical clipping, stock TS symmetric.
  int diodeType=clipping;
  if(clipping==0 && sd)diodeType=3;
  // Clipping is exclusively selected by the independent global diode menu.
  
  // Soft feedback limiting retains more signal dynamics than RAT hard clip.
  // Use the exponential implicit feedback network for stock silicon
  // clipping. Explicit alternate diode selections keep their existing
  // user-selectable clipping behaviour.
  const float clipped=clipping==0
      ? diodeFeedbackStage(amplified,sd?0.55f:0.48f,.65f,sd?1.30f:.65f)
      : feedbackClip(amplified,diodeType,sd?5.2f:4.3f);
  const float clippedGain=clipped/juce::jmax(.25f,1.f+drive*1.5f);
  y=.22f*x+.78f*clippedGain;
  if(m2&&!sd)y=.27f*x+.73f*clippedGain; // TS808-style response
  const float cutoff=850.f*std::pow(12.f,tone)*(m4?(1.f+.5f*a4):1.f);
  y=s.tsTone.lp(y,cutoff,rate);
  if(m1&&sd)y=s.preMid.lp(y,8500.f-3500.f*a1,rate); // C6 mod
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
 const int model=juce::jlimit(0,10,(int)v("pedal")),mod=0; // Legacy argument retained; mods are independent booleans.
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
