#pragma once
#include <JuceHeader.h>

// Original granular pitch-delay inspired by crystal-like pitch echoes.
// This is not code from Soundtoys Crystallizer.
class CrystalEngine
{
public:
    void prepare (double sampleRate, int maximumBlockSize, int channels);
    void reset();
    void setParameters (float mix, float feedback, float pitchSemitones,
                        float grainMs, float spread, bool reverse);
    void process (juce::AudioBuffer<float>& buffer);

private:
    float readLinear (int channel, double position) const;
    void writeSample (int channel, float value);

    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
    int bufferLength = 0;
    double sr = 44100.0;
    float wet = 0.35f, fb = 0.25f, pitch = 12.0f, grain = 120.0f, stereoSpread = 0.35f;
    bool reverseGrains = false;
    double phase = 0.0;
};
