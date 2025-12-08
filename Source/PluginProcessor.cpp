#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================
AnimeAnalyzerAudioProcessor::AnimeAnalyzerAudioProcessor()
    : juce::AudioProcessor (juce::BusesProperties()
                                .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

//==============================================================
void AnimeAnalyzerAudioProcessor::prepareToPlay (double, int)
{
    for (auto& level : rmsLevels)
        level.store (0.0f);
}

void AnimeAnalyzerAudioProcessor::releaseResources() {}

bool AnimeAnalyzerAudioProcessor::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
{
    auto mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::stereo()
        || mainOut == juce::AudioChannelSet::mono();
}

void AnimeAnalyzerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numInputChannels  = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples        = buffer.getNumSamples();

    for (int ch = numInputChannels; ch < numOutputChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    for (int ch = 0; ch < numInputChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer (ch);

        double sumSquares = 0.0;
        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = channelData[i];
            sumSquares += (double) sample * (double) sample;
        }

        const float rms = numSamples > 0 ? std::sqrt ((float) (sumSquares / numSamples)) : 0.0f;
        updateLevelSmoother (ch, rms);
    }
}

//==============================================================
void AnimeAnalyzerAudioProcessor::updateLevelSmoother (int channel, float newLevel)
{
    if (channel < 0 || channel >= (int) rmsLevels.size())
        return;

    const float current = rmsLevels[(size_t) channel].load();
    const float smoothed = current + smoothingCoefficient * (newLevel - current);
    rmsLevels[(size_t) channel].store (smoothed);
}

float AnimeAnalyzerAudioProcessor::getRmsLevel (int channel) const
{
    if (channel < 0 || channel >= (int) rmsLevels.size())
        return 0.0f;

    return rmsLevels[(size_t) channel].load();
}

//==============================================================
juce::AudioProcessorEditor* AnimeAnalyzerAudioProcessor::createEditor()
{
    return new AnimeAnalyzerAudioProcessorEditor (*this);
}

const juce::String AnimeAnalyzerAudioProcessor::getName() const      { return "ANIME-ANALYZER"; }
bool AnimeAnalyzerAudioProcessor::acceptsMidi() const                { return false; }
bool AnimeAnalyzerAudioProcessor::producesMidi() const               { return false; }
bool AnimeAnalyzerAudioProcessor::isMidiEffect() const               { return false; }
double AnimeAnalyzerAudioProcessor::getTailLengthSeconds() const     { return 0.0; }

int AnimeAnalyzerAudioProcessor::getNumPrograms()                    { return 1; }
int AnimeAnalyzerAudioProcessor::getCurrentProgram()                 { return 0; }
void AnimeAnalyzerAudioProcessor::setCurrentProgram (int)            {}
const juce::String AnimeAnalyzerAudioProcessor::getProgramName (int) { return "Default"; }
void AnimeAnalyzerAudioProcessor::changeProgramName (int, const juce::String&) {}

void AnimeAnalyzerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void AnimeAnalyzerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}
