#pragma once
#include <JuceHeader.h>

class ReverbDistortionAudioProcessor : public juce::AudioProcessor
{
public:
    ReverbDistortionAudioProcessor();
    ~ReverbDistortionAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Components
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    
    juce::dsp::Chorus<float> flanger; 
    juce::dsp::StateVariableTPTFilter<float> hpFilter;
    juce::dsp::StateVariableTPTFilter<float> cabFilter; // Simple amp cabinet sim
    
    juce::AudioBuffer<float> wetBuffer;

    // Smoothed Parameters
    juce::SmoothedValue<float> intensitySmoothed;
    juce::SmoothedValue<float> mixSmoothed;
    juce::SmoothedValue<float> ampSmoothed;

    // Tape Saturation with aggressive Inverse-Energy compensation
    float applySaturation (float x, float intensity) noexcept
    {
        if (intensity <= 0.001f) return x;
        
        const float drive = 1.0f + (intensity * 5.0f);
        const float x_driven = x * drive;
        
        // Soft-clip
        const float out = std::tanh(x_driven);
        
        // Compensate for the thickening effect of drive
        const float compensation = 1.0f / (1.0f + (intensity * 1.8f));
        return out * compensation;
    }

    // Aggressive Amp Saturation for extra bite
    float applyAmp (float x, float ampAmount) noexcept
    {
        if (ampAmount <= 0.001f) return x;
        
        const float drive = 1.0f + (ampAmount * 8.0f);
        float x_driven = x * drive;
        
        // Asymmetrical tube-like clipping
        float out = (x_driven > 0.0f) ? std::tanh(x_driven) : std::tanh(x_driven * 0.5f) * 2.0f;
        
        return out * 0.8f; // Native gain pull-back
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbDistortionAudioProcessor)
};
