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
    
    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();

	juce::AudioProcessorValueTreeState avpts {
		*this, NULL, "Parameters", createParameterLayout()
    };
private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition{ 0 };
    int delayBufferSize = 0;

    // Other necessary variables like sample rate, delay time, etc.
    double sampleRate = 44100.0;
    float delayTime = 1.0f;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XdelayAudioProcessor)
};
