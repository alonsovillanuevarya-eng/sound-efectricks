#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SoundEfectricksAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit SoundEfectricksAudioProcessorEditor (SoundEfectricksAudioProcessor&);
    ~SoundEfectricksAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureSlider (juce::Slider&, juce::Label&, const juce::String&);
    SoundEfectricksAudioProcessor& processor;
    juce::OwnedArray<juce::TextButton> cells;
    juce::Label title, subtitle, crystalTitle;
    juce::Slider mix, feedback, pitch, grain, spread, cutoff, crush, stutter, master;
    juce::Label mixLabel, feedbackLabel, pitchLabel, grainLabel, spreadLabel,
                cutoffLabel, crushLabel, stutterLabel, masterLabel;
    std::array<juce::String, 8> rowNames { "STUTTER", "LOOP", "REVERSE", "CRUSH",
                                          "FILTER", "PHASER", "SPACE", "CRYSTAL" };
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundEfectricksAudioProcessorEditor)
};
