#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AnimeAnalyzerAudioProcessor::AnimeAnalyzerAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                      )
{
}

AnimeAnalyzerAudioProcessor::~AnimeAnalyzerAudioProcessor() = default;

//==============================================================================
void AnimeAnalyzerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    rmsLeft.store  (0.0f);
    rmsRight.store (0.0f);
}

void AnimeAnalyzerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AnimeAnalyzerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
   #endif
}
#endif

void AnimeAnalyzerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples  = buffer.getNumSamples();

    // Pass-through: clear any extra output channels
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    if (numSamples > 0)
    {
        for (int ch = 0; ch < juce::jmin (numChannels, 2); ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);
            double sumSquares = 0.0;

            for (int i = 0; i < numSamples; ++i)
            {
                const auto s = static_cast<double> (data[i]);
                sumSquares += s * s;
            }

            const auto rms = static_cast<float> (std::sqrt (sumSquares / numSamples));

            if (ch == 0)
                rmsLeft.store (rms);
            else if (ch == 1)
                rmsRight.store (rms);
        }
    }
}

//==============================================================================
void AnimeAnalyzerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // No params yet
    destData.setSize (0);
}

void AnimeAnalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

float AnimeAnalyzerAudioProcessor::getRmsLevel (int channel) const
{
    if (channel == 0)
        return rmsLeft.load();
    if (channel == 1)
        return rmsRight.load();
    return 0.0f;
}

//==============================================================================
juce::AudioProcessorEditor* AnimeAnalyzerAudioProcessor::createEditor()
{
    return new AnimeAnalyzerAudioProcessorEditor (*this);
}

// JUCE plugin factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnimeAnalyzerAudioProcessor();
}
