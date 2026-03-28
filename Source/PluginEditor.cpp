#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace Colors
{
    const juce::Colour bg           { 0xff050505 };
    const juce::Colour accent       { 0xffffffff };
    const juce::Colour text         { 0xffe0e0e0 };
    const juce::Colour textMuted    { 0xff505050 };
    const juce::Colour border       { 0xff1a1a1a };
}

static void applyKnobStyle (juce::Slider& s)
{
    s.setColour (juce::Slider::rotarySliderFillColourId,   Colors::accent);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, Colors::border);
    s.setColour (juce::Slider::thumbColourId,               Colors::accent);
    s.setColour (juce::Slider::textBoxTextColourId,         Colors::text);
    s.setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
}

ReverbDistortionAudioProcessorEditor::ReverbDistortionAudioProcessorEditor (ReverbDistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      intensityAttach (p.apvts, "intensity", intensityKnob.slider),
      mixAttach (p.apvts, "mix", mixKnob.slider),
      ampAttach (p.apvts, "amp", ampKnob.slider)
{
    setSize (600, 320);
    
    applyKnobStyle (intensityKnob.slider);
    applyKnobStyle (mixKnob.slider);
    applyKnobStyle (ampKnob.slider);
    
    intensityKnob.label.setColour (juce::Label::textColourId, Colors::text);
    mixKnob.label.setColour (juce::Label::textColourId, Colors::text);
    ampKnob.label.setColour (juce::Label::textColourId, Colors::textMuted);

    addAndMakeVisible (intensityKnob);
    addAndMakeVisible (mixKnob);
    addAndMakeVisible (ampKnob);
}

ReverbDistortionAudioProcessorEditor::~ReverbDistortionAudioProcessorEditor() {}

void ReverbDistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colors::bg);

    auto bounds = getLocalBounds().toFloat();
    
    // Header
    auto header = bounds.removeFromTop (60).reduced (25, 0);
    g.setColour (Colors::accent);
    g.setFont (juce::Font (juce::FontOptions ("Helvetica", 22.0f, juce::Font::bold)));
    g.drawText ("STUKJE VERB", header, juce::Justification::centredLeft);

    // Subtle divider
    g.setColour (Colors::border);
    g.drawHorizontalLine (60, 25.0f, bounds.getWidth() - 25.0f);
    
    // Branding at bottom
    g.setColour (Colors::textMuted);
    g.setFont (juce::Font (juce::FontOptions ("Helvetica", 9.0f, juce::Font::plain)));
    g.drawText ("BY STUKJE  |  V1.0.2", bounds.removeFromBottom (30).reduced (25, 0), juce::Justification::centredLeft);
}

void ReverbDistortionAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (80);
    area.removeFromBottom (40);
    
    auto mainContent = area.reduced (40, 0);
    
    // Layout: Intensity | Amp (Small) | Mix
    int w = mainContent.getWidth();
    int largeW = (int)(w * 0.4f);
    int smallW = (int)(w * 0.2f);
    
    intensityKnob.setBounds (mainContent.removeFromLeft (largeW).reduced (10));
    ampKnob.setBounds (mainContent.removeFromLeft (smallW).reduced (5, 30));
    mixKnob.setBounds (mainContent.reduced (10));
}
