#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
class GuitarOverdriveProcessor final : public juce::AudioProcessor {
public:
 GuitarOverdriveProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "Guitar Overdrive";}
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters;
 std::atomic<float> peak{-90.f};
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 struct OnePole {float z=0;float lp(float x,float hz,float sr);float hp(float x,float hz,float sr){return x-lp(x,hz,sr);}};
 struct State {
  OnePole inputHp,inputLp,outputHp,outputLp,preBass,preMid,preTop,feedback,ratTone,ratSlew;
  OnePole muffOne,muffTwo,muffToneLow,muffToneHigh,tsTone,tsPre,sdPre;
  OnePole xfLow,xfHigh,xfLeak,boostPreHp,boostPreLp,boostPostHp,boostPostLp;
  OnePole newInput,newGain1,newGain2,newBass,newTreble,newMid,newPresence,newOutput;
  float flux=0,opamp=0,stage1=0,stage2=0;
  float midPre1=0,midPre2=0,midPost1=0,midPost2=0;
  std::array<float,3> hm2EqIc1{},hm2EqIc2{};
 };
 std::array<State,2> states{};
 std::array<std::unique_ptr<juce::dsp::Oversampling<float>>,4> oversamplers;
 juce::AudioBuffer<float> scratch;
 double sr=44100;
 int maxBlock=512,fixedLatency=0,delaySize=0,write=0;
 std::array<std::vector<float>,2> dryDelay,wetDelay;
 float transformer(float,int,float,float,float);
 float tcBoost(float,int,float,float,bool,int,bool,float,float);
 float pedal(float,int,float,float,float,int,int,float,float,float,float);
 void processChunk(juce::AudioBuffer<float>&,int,int,int);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarOverdriveProcessor)
};
