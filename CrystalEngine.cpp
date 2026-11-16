#include "CrystalEngine.h"
#include <cmath>

void CrystalEngine::prepare (double sampleRate, int, int channels)
{
    sr = sampleRate;
    bufferLength = juce::jmax (4096, (int) std::ceil (sr * 3.0));
    delayBuffer.setSize (juce::jmax (2, channels), bufferLength);
    reset();
}

void CrystalEngine::reset()
{
    delayBuffer.clear();
    writePosition = 0;
    phase = 0.0;
}

void CrystalEngine::setParameters (float mix, float feedback, float pitchSemitones,
                                   float grainMs, float spread, bool reverse)
{
    wet = juce::jlimit (0.0f, 1.0f, mix);
    fb = juce::jlimit (0.0f, 0.92f, feedback);
    pitch = juce::jlimit (-24.0f, 24.0f, pitchSemitones);
    grain = juce::jlimit (25.0f, 500.0f, grainMs);
    stereoSpread = juce::jlimit (0.0f, 1.0f, spread);
    reverseGrains = reverse;
}

float CrystalEngine::readLinear (int channel, double position) const
{
    const int c = juce::jlimit (0, delayBuffer.getNumChannels() - 1, channel);
    while (position < 0.0) position += bufferLength;
    while (position >= bufferLength) position -= bufferLength;
    const int i0 = (int) position;
    const int i1 = (i0 + 1) % bufferLength;
    const float frac = (float) (position - i0);
    return delayBuffer.getSample (c, i0) * (1.0f - frac)
         + delayBuffer.getSample (c, i1) * frac;
}

void CrystalEngine::writeSample (int channel, float value)
{
    const int c = juce::jlimit (0, delayBuffer.getNumChannels() - 1, channel);
    delayBuffer.setSample (c, writePosition, value);
}

void CrystalEngine::process (juce::AudioBuffer<float>& buffer)
{
    if (bufferLength == 0 || wet <= 0.0001f) return;

    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    const double grainSamples = juce::jmax (1.0, sr * (double) grain * 0.001);
    const double ratio = std::pow (2.0, (double) pitch / 12.0);
    const double phaseIncrement = 1.0 / grainSamples;

    for (int n = 0; n < samples; ++n)
    {
        const float inL = buffer.getSample (0, n);
        const float inR = channels > 1 ? buffer.getSample (1, n) : inL;
        float wetL = 0.0f, wetR = 0.0f;

        // Two overlapping Hann-windowed read heads create a continuous grain stream.
        for (int head = 0; head < 2; ++head)
        {
            double p = phase + head * 0.5;
            p -= std::floor (p);
            const double window = 0.5 - 0.5 * std::cos (juce::MathConstants<double>::twoPi * p);
            const double travel = (reverseGrains ? -1.0 : 1.0) * p * grainSamples * (ratio - 1.0);
            const double baseDelay = grainSamples * 1.15;
            const double offsetL = baseDelay + travel;
            const double offsetR = baseDelay + travel + stereoSpread * grainSamples * 0.35;
            wetL += (float) window * readLinear (0, (double) writePosition - offsetL);
            wetR += (float) window * readLinear (channels > 1 ? 1 : 0, (double) writePosition - offsetR);
        }

        wetL *= 0.5f;
        wetR *= 0.5f;
        writeSample (0, inL + wetL * fb);
        if (delayBuffer.getNumChannels() > 1)
            writeSample (1, inR + wetR * fb);

        buffer.setSample (0, n, inL * (1.0f - wet) + wetL * wet);
        if (channels > 1)
            buffer.setSample (1, n, inR * (1.0f - wet) + wetR * wet);

        writePosition = (writePosition + 1) % bufferLength;
        phase += phaseIncrement;
        if (phase >= 1.0) phase -= 1.0;
    }
}
