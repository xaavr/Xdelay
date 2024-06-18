/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class XdelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    XdelayAudioProcessorEditor (XdelayAudioProcessor&);
    ~XdelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    
    XdelayAudioProcessor& audioProcessor;
    juce::Slider feedbackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;

    juce::Slider timingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timingAttachment;

    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    juce::Label feedbackLabel;
    juce::Label timingLabel;
    juce::Label mixLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XdelayAudioProcessorEditor)
};
