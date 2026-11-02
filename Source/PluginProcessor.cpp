#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RetricaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterBool>("POWER", "Power", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("MODE", "Mode", juce::StringArray { "Stereo", "M/S", "L/R" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("RATIO", "Ratio", juce::StringArray { "2:1", "4:1", "6:1", "8:1", "12:1", "20:1" }, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT_L", "Input L", juce::NormalisableRange<float>(-60.0f, 12.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MAKEUP_L", "MakeUp L", juce::NormalisableRange<float>(-60.0f, 12.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("THRESH_L", "Threshold L", juce::NormalisableRange<float>(-60.0f, 0.0f), -30.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK_L", "Attack L", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.5f), 30.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE_L", "Release L", juce::NormalisableRange<float>(1.0f, 1000.0f, 0.1f, 0.5f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("KNEE_L", "Knee L", juce::NormalisableRange<float>(0.0f, 72.0f), 24.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT_R", "Input R", juce::NormalisableRange<float>(-60.0f, 12.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MAKEUP_R", "MakeUp R", juce::NormalisableRange<float>(-60.0f, 12.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("THRESH_R", "Threshold R", juce::NormalisableRange<float>(-60.0f, 0.0f), -30.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK_R", "Attack R", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.5f), 30.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE_R", "Release R", juce::NormalisableRange<float>(1.0f, 1000.0f, 0.1f, 0.5f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("KNEE_R", "Knee R", juce::NormalisableRange<float>(0.0f, 72.0f), 24.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", juce::NormalisableRange<float>(0.0f, 100.0f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ANALOG", "Analog", juce::NormalisableRange<float>(0.0f, 100.0f), 25.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SC_HPF", "SC HPF", juce::NormalisableRange<float>(20.0f, 400.0f, 1.0f, 0.4f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("PROG_REL", "Prog Release", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT", "Output", juce::NormalisableRange<float>(-24.0f, 12.0f), 0.0f));

    return { params.begin(), params.end() };
}

RetricaAudioProcessor::RetricaAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

void RetricaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    rmsL = rmsR = peakL = peakR = grEnvL = grEnvR = 0.0f;
    dcX1_L = dcY1_L = dcX1_R = dcY1_R = 0.0f;

    auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f, 0.707f);
    scFilterL.coefficients = hpfCoeffs;
    scFilterR.coefficients = hpfCoeffs;
    scFilterL.reset();
    scFilterR.reset();
}

void RetricaAudioProcessor::releaseResources() {}

float RetricaAudioProcessor::applyTransformer(float x, float drive, float analog)
{
    if (drive <= 0.001f && analog <= 0.001f) return x;

    const float d = 1.0f + drive * 2.5f;
    float in = x * d;
    const float asym = analog * 0.15f;
    float biased = in + asym * in * in;

    float sat = std::tanh(biased);
    float normalizer = 1.0f / (1.0f + drive * 0.85f);
    return sat * normalizer;
}

float RetricaAudioProcessor::processVcaCore(float input, float sidechain, float thresh, float ratio, float attMs, float relMs, float knee, float& grEnv, float& rmsState, float& peakState, bool progRel)
{
    float sq = sidechain * sidechain;
    float alphaRms = std::exp(-1000.0f / (30.0f * (float)currentSampleRate));
    rmsState = alphaRms * rmsState + (1.0f - alphaRms) * sq;

    float absSig = std::abs(sidechain);
    float alphaPeak = std::exp(-1000.0f / (1.5f * (float)currentSampleRate));
    peakState = std::max(absSig, peakState * alphaPeak);

    float hybrid = std::sqrt(std::max(rmsState, 1e-12f)) * 0.65f + peakState * 0.35f;
    float detDb = juce::Decibels::gainToDecibels(std::max(hybrid, 1e-6f));

    float targetGrDb = 0.0f;
    float overDb = detDb - thresh;
    float halfKnee = knee * 0.5f;

    if (knee > 0.1f)
    {
        if (overDb <= -halfKnee)
            targetGrDb = 0.0f;
        else if (overDb < halfKnee)
        {
            float t = (overDb + halfKnee) / knee;
            targetGrDb = (t * t * halfKnee) * (1.0f - 1.0f / ratio);
        }
        else
            targetGrDb = overDb * (1.0f - 1.0f / ratio);
    }
    else
    {
        targetGrDb = (overDb > 0.0f) ? (overDb * (1.0f - 1.0f / ratio)) : 0.0f;
    }
    targetGrDb = std::max(targetGrDb, 0.0f);

    float alphaAtt = std::exp(-1000.0f / (attMs * (float)currentSampleRate));
    float alphaRel = std::exp(-1000.0f / (relMs * (float)currentSampleRate));

    if (progRel && (grEnv - targetGrDb > 6.0f))
        alphaRel *= 0.985f;

    if (targetGrDb > grEnv)
        grEnv = (1.0f - alphaAtt) * targetGrDb + alphaAtt * grEnv;
    else
        grEnv = (1.0f - alphaRel) * targetGrDb + alphaRel * grEnv;

    float vcaGain = juce::Decibels::decibelsToGain(-grEnv);
    return input * vcaGain;
}

void RetricaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels < 2) return;

    if (!apvts.getRawParameterValue("POWER")->load())
        return;

    const int mode = (int)apvts.getRawParameterValue("MODE")->load();
    const int ratioChoice = (int)apvts.getRawParameterValue("RATIO")->load();
    const float ratios[] = { 2.0f, 4.0f, 6.0f, 8.0f, 12.0f, 20.0f };
    const float ratio = ratios[std::clamp(ratioChoice, 0, 5)];

    const float inGainL = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("INPUT_L")->load());
    const float inGainR = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("INPUT_R")->load());
    const float muGainL = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("MAKEUP_L")->load());
    const float muGainR = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("MAKEUP_R")->load());

    const float threshL = apvts.getRawParameterValue("THRESH_L")->load();
    const float threshR = apvts.getRawParameterValue("THRESH_R")->load();
    const float attL = apvts.getRawParameterValue("ATTACK_L")->load();
    const float attR = apvts.getRawParameterValue("ATTACK_R")->load();
    const float relL = apvts.getRawParameterValue("RELEASE_L")->load();
    const float relR = apvts.getRawParameterValue("RELEASE_R")->load();
    const float kneeL = apvts.getRawParameterValue("KNEE_L")->load();
    const float kneeR = apvts.getRawParameterValue("KNEE_R")->load();

    const float drive = apvts.getRawParameterValue("DRIVE")->load() * 0.01f;
    const float analog = apvts.getRawParameterValue("ANALOG")->load() * 0.01f;
    const float scHpf = apvts.getRawParameterValue("SC_HPF")->load();
    const bool progRel = apvts.getRawParameterValue("PROG_REL")->load() > 0.5f;
    const float mix = apvts.getRawParameterValue("MIX")->load() * 0.01f;
    const float outGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("OUTPUT")->load());

    auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, scHpf, 0.707f);
    scFilterL.coefficients = hpfCoeffs;
    scFilterR.coefficients = hpfCoeffs;

    auto* channelL = buffer.getWritePointer(0);
    auto* channelR = buffer.getWritePointer(1);

    float maxGrL = 0.0f, maxGrR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = channelL[i];
        float dryR = channelR[i];

        float sigL = dryL * inGainL;
        float sigR = dryR * inGainR;

        float ch1 = (mode == 1) ? (0.5f * (sigL + sigR)) : sigL;
        float ch2 = (mode == 1) ? (0.5f * (sigL - sigR)) : sigR;

        float sc1 = scFilterL.processSample(ch1);
        float sc2 = scFilterR.processSample(ch2);

        if (mode == 0)
        {
            float maxSc = std::max(std::abs(sc1), std::abs(sc2));
            sc1 = sc2 = maxSc;
        }

        float vca1 = processVcaCore(ch1, sc1, threshL, ratio, attL, relL, kneeL, grEnvL, rmsL, peakL, progRel);
        float vca2 = processVcaCore(ch2, sc2, (mode == 0 ? threshL : threshR), ratio, (mode == 0 ? attL : attR), (mode == 0 ? relL : relR), (mode == 0 ? kneeL : kneeR), grEnvR, rmsR, peakR, progRel);

        vca1 *= muGainL;
        vca2 *= (mode == 0 ? muGainL : muGainR);

        float outCompL = (mode == 1) ? (vca1 + vca2) : vca1;
        float outCompR = (mode == 1) ? (vca1 - vca2) : vca2;

        float trL = applyTransformer(outCompL, drive, analog);
        float trR = applyTransformer(outCompR, drive, analog);

        float dcL = trL - dcX1_L + 0.9985f * dcY1_L;
        dcX1_L = trL; dcY1_L = dcL; trL = dcL;

        float dcR = trR - dcX1_R + 0.9985f * dcY1_R;
        dcX1_R = trR; dcY1_R = dcR; trR = dcR;

        float wetL = trL * outGain;
        float wetR = trR * outGain;

        channelL[i] = (1.0f - mix) * dryL + mix * wetL;
        channelR[i] = (1.0f - mix) * dryR + mix * wetR;

        maxGrL = std::max(maxGrL, grEnvL);
        maxGrR = std::max(maxGrR, grEnvR);
    }

    grL.store(maxGrL, std::memory_order_relaxed);
    grR.store(maxGrR, std::memory_order_relaxed);
}

void RetricaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RetricaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<xmlState (getXmlFromBinary (data, sizeInBytes));State != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* RetricaAudioProcessor::createEditor()
{
    return new RetricaAudioProcessorEditor (*this);
}
