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
                       ), avpts(*this, NULL, "Parameters", createParameterLayout())
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
    delayBufferSize = static_cast<int>(sampleRate * 32.0); // 2 seconds buffer
    delayBuffer.setSize(getTotalNumInputChannels(), delayBufferSize);
    delayBuffer.clear();
    writePosition = 0;

    wetDryBuffer.setSize(getTotalNumInputChannels(), static_cast<int>(sampleRate));
    reverseBuffer.setSize(getTotalNumInputChannels(), delayBufferSize);

    delayTime.reset(sampleRate, 0.005);
    feedback.reset(sampleRate, 0.0005);
    mix.reset(sampleRate, 0.005);

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
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    //Set Parameter Values
    auto feedbackValue = avpts.getRawParameterValue("FEEDBACK")->load();
    feedback.setTargetValue(feedbackValue);

    auto timingValue = avpts.getRawParameterValue("TIMING")->load();
    delayTime.setTargetValue(timingValue);

    mix = avpts.getRawParameterValue("MIX")->load();

    //Calculate delay time
    processDelayTimeSamples(static_cast<bool>(avpts.getRawParameterValue("TEMPO_BASED")), timingValue);

    //DBG("Feedback: " << feedback.getNextValue());
    //DBG("Delay: " << delayTime.getNextValue());
    //DBG("BPM?" << avpts.getRawParameterValue("TEMPO_BASED")->load());
    DBG("DTIS:" << delayTimeInSamples);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {

        
        wetDryBuffer.copyFrom(channel, 0, buffer, channel, 0, bufferSize);

        

        writeToDelayBuffer(channel, bufferSize, wetDryBuffer);
        readFromDelayBuffer(channel, bufferSize, wetDryBuffer);
        writeFeedbackToDelayBuffer(channel, bufferSize, wetDryBuffer);

        

        buffer.applyGain(channel, 0, bufferSize, 1.0f - mix.getNextValue());
        wetDryBuffer.applyGain(channel, 0, bufferSize, mix.getNextValue());
        buffer.addFrom(channel, 0, wetDryBuffer, channel, 0, bufferSize);
        
        
    }
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
    
}

void XdelayAudioProcessor::processDelayTimeSamples(bool tempoBased, float timingValue)
{
	if (tempoBased)
	{
		if (auto playHead = getPlayHead())
		{
            if (auto bpm = *playHead->getPosition()->getBpm())
            
                if (bpm > 0)
                {
						auto bps = bpm / 60.0;
                    auto spb = 1.0 / bps; //seconds per beat
                    auto noteDuration = spb * (timingValue);
                    avpts.state.setProperty("TEMPO_BASED", true, nullptr);
                    delayTimeInSamples = static_cast<int>(noteDuration * sampleRate);
                }
            }
		}
    else
    {
        delayTimeInSamples = static_cast<int>(timingValue * sampleRate);
    }
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

void XdelayAudioProcessor::readFromDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer)
{
	
    auto delayTimeSamples = delayTimeInSamples;
    auto readPosition = writePosition - delayTimeSamples;
    auto g = feedback.getNextValue();

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

void XdelayAudioProcessor::writeFeedbackToDelayBuffer(int channel, int bufferSize, juce::AudioBuffer<float>& buffer)
{
    auto* delayData = delayBuffer.getWritePointer(channel);
    auto* bufferData = buffer.getWritePointer(channel);
    auto feedbackAmount = feedback.getNextValue();

    for (int i = 0; i < bufferSize; ++i)
    {
        int delayIndex = (writePosition + i) % delayBufferSize;
        delayData[delayIndex] += bufferData[i] * feedbackAmount;
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
    return new XdelayAudioProcessorEditor(*this);
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

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FEEDBACK", "Feedback", juce::NormalisableRange<float>(0.0f, 1.f, 0.01f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TIMING", "Timing", 0.0f, 16.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", juce::NormalisableRange<float>(0.0f, 1.f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("TEMPO_BASED", "Tempo_Based", true));
    return { params.begin(), params.end() };
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XdelayAudioProcessor();
}
