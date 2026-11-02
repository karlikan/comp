#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class RetricaAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    RetricaAudioProcessorEditor (RetricaAudioProcessor&);
    ~RetricaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    RetricaAudioProcessor& audioProcessor;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::Slider inputLSlider, makeUpLSlider, threshLSlider, attLSlider, relLSlider, kneeLSlider;
    juce::Slider inputRSlider, makeUpRSlider, threshRSlider, attRSlider, relRSlider, kneeRSlider;
    juce::Slider driveSlider, mixSlider, outputSlider;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;

    float currentGrL = 0.0f;
    float currentGrR = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RetricaAudioProcessorEditor)
};
