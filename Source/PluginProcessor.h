#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class RetricaAudioProcessor : public juce::AudioProcessor
{
RetricaAudioProcessor();
    ~RetricaAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Retrica VCA Compressor"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (destData) override;InBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    float getGainReductionL() const { return grL.load(std::memory_order_relaxed); }
    float getGainReductionR() const { return grR.load(std::memory_order_relaxed); }

private:
    float processVcaCore(float input, float sidechain, float thresh, float ratio, float attMs, float relMs, float knee, float& grEnv, float& rmsState, float& peakState, bool progRel);
    float applyTransformer(float x, float drive, float analog);

    double currentSampleRate = 44100.0;

    juce::dsp::IIR::Filter<float> scFilterL;
    juce::dsp::scFilterR;R = 0.0f;
    float peakL = 0.0f, peakR = 0.0f;
    float grEnvL = 0.0f, grEnvR = 0.0f;

    float dcX1_L = 0.0f, dcY1_L = 0.0f;
    float dcX1_R = 0.0f, dcY1_R = 0.0f;

    std::atomic<float> grL { 0.0f };
    std::atomic<float> grR { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RetricaAudioProcessor)
};
