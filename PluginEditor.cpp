#include "PluginEditor.h"

DJOGuitarStudioAudioProcessorEditor::DJOGuitarStudioAudioProcessorEditor(DJOGuitarStudioAudioProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    setSize(620, 390);
    title.setText("DJO GUITAR STUDIO", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xfff4e8d0));
    addAndMakeVisible(title);
    subtitle.setText("DREAMY / PSYCHEDELIC INDIE GUITAR", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffb5a98f));
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(subtitle);

    preset.addItem("End of Beginning", 1);
    preset.addItem("Basic Being Basic", 2);
    addAndMakeVisible(preset);
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, "preset", preset);
    preset.onChange = [this] { if (preset.getSelectedId() > 0) processor.applyPreset(preset.getSelectedId() - 1); };

    setupSlider(drive, "DRIVE", "drive"); setupSlider(tone, "TONE", "tone");
    setupSlider(mod, "MOD", "mod"); setupSlider(rate, "RATE", "rate");
    setupSlider(delay, "DELAY", "delay"); setupSlider(reverb, "REVERB", "reverb");
    setupSlider(mix, "MIX", "mix"); setupSlider(output, "OUTPUT", "output");
}

void DJOGuitarStudioAudioProcessorEditor::setupSlider(juce::Slider& s, const juce::String& label, const juce::String& id)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 20);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd59b54));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffdca1));
    addAndMakeVisible(s);
    auto* l = new juce::Label();
    l->setText(label, juce::dontSendNotification);
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::textColourId, juce::Colour(0xffe9dcc2));
    addAndMakeVisible(l);
    s.setName(label);
    s.getProperties().set("label", label);
    attachments.push_back(std::make_unique<Attachment>(processor.parameters, id, s));
}

void DJOGuitarStudioAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff171a1b));
    g.setColour(juce::Colour(0xff303638)); g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(12), 16.0f);
    g.setColour(juce::Colour(0xffd59b54)); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(12), 16.0f, 1.5f);
    g.setColour(juce::Colour(0xffd59b54)); g.drawLine(28.0f, 86.0f, 592.0f, 86.0f, 1.0f);
    const juce::Slider* sliders[] = { &drive,&tone,&mod,&rate,&delay,&reverb,&mix,&output };
    for (auto* s : sliders)
    {
        auto b = s->getBounds().toFloat();
        g.setColour(juce::Colour(0xffe9dcc2));
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText(s->getName(), b.getX(), b.getY()-17, b.getWidth(), 16, juce::Justification::centred);
    }
}
void DJOGuitarStudioAudioProcessorEditor::resized()
{
    title.setBounds(28, 20, 330, 36); subtitle.setBounds(30, 54, 350, 20);
    preset.setBounds(390, 35, 200, 28);
    juce::Slider* s[] = { &drive,&tone,&mod,&rate,&delay,&reverb,&mix,&output };
    for (int i=0;i<8;++i) {
        const int col=i%4, row=i/4;
        s[i]->setBounds(28 + col*145, 125 + row*125, 112, 105);
    }
}
