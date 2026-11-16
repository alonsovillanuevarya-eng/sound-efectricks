#include "PluginEditor.h"

SoundEfectricksAudioProcessorEditor::SoundEfectricksAudioProcessorEditor (SoundEfectricksAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (1120, 690);
    title.setText ("SOUND EFECTRICKS", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (30.0f, juce::Font::bold)));
    title.setColour (juce::Label::textColourId, juce::Colour (0xffe8f4ff));
    addAndMakeVisible (title);
    subtitle.setText ("SOUND ERA  /  CREATIVE STEP SEQUENCER", juce::dontSendNotification);
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xff7f9aaf));
    subtitle.setFont (juce::Font (juce::FontOptions (11.0f)));
    addAndMakeVisible (subtitle);
    crystalTitle.setText ("CRYSTAL ENGINE", juce::dontSendNotification);
    crystalTitle.setColour (juce::Label::textColourId, juce::Colour (0xff72e5dc));
    crystalTitle.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
    addAndMakeVisible (crystalTitle);

    auto addSlider = [this] (juce::Slider& s, juce::Label& l, const juce::String& name,
                             const juce::String& id)
    {
        configureSlider (s, l, name);
        attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                               (processor.parameters, id, s));
    };
    addSlider (mix, mixLabel, "MIX", "mix");
    addSlider (feedback, feedbackLabel, "FEEDBACK", "feedback");
    addSlider (pitch, pitchLabel, "PITCH", "pitch");
    addSlider (grain, grainLabel, "GRAIN", "grain");
    addSlider (spread, spreadLabel, "SPREAD", "spread");
    addSlider (cutoff, cutoffLabel, "FILTER", "filterCutoff");
    addSlider (crush, crushLabel, "CRUSH", "crush");
    addSlider (stutter, stutterLabel, "STUTTER", "stutter");
    addSlider (master, masterLabel, "OUTPUT", "master");

    for (int r = 0; r < 8; ++r)
        for (int s = 0; s < 32; ++s)
        {
            auto* b = cells.add (new juce::TextButton());
            b->setClickingTogglesState (true);
            b->setToggleState (processor.isStepEnabled (r, s), juce::dontSendNotification);
            b->onClick = [this, r, s, b]
            {
                processor.setStepEnabled (r, s, b->getToggleState());
            };
            addAndMakeVisible (b);
        }
    startTimerHz (15);
}

void SoundEfectricksAudioProcessorEditor::configureSlider (juce::Slider& s, juce::Label& l,
                                                             const juce::String& name)
{
    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, juce::Colour (0xff9eb7c8));
    l.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    addAndMakeVisible (l);
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff72e5dc));
    s.setColour (juce::Slider::thumbColourId, juce::Colour (0xffe8f4ff));
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffdceaf2));
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff17232e));
    addAndMakeVisible (s);
}

void SoundEfectricksAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b1118));
    g.setColour (juce::Colour (0xff142330));
    g.fillRoundedRectangle (18.0f, 92.0f, 1084.0f, 385.0f, 12.0f);
    g.fillRoundedRectangle (18.0f, 493.0f, 1084.0f, 178.0f, 12.0f);
    g.setColour (juce::Colour (0xff223746));
    g.drawRoundedRectangle (18.0f, 92.0f, 1084.0f, 385.0f, 12.0f, 1.0f);
    g.drawRoundedRectangle (18.0f, 493.0f, 1084.0f, 178.0f, 12.0f, 1.0f);
    g.setColour (juce::Colour (0xff72e5dc));
    g.drawLine (28.0f, 82.0f, 270.0f, 82.0f, 2.0f);
    g.setColour (juce::Colour (0xff607d90));
    g.setFont (11.0f);
    g.drawText ("32-STEP PATTERN  /  1/16 GRID", 34, 103, 300, 18, juce::Justification::left);
    g.drawText ("EFFECT LANES", 34, 129, 110, 20, juce::Justification::left);
    for (int r = 0; r < 8; ++r)
        g.drawText (rowNames[(size_t) r], 34, 151 + r * 41, 112, 29, juce::Justification::centredLeft);
    g.drawText ("CRYSTAL: GRANULAR PITCH DELAY", 36, 505, 360, 20, juce::Justification::left);
}

void SoundEfectricksAudioProcessorEditor::resized()
{
    title.setBounds (28, 18, 440, 42);
    subtitle.setBounds (31, 59, 420, 20);
    const int gridX = 158, gridY = 151, cellW = 27, cellH = 29, gap = 4, rowGap = 12;
    for (int r = 0; r < 8; ++r)
    {
        for (int s = 0; s < 32; ++s)
        {
            auto* b = cells[r * 32 + s];
            b->setBounds (gridX + s * (cellW + gap), gridY + r * (cellH + rowGap), cellW, cellH);
            b->setColour (juce::TextButton::buttonColourId,
                          ((s % 4) == 0) ? juce::Colour (0xff263f4c) : juce::Colour (0xff1a2a35));
            b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff72e5dc));
            b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff7894a5));
            b->setColour (juce::TextButton::textColourOnId, juce::Colour (0xff071217));
            b->setButtonText ({});
        }
    }
    // Row labels are drawn in paint; use compact label region by placing labels over the left grid.
    for (int i = 0; i < 9; ++i) {}
    const int knobY = 535, knobW = 100, knobH = 112, startX = 35;
    juce::Slider* sliders[] = { &mix, &feedback, &pitch, &grain, &spread, &cutoff, &crush, &stutter, &master };
    juce::Label* labels[] = { &mixLabel, &feedbackLabel, &pitchLabel, &grainLabel, &spreadLabel,
                              &cutoffLabel, &crushLabel, &stutterLabel, &masterLabel };
    for (int i = 0; i < 9; ++i)
    {
        const int x = startX + i * 116;
        labels[i]->setBounds (x, knobY - 8, knobW, 18);
        sliders[i]->setBounds (x, knobY + 8, knobW, knobH);
    }
    crystalTitle.setBounds (850, 24, 240, 30);
}

void SoundEfectricksAudioProcessorEditor::timerCallback()
{
    const int step = processor.getCurrentStep();
    for (int r = 0; r < 8; ++r)
        for (int s = 0; s < 32; ++s)
        {
            auto* b = cells[r * 32 + s];
            b->setColour (juce::TextButton::buttonOnColourId,
                          s == step ? juce::Colour (0xffffb45b) : juce::Colour (0xff72e5dc));
        }
    repaint();
}
