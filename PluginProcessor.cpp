#include "PluginProcessor.h"
#include "PluginEditor.h"

SoundEfectricksAudioProcessor::SoundEfectricksAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (auto& row : pattern)
        for (auto& step : row) step.store (false);

    // Factory pattern: a few musical events are enabled on the first four rows.
    for (int s : { 0, 8, 16, 24 }) pattern[0][s].store (true);
    for (int s : { 4, 12, 20, 28 }) pattern[1][s].store (true);
    for (int s : { 7, 15, 23, 31 }) pattern[2][s].store (true);
}

juce::AudioProcessorValueTreeState::ParameterLayout SoundEfectricksAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APB = juce::AudioParameterBool;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add (std::make_unique<APF> ("master", "Master", 0.0f, 1.0f, 0.85f));
    p.add (std::make_unique<APF> ("mix", "Crystal Mix", 0.0f, 1.0f, 0.35f));
    p.add (std::make_unique<APF> ("feedback", "Crystal Feedback", 0.0f, 0.92f, 0.28f));
    p.add (std::make_unique<APF> ("pitch", "Crystal Pitch", -24.0f, 24.0f, 12.0f));
    p.add (std::make_unique<APF> ("grain", "Grain Size", 25.0f, 500.0f, 120.0f));
    p.add (std::make_unique<APF> ("spread", "Stereo Spread", 0.0f, 1.0f, 0.4f));
    p.add (std::make_unique<APB> ("reverse", "Reverse Grains", false));
    p.add (std::make_unique<APF> ("filterCutoff", "Filter Cutoff", 80.0f, 18000.0f, 12000.0f));
    p.add (std::make_unique<APF> ("crush", "Crush", 0.0f, 1.0f, 0.0f));
    p.add (std::make_unique<APF> ("stutter", "Stutter", 0.0f, 1.0f, 0.0f));
    p.add (std::make_unique<APF> ("reverb", "Reverb", 0.0f, 1.0f, 0.12f));
    return p;
}

void SoundEfectricksAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    crystal.prepare (sr, samplesPerBlock, 2);
    filterL.reset(); filterR.reset();
    filterL.prepare ({ sr, (juce::uint32) samplesPerBlock, 1 });
    filterR.prepare ({ sr, (juce::uint32) samplesPerBlock, 1 });
    filterL.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filterR.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    phaser.prepare ({ sr, (juce::uint32) samplesPerBlock, 2 });
    phaser.setRate (0.18f);
    phaser.setDepth (0.55f);
    phaser.setCentreFrequency (900.0f);
    phaser.setFeedback (0.25f);
    phaser.setMix (0.35f);
    reverb.reset();
    scratch.setSize (2, samplesPerBlock);
    fallbackPpq = 0.0;
}

void SoundEfectricksAudioProcessor::releaseResources() {}

bool SoundEfectricksAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

bool SoundEfectricksAudioProcessor::isStepEnabled (int row, int step) const
{
    if (row < 0 || row >= rows || step < 0 || step >= steps) return false;
    return pattern[(size_t) row][(size_t) step].load();
}

void SoundEfectricksAudioProcessor::setStepEnabled (int row, int step, bool enabled)
{
    if (row >= 0 && row < rows && step >= 0 && step < steps)
        pattern[(size_t) row][(size_t) step].store (enabled);
}

void SoundEfectricksAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numIn = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    for (int ch = numIn; ch < numOut; ++ch) buffer.clear (ch, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() == 0) return;

    double bpm = 120.0, ppq = fallbackPpq;
    bool playing = false;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            if (auto q = pos->getPpqPosition()) ppq = *q;
            playing = pos->getIsPlaying();
        }
    }
    bpm = juce::jlimit (20.0, 300.0, bpm);
    const double quarterNotesPerStep = 0.25; // sixteenth-note grid
    const double stepLength = quarterNotesPerStep;
    const int step = ((int) std::floor (ppq / stepLength)) % steps;
    currentStep.store (juce::jlimit (0, steps - 1, step));

    // Determine active effects at the block's musical position.
    std::array<bool, rows> active {};
    for (int r = 0; r < rows; ++r) active[(size_t) r] = isStepEnabled (r, step);

    const float master = parameters.getRawParameterValue ("master")->load();
    const float cutoff = parameters.getRawParameterValue ("filterCutoff")->load();
    const float crush = parameters.getRawParameterValue ("crush")->load();
    const float stutter = parameters.getRawParameterValue ("stutter")->load();
    const float mix = parameters.getRawParameterValue ("mix")->load();
    const float feedback = parameters.getRawParameterValue ("feedback")->load();
    const float pitch = parameters.getRawParameterValue ("pitch")->load();
    const float grain = parameters.getRawParameterValue ("grain")->load();
    const float spread = parameters.getRawParameterValue ("spread")->load();
    const bool reverse = parameters.getRawParameterValue ("reverse")->load() > 0.5f;
    const float rev = parameters.getRawParameterValue ("reverb")->load();

    filterL.setCutoffFrequency (cutoff);
    filterR.setCutoffFrequency (cutoff);

    // Row mapping: STUTTER, LOOP, REVERSE, CRUSH, FILTER, PHASER, SPACE, CRYSTAL.
    if (active[4])
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        if (buffer.getNumChannels() > 1)
        {
            auto left = block.getSingleChannelBlock (0);
            auto right = block.getSingleChannelBlock (1);
            juce::dsp::ProcessContextReplacing<float> cL (left), cR (right);
            filterL.process (cL); filterR.process (cR);
        }
        else
        {
            auto mono = block.getSingleChannelBlock (0);
            juce::dsp::ProcessContextReplacing<float> c (mono);
            filterL.process (c);
        }
    }

    if (active[5] && buffer.getNumChannels() > 1)
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        phaser.process (context);
    }

    if (active[7])
    {
        crystal.setParameters (mix, feedback, pitch, grain, spread, reverse || active[2]);
        crystal.process (buffer);
    }

    if (active[3] && crush > 0.001f)
    {
        const int bits = juce::jlimit (4, 16, (int) std::round (16.0f - crush * 12.0f));
        const float levels = (float) (1 << bits);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                const float x = buffer.getSample (ch, n);
                buffer.setSample (ch, n, std::round (x * levels) / levels);
            }
    }

    if (active[6] && rev > 0.001f)
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = 0.72f; rp.damping = 0.35f; rp.wetLevel = rev;
        rp.dryLevel = 1.0f - rev * 0.5f; rp.width = 1.0f; rp.freezeMode = 0.0f;
        reverb.setParameters (rp);
        reverb.processStereo (buffer.getWritePointer (0),
                              buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : buffer.getWritePointer (0),
                              buffer.getNumSamples());
    }

    if (active[0] && stutter > 0.001f)
    {
        // Short periodic amplitude gates; rate follows a 1/16-step pulse.
        const int period = juce::jmax (1, (int) (sr * 60.0 / bpm / 4.0));
        const int gate = juce::jmax (1, (int) (period * (1.0f - stutter * 0.85f)));
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                if ((n % period) >= gate) buffer.setSample (ch, n, 0.0f);
    }

    // Reverse/loop rows are sequencer lanes reserved for future buffer-based modes.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain (ch, 0, buffer.getNumSamples(), master);

    if (! playing)
        fallbackPpq += (double) buffer.getNumSamples() / sr * bpm / 60.0;
}

void SoundEfectricksAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    juce::MemoryOutputStream stream (destData, false);
    state.writeToStream (stream);
    for (int r = 0; r < rows; ++r)
        for (int s = 0; s < steps; ++s)
            stream.writeBool (isStepEnabled (r, s));
}

void SoundEfectricksAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, (size_t) sizeInBytes, false);
    auto state = juce::ValueTree::readFromStream (stream);
    if (state.isValid()) parameters.replaceState (state);
    for (int r = 0; r < rows; ++r)
        for (int s = 0; s < steps; ++s)
            if (! stream.isExhausted()) setStepEnabled (r, s, stream.readBool());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoundEfectricksAudioProcessor();
}
