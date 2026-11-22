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
 std::array<Knob,13> knobs;
 std::array<Choice,6> choices;
 std::array<juce::ToggleButton,4> mods;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,4> modAttachments;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarOverdriveEditor)
};
