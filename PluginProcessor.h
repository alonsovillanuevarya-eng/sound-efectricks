#pragma once
#include <JuceHeader.h>
#include "CrystalEngine.h"

class SoundEfectricksAudioProcessor : public juce::AudioProcessor
{
public:
    SoundEfectricksAudioProcessor();
    ~SoundEfectricksAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SOUND EFECTRICKS"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    bool isStepEnabled (int row, int step) const;
    void setStepEnabled (int row, int step, bool enabled);
    int getCurrentStep() const { return currentStep.load(); }

private:
    static constexpr int rows = 8, steps = 32;
    std::array<std::array<std::atomic<bool>, steps>, rows> pattern;
    std::atomic<int> currentStep { 0 };
    CrystalEngine crystal;
    juce::AudioBuffer<float> scratch;
    juce::dsp::StateVariableTPTFilter<float> filterL, filterR;
    juce::dsp::Phaser<float> phaser;
    juce::Reverb reverb;
    juce::Random random;
    double sr = 44100.0;
    double fallbackPpq = 0.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundEfectricksAudioProcessor)
};
