#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout
ReverbDistortionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "intensity", 1 }, "INTENSITY", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (juce::roundToInt (value * 100.0f)) + "%"; },
        [](const juce::String& text) { return text.getFloatValue() / 100.0f; }));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "amp", 1 }, "AMP", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (juce::roundToInt (value * 100.0f)) + "%"; },
        [](const juce::String& text) { return text.getFloatValue() / 100.0f; }));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 }, "MIX", 
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (juce::roundToInt (value * 100.0f)) + "%"; },
        [](const juce::String& text) { return text.getFloatValue() / 100.0f; }));

    return { params.begin(), params.end() };
}

ReverbDistortionAudioProcessor::ReverbDistortionAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

ReverbDistortionAudioProcessor::~ReverbDistortionAudioProcessor() {}

juce::AudioProcessorEditor* ReverbDistortionAudioProcessor::createEditor()
{
    return new ReverbDistortionAudioProcessorEditor (*this);
}

void ReverbDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = 2;

    reverb.prepare (spec);
    
    hpFilter.prepare (spec);
    hpFilter.setType (juce::dsp::StateVariableTPTFilterType::highpass);

    cabFilter.prepare (spec);
    cabFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    cabFilter.setResonance (0.7f);
    cabFilter.setCutoffFrequency (2500.0f); // Typical guitar cab mid-range focus
    
    flanger.prepare (spec);
    flanger.setRate (0.25f); 
    flanger.setCentreDelay (3.5f);
    flanger.setFeedback (0.3f);
    
    intensitySmoothed.reset (sampleRate, 0.02);
    ampSmoothed      .reset (sampleRate, 0.02);
    mixSmoothed      .reset (sampleRate, 0.02);

    wetBuffer.setSize (2, samplesPerBlock);
}

void ReverbDistortionAudioProcessor::releaseResources() {}

bool ReverbDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    if (layouts.getMainInputChannelSet()  != juce::AudioChannelSet::stereo()) return false;
    return true;
}

void ReverbDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    intensitySmoothed.setTargetValue (*apvts.getRawParameterValue ("intensity"));
    ampSmoothed      .setTargetValue (*apvts.getRawParameterValue ("amp"));
    mixSmoothed      .setTargetValue (*apvts.getRawParameterValue ("mix"));

    // Capture dry for global mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    // Initialise wet buffer
    wetBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
    if (numChannels > 1)
        wetBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);

    const float intensity = intensitySmoothed.getNextValue();
    const float ampAmount = ampSmoothed      .getNextValue();
    
    intensitySmoothed.skip (numSamples - 1);
    ampSmoothed      .skip (numSamples - 1);

    if (intensity > 0.0001f || ampAmount > 0.0001f)
    {
        juce::dsp::AudioBlock<float> wetBlock (wetBuffer);
        juce::dsp::ProcessContextReplacing<float> context (wetBlock);

        // 1. High Pass (Risir)
        float hpFreq = 20.0f + (intensity * intensity * 4000.0f); 
        hpFilter.setCutoffFrequency (hpFreq);
        hpFilter.process (context);

        // 2. Reverb
        reverbParams.roomSize = 0.85f + (intensity * 0.149f);
        reverbParams.damping  = 0.1f + (intensity * 0.7f);
        reverbParams.width    = 1.0f;
        reverbParams.wetLevel = intensity * 0.85f; 
        reverbParams.dryLevel = 1.0f - (intensity * 0.95f);
        reverb.setParameters (reverbParams);
        reverb.process (context);

        // 3. Flanger
        flanger.setDepth (intensity * 0.6f);
        flanger.setMix (intensity * 0.45f);
        flanger.process (context);

        // 4. Amp & Grit
        auto* wetL = wetBuffer.getWritePointer (0);
        auto* wetR = (numChannels > 1) ? wetBuffer.getWritePointer (1) : nullptr;

        for (int s = 0; s < numSamples; ++s)
        {
            // First apply the basic grit saturation linked to intensity
            wetL[s] = applySaturation (wetL[s], intensity);
            if (wetR) wetR[s] = applySaturation (wetR[s], intensity);

            // Then apply the dedicated AMP effect
            wetL[s] = applyAmp (wetL[s], ampAmount);
            if (wetR) wetR[s] = applyAmp (wetR[s], ampAmount);
        }

        // Apply "Cabinet" filtering if amp is active
        if (ampAmount > 0.01f)
        {
            cabFilter.process (context);
        }

        // Final Active Volume Leveling
        const float washoutCompensation = 1.0f / (1.0f + (intensity * 1.5f) + (ampAmount * 0.5f));
        wetBuffer.applyGain (washoutCompensation);
    }

    // 5. Global Mix
    for (int s = 0; s < numSamples; ++s)
    {
        const float m = mixSmoothed.getNextValue();
        const float dL = dryBuffer.getReadPointer (0)[s];
        const float wL = wetBuffer.getReadPointer (0)[s];
        
        buffer.getWritePointer (0)[s] = (1.0f - m) * dL + m * wL;

        if (numChannels > 1)
        {
            const float dR = dryBuffer.getReadPointer (1)[s];
            const float wR = wetBuffer.getReadPointer (1)[s];
            buffer.getWritePointer (1)[s] = (1.0f - m) * dR + m * wR;
        }
    }
}

void ReverbDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ReverbDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbDistortionAudioProcessor();
}
