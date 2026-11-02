#include "PluginProcessor.h"
#include "PluginEditor.h"

RetricaAudioProcessorEditor::RetricaAudioProcessorEditor (RetricaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);

    auto setupKnob = [this](juce::Slider& s, const juce::String& paramId)
    {
        s.setSliderStyle(juce::s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
        addAndMakeVisible(s);
        sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, s));
    };

    setupKnob(inputLSlider, "INPUT_L");
    setupKnob(makeUpLSlider, "MAKEUP_L");
    setupKnob(threshLSlider, "THRESH_L");
    setupKnob(attLSlider, "ATTACK_L");
    setupKnob(relLSlider, "RELEASE_L");
    setupKnob(kneeLSlider, "KNEE_L");

    setupKnob(inputRSlider, "INPUT_R");
    setupKnob(makeUpRSlider, "MAKEUP_R");
    setupKnob(threshRSlider, "THRESH_R");
    setupKnob(attRSlider, "ATTACK_R");
    setupKnob(relRSlider, "RELEASE_R");
    setupKnob(kneeRSlider, "KNEE_R");

    setupKnob(driveSlider, "DRIVE");
    setupKnob(mixSlider, "MIX");
    setupKnob(outputSlider, "OUTPUT");

    startTimerHz(30);
}

RetricaAudioProcessorEditor::~RetricaAudioProcessorEditor()
{
    stopTimer();
}

void Retrica{
    currentGrL = audioProcessor.getGainReductionL();
    currentGrR = audioProcessor.getGainReductionR();
    repaint();
}

void RetricaAudio(juce::Graphics& g)juce::Colour(0xff38277d), 0, 0, juce::Colour(0xff1a1040), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    g.setColour(juce::Colour(0xff120d29).withAlpha(0.7f));
    g.fillRect(0, 0, getWidth(), 36);

    g.setColour(juce::Colg.setFont(juce::Font(22.0f, juce::| juce::Font::bold));4, 0, 200, 36, juce::Justification::centredLeft);

    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffbeb7ea));
    g.drawText("VCA TRANSFORMER COMPRESSOR", 140, 0, 300, 36, juce::Justification::centredLeft);

    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff554892));
    g.drawText("L", 24, 90, 30, 40, juce::Justification::centred);
    g.drawText("R", 24, 260, 30, 40, juce::Justification::centred);

    auto vuBoundsL = juce::Rectangle<int>(560, 60, 180, 100);
    auto vuBoundsR = juce::Rectangle<int>(560, 180, 180, 100);

    auto drawVu = [&](juce::Rectangle<int> b, float grVal, const juce::String& title)
    {
        g.setColour(juce::Colour(0xff0c0919));
        g.fillRoundedRectangle(b.toFloat(), 6.0f);
        g.setColour(g.fillRoundedRectangle(b.reduced(4).toFloat(), 4.0f);

        g.setColour(juce::Colours::black);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText(title + " - VU GR", b.getX(), b.getY() + 8, b.getWidth(), 14, juce::Justification::centred);

        float needleAngle = -0.6f + std::min(grVal / 20.0f, 1.0f) * 1.2f;
        float pivotX = (float)b.getCentreX();
        float pivotY = (float)b.getBottom() - 8.0f;
        float len = 60.0f;
        float tipX = pivotX + std::sin(needleAngle) * len;
        float tipY = pivotY - std::cos(needleAngle) * len;

        g.setColour(juce::Colours::darkred);
        g.drawLine(pivotX, pivotY, tipX, tipY, 2.0f);
    };

    drawVu(vuBoundsL, currentGrL, "L");
    drawVu(vuBoundsR, currentGrR, "R");
}

void RetricaAudioProcessorEditor::resized()
{
    int startY = 70;
    int knobSize = 60;
    int spacing = 68;

    inputLSlider.setBounds(60, startY, knobSize, knobSize + 16);
    makeUpLSlider.setBounds(60 + spacing, startY, knobSize, knobSize + 16);
    threshLSlider.setBounds(60 + spacing * 2, startY, knobSize, knobSize + 16);
    attLSlider.setBounds(70 + spacing * 3, startY, knobSize, knobSize + 16);
    relLSlider.setBounds(70 + spacing * 4, startY, knobSize, knobSize + 16);
    kneeLSlider.setBounds(70 + spacing * 5, startY, knobSize, knobSize + 16);

    startY = 240;
    inputRSlider.setBounds(60, startY, knobSize, knobSize + 16);
    makeUpRSlider.setBounds(60 + spacing, startY, knobSize, knobSize + 16);
    threshRSlider.setBounds(60 + spacing * 2, startY, knobSize, knobSize + 16);
    attRSlider.setBounds(70 + spacing * 3, startY, knobSize, knobSize + 16);
    relRSlider.setBounds(70 + spacing * 4, startY, knobSize, knobSize + 16);
    kneeRSlider.setBounds(70 + spacing * 5, startY, knobSize, knobSize + 16);

    driveSlider.setBounds(100, 440, 70, 85);
    mixSlider.setBounds(200, 440, 70, 85);
    outputSlider.setBounds(300, 440, 70, 85);
}
