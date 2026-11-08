#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    using APVTS = juce::AudioProcessorValueTreeState;
    std::unique_ptr<APVTS::RangedAudioParameter> makeFloat(const char* id, const char* name,
        float min, float max, float def, float step = 0.01f)
    {
        return std::make_unique<juce::AudioParameterFloat>(id, name,
            juce::NormalisableRange<float>(min, max, step), def);
    }
}

DJOGuitarStudioAudioProcessor::DJOGuitarStudioAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                   .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
   parameters(*this, nullptr, "PARAMETERS", createParameters())
{
}

APVTS::ParameterLayout DJOGuitarStudioAudioProcessor::createParameters()
{
    APVTS::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterChoice>("preset", "Preset",
        juce::StringArray { "End of Beginning", "Basic Being Basic" }, 0));
    p.add(makeFloat("drive", "Drive", 0.0f, 1.0f, 0.22f));
    p.add(makeFloat("tone", "Tone", 0.0f, 1.0f, 0.58f));
    p.add(makeFloat("mod", "Modulation", 0.0f, 1.0f, 0.35f));
    p.add(makeFloat("rate", "Mod Rate", 0.05f, 5.0f, 0.35f, 0.01f));
    p.add(makeFloat("delay", "Delay", 0.0f, 1.0f, 0.12f));
    p.add(makeFloat("reverb", "Reverb", 0.0f, 1.0f, 0.28f));
    p.add(makeFloat("mix", "Mix", 0.0f, 1.0f, 0.75f));
    p.add(makeFloat("output", "Output", -18.0f, 6.0f, 0.0f, 0.1f));
    return p;
}

bool DJOGuitarStudioAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet();
    const auto out = l.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void DJOGuitarStudioAudioProcessor::prepareToPlay(double sr, int block)
{
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, (juce::uint32) getTotalNumOutputChannels() };
    lowCutL.reset(); lowCutR.reset(); highCutL.reset(); highCutR.reset();
    lowCutL.prepare(spec); lowCutR.prepare(spec); highCutL.prepare(spec); highCutR.prepare(spec);
    chorus.prepare(spec); phaser.prepare(spec);
    chorus.setCentreDelay(7.0f); chorus.setFeedback(0.12f); chorus.setDepth(0.22f);
    chorus.setRate(0.35f); chorus.setMix(0.35f);
    phaser.setRate(0.18f); phaser.setDepth(0.35f); phaser.setFeedback(0.15f); phaser.setMix(0.2f);
    delayL.prepare(spec); delayR.prepare(spec);
    delayL.setMaximumDelayInSamples((int) (sr * 1.5)); delayR.setMaximumDelayInSamples((int) (sr * 1.5));
    delayL.setDelay(sr * 0.28); delayR.setDelay(sr * 0.34);
    reverb.setParameters({ 0.35f, 0.55f, 0.32f, 0.25f, 1.0f, 0.0f });
    wetBuffer.setSize(getTotalNumOutputChannels(), block);
}

void DJOGuitarStudioAudioProcessor::setCurrentProgram(int index)
{
    currentProgram = juce::jlimit(0, 1, index);
    applyPreset(currentProgram);
    if (auto* v = parameters.getParameter("preset")) v->setValueNotifyingHost(v->convertTo0to1((float) currentProgram));
}

const juce::String DJOGuitarStudioAudioProcessor::getProgramName(int i)
{
    return i == 0 ? "End of Beginning" : "Basic Being Basic";
}

