#pragma once
#include "PluginProcessor.h"
class GuitarOverdriveEditor final : public juce::AudioProcessorEditor {
public:
 explicit GuitarOverdriveEditor(GuitarOverdriveProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 GuitarOverdriveProcessor& processor;
 struct Knob {juce::Slider slider;juce::Label label;std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;};
 struct Choice {juce::ComboBox box;juce::Label label;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attach;};
 std::array<Knob,18> knobs;
 std::array<Knob,4> modKnobs;
 std::array<Choice,11> choices;
 juce::ToggleButton pedalBypass,boostMidOn;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pedalBypassAttachment,boostMidAttachment;
 std::array<juce::ToggleButton,4> mods;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,4> modAttachments;
 void updateContext();
 void placeKnob(int index,juce::Rectangle<int> bounds);
 void placeChoice(int index,juce::Rectangle<int> bounds);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarOverdriveEditor)
};
