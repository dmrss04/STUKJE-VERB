#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Large high-resolution knob
class LargeKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;

    LargeKnob (const juce::String& labelText)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible (slider);

        label.setText (labelText, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (juce::FontOptions ("Helvetica", 14.0f, juce::Font::bold)));
        addAndMakeVisible (label);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label.setBounds (area.removeFromTop (25));
        slider.setBounds (area);
    }
};

// Smaller utility knob
class SmallKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  label;

    SmallKnob (const juce::String& labelText)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
        addAndMakeVisible (slider);

        label.setText (labelText, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (juce::FontOptions ("Helvetica", 11.0f, juce::Font::plain)));
        addAndMakeVisible (label);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label.setBounds (area.removeFromTop (18));
        slider.setBounds (area);
    }
};

class ReverbDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ReverbDistortionAudioProcessorEditor (ReverbDistortionAudioProcessor&);
    ~ReverbDistortionAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ReverbDistortionAudioProcessor& audioProcessor;

    LargeKnob intensityKnob { "INTENSITY" };
    LargeKnob mixKnob       { "MIX" };
    SmallKnob ampKnob       { "AMP" };

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    Attachment intensityAttach, mixAttach, ampAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbDistortionAudioProcessorEditor)
};