void DJOGuitarStudioAudioProcessor::applyPreset(int i)
{
    auto set = [this](const char* id, float value) {
        if (auto* p = parameters.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    if (i == 0) { // wide, dreamy, clean-ish and modulated
        set("drive", 0.16f); set("tone", 0.52f); set("mod", 0.62f);
        set("rate", 0.28f); set("delay", 0.22f); set("reverb", 0.48f); set("mix", 0.82f);
    } else { // tighter, brighter, more forward indie-rock rhythm
        set("drive", 0.38f); set("tone", 0.68f); set("mod", 0.22f);
        set("rate", 0.55f); set("delay", 0.08f); set("reverb", 0.20f); set("mix", 0.88f);
    }
}

void DJOGuitarStudioAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumOutputChannels();
    const int samples = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < channels; ++c) buffer.clear(c, 0, samples);

    const float drive = *parameters.getRawParameterValue("drive");
    const float tone = *parameters.getRawParameterValue("tone");
    const float mod = *parameters.getRawParameterValue("mod");
    const float rate = *parameters.getRawParameterValue("rate");
    const float delayAmt = *parameters.getRawParameterValue("delay");
    const float revAmt = *parameters.getRawParameterValue("reverb");
    const float mix = *parameters.getRawParameterValue("mix");
    const float out = juce::Decibels::decibelsToGain(*parameters.getRawParameterValue("output"));

    wetBuffer.makeCopyOf(buffer, true);
    const float sr = (float) getSampleRate();
    auto low = juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 75.0f);
    auto high = juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, 4200.0f + tone * 11500.0f);
    *lowCutL.coefficients = *low; *lowCutR.coefficients = *low;
    *highCutL.coefficients = *high; *highCutR.coefficients = *high;

    for (int c = 0; c < channels; ++c)
    {
        auto* x = wetBuffer.getWritePointer(c);
        for (int n = 0; n < samples; ++n)
        {
            const float gain = 1.0f + drive * 8.0f;
            const float driven = std::tanh(x[n] * gain) / std::tanh(gain);
            x[n] = x[n] * (1.0f - drive * 0.45f) + driven * (drive * 0.45f);
        }
        juce::dsp::AudioBlock<float> block(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block.getSingleChannelBlock((size_t)c));
        if (c == 0) { lowCutL.process(ctx); highCutL.process(ctx); }
        else { lowCutR.process(ctx); highCutR.process(ctx); }
    }

    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    if (channels > 0)
    {
        juce::dsp::ProcessContextReplacing<float> ctx(wetBlock);
        chorus.setRate(rate); chorus.setDepth(0.08f + mod * 0.62f); chorus.setMix(mod * 0.48f);
        chorus.process(ctx);
        phaser.setRate(0.08f + rate * 0.45f); phaser.setDepth(mod * 0.5f); phaser.setMix(mod * 0.18f);
        phaser.process(ctx);
    }

    // Stereo ping-pong-ish delay. Keep feedback low to avoid runaway buildup.
    for (int n = 0; n < samples; ++n)
    {
        const float l = wetBuffer.getNumChannels() > 0 ? wetBuffer.getSample(0,n) : 0.0f;
        const float r = wetBuffer.getNumChannels() > 1 ? wetBuffer.getSample(1,n) : l;
        const float dl = delayL.popSample(0);
        const float dr = delayR.popSample(0);
        delayL.pushSample(0, l + dr * 0.22f);
        delayR.pushSample(0, r + dl * 0.22f);
        if (channels > 0) wetBuffer.addSample(0,n, dl * delayAmt * 0.55f);
        if (channels > 1) wetBuffer.addSample(1,n, dr * delayAmt * 0.55f);
    }

    if (channels > 0)
    {
        juce::dsp::AudioBlock<float> rb(wetBuffer);
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = 0.25f + revAmt * 0.65f; rp.damping = 0.55f;
        rp.wetLevel = revAmt * 0.45f; rp.dryLevel = 0.0f; rp.width = 1.0f;
        reverb.setParameters(rp);
        reverb.processStereo(wetBuffer.getWritePointer(0),
            channels > 1 ? wetBuffer.getWritePointer(1) : wetBuffer.getWritePointer(0), samples);
    }

    for (int c = 0; c < channels; ++c)
        for (int n = 0; n < samples; ++n)
            buffer.setSample(c,n, out * (buffer.getSample(c,n) * (1.0f - mix) + wetBuffer.getSample(c,n) * mix));
}

juce::AudioProcessorEditor* DJOGuitarStudioAudioProcessor::createEditor()
{ return new DJOGuitarStudioAudioProcessorEditor(*this); }

void DJOGuitarStudioAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void DJOGuitarStudioAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{ return new DJOGuitarStudioAudioProcessor(); }
