#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DJOGuitarStudioAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DJOGuitarStudioAudioProcessorEditor(DJOGuitarStudioAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    DJOGuitarStudioAudioProcessor& processor;
    juce::ComboBox preset;
    juce::Slider drive, tone, mod, rate, delay, reverb, mix, output;
    juce::Label title, subtitle;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<Attachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;
    void setupSlider(juce::Slider&, const juce::String&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJOGuitarStudioAudioProcessorEditor)
};
