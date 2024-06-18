/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XdelayAudioProcessorEditor::XdelayAudioProcessorEditor (XdelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Feedback slider
    addAndMakeVisible(feedbackSlider);
    feedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.avpts, "FEEDBACK", feedbackSlider);

    //Feedback label
    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider, false);
    feedbackLabel.setJustificationType(juce::Justification::centred);

    // Timing slider
    addAndMakeVisible(timingSlider);
    timingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    timingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    timingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.avpts, "TIMING", timingSlider);

    //Timing Label
    timingLabel.setText("Timing", juce::dontSendNotification);
    timingLabel.attachToComponent(&timingSlider, false);
    timingLabel.setJustificationType(juce::Justification::centred);

    //Mix slider
    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.avpts, "MIX", mixSlider);

    //Mix label
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider, false);
    mixLabel.setJustificationType(juce::Justification::centred);


	setSize (400, 300);
}

XdelayAudioProcessorEditor::~XdelayAudioProcessorEditor()
{
}

//==============================================================================
void XdelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    /*g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    
}

void XdelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    feedbackSlider.setBounds(100, 75, 100, 100);
    
    timingSlider.setBounds(200, 75, 100, 100);

    mixSlider.setBounds(150, 175, 100, 100);
    
}
