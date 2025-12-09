#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

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
    peakLeft.store (0.0f);
    peakRight.store (0.0f);
    correlation.store (0.0f);
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
        double sumSquaresLeft  = 0.0;
        double sumSquaresRight = 0.0;
        double sumCross        = 0.0;

        float peakL = 0.0f;
        float peakR = 0.0f;

        const bool hasLeft  = numChannels > 0;
        const bool hasRight = numChannels > 1;

        if (hasLeft)
        {
            const auto* data = buffer.getReadPointer (0);

            for (int i = 0; i < numSamples; ++i)
            {
                const auto s = static_cast<double> (data[i]);
                sumSquaresLeft += s * s;
                peakL = juce::jmax (peakL, static_cast<float> (std::abs (data[i])));
            }

            rmsLeft.store (static_cast<float> (std::sqrt (sumSquaresLeft / numSamples)));
            peakLeft.store (peakL);
        }

        if (hasRight)
        {
            const auto* data = buffer.getReadPointer (1);

            for (int i = 0; i < numSamples; ++i)
            {
                const auto s = static_cast<double> (data[i]);
                sumSquaresRight += s * s;
                peakR = juce::jmax (peakR, static_cast<float> (std::abs (data[i])));
            }

            rmsRight.store (static_cast<float> (std::sqrt (sumSquaresRight / numSamples)));
            peakRight.store (peakR);
        }

        if (hasLeft && hasRight)
        {
            const auto* left  = buffer.getReadPointer (0);
            const auto* right = buffer.getReadPointer (1);

            for (int i = 0; i < numSamples; ++i)
                sumCross += static_cast<double> (left[i]) * static_cast<double> (right[i]);

            const auto denom = std::sqrt (sumSquaresLeft * sumSquaresRight);
            const auto corr = (denom > 0.0) ? juce::jlimit (-1.0, 1.0, sumCross / denom) : 0.0;
            correlation.store (static_cast<float> (corr));
        }
        else
        {
            correlation.store (0.0f);
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

float AnimeAnalyzerAudioProcessor::getPeakLevel (int channel) const
{
    if (channel == 0)
        return peakLeft.load();
    if (channel == 1)
        return peakRight.load();
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
