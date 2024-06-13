/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
XdelayAudioProcessor::XdelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

XdelayAudioProcessor::~XdelayAudioProcessor()
{
}

//==============================================================================
const juce::String XdelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool XdelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool XdelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool XdelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double XdelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int XdelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int XdelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void XdelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String XdelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void XdelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void XdelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    this->sampleRate = sampleRate;
    delayBufferSize = static_cast<int>(sampleRate * 2.0); // 2 seconds buffer
    delayBuffer.setSize(getTotalNumInputChannels(), delayBufferSize);
    delayBuffer.clear();
    writePosition = 0;
}

void XdelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool XdelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void XdelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto bufferSize = buffer.getNumSamples();
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    //auto delayBufferSize = delayBuffer.getNumSamples();

    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        
        writeToDelayBuffer(channel, bufferSize, buffer);
        readToDelayBuffer(channel, bufferSize, buffer);
        writeToDelayBuffer(channel, bufferSize, buffer);
        
    }
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
    
}

void XdelayAudioProcessor::writeToDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer)
{
    auto* channelData = buffer.getWritePointer(channel);
    if (delayBufferSize >= bufferSize + writePosition)
    {
        delayBuffer.copyFrom(channel, writePosition, channelData, bufferSize);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - writePosition;
        delayBuffer.copyFrom(channel, writePosition, channelData, numSamplesToEnd);
        delayBuffer.copyFrom(channel, 0, channelData + numSamplesToEnd, bufferSize - numSamplesToEnd);
    }
}

void XdelayAudioProcessor::readToDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer)
{
   
    auto delayTimeSamples = static_cast<int>(delayTime * sampleRate);
    auto readPosition = writePosition - delayTimeSamples;
    auto g = 0.2f;
    if (readPosition < 0)
        readPosition += delayBufferSize;

    if (readPosition + bufferSize < delayBufferSize)
    {
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), bufferSize, g, g);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - readPosition;
        buffer.addFromWithRamp(channel, 0, delayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, g, g);
        auto numSamplesToStart = bufferSize - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0), numSamplesToStart, g, g);
    }
}

//==================s============================================================
bool XdelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* XdelayAudioProcessor::createEditor()
{
    //return new XdelayAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void XdelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream stream(destData, true);
    avpts.state.writeToStream(stream);
}

void XdelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::MemoryInputStream stream(data, static_cast<size_t> (sizeInBytes), false);
    juce::ValueTree tree = juce::ValueTree::readFromStream(stream);

    if (tree.isValid())
    {
        avpts.state = tree;
    }
}   
juce::AudioProcessorValueTreeState::ParameterLayout XdelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("wet", "Wet", 
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f, 1.0f), 100));

    layout.add(std::make_unique<juce::AudioParameterFloat>("dry", "Dry", 
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f, 1.0f), 100));

    layout.add(std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f, 1.0f), 100));

    layout.add(std::make_unique<juce::AudioParameterFloat>("timing", "Timing",
        juce::NormalisableRange<float>(0.0f, 32.0f, 0.5f, 1.0f), 100));

    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XdelayAudioProcessor();
}
