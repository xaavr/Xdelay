/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_data_structures/juce_data_structures.h>

//==============================================================================
/**
*/
class XdelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    XdelayAudioProcessor();
    ~XdelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void writeToDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer);
    void XdelayAudioProcessor::readToDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer);
    void XdelayAudioProcessor::writeFeedbackToDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState avpts;
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::LinearSmoothedValue<float> feedback{ 0.0f };
    juce::LinearSmoothedValue<float> delayTime{ 1.0f };
    juce::LinearSmoothedValue<float> mix{ 1.0f };

	juce::AudioBuffer<float> delayBuffer;
    juce::AudioBuffer <float> wetDryBuffer;


    int writePosition{ 0 };
    int delayBufferSize = 0;
    double sampleRate = 44100.0;
    //float delayTime = 1.0f;
    double bpm = 120.0;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XdelayAudioProcessor)
};
