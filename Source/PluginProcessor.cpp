#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

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
    fftFifo.resize (fftSize, 0.0f);
    fftBuffer.resize (fftSize * 2, 0.0f);
    magnitudes.resize (fftSize / 2, 0.0f);

    for (auto& band : spectrumBandLevels)
        band.store (0.0f);
}

AnimeAnalyzerAudioProcessor::~AnimeAnalyzerAudioProcessor() = default;

//==============================================================================
void AnimeAnalyzerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;

    fifoIndex = 0;
    std::fill (fftFifo.begin(), fftFifo.end(), 0.0f);
    std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);
    std::fill (magnitudes.begin(), magnitudes.end(), 0.0f);

    rmsLeft.store  (0.0f);
    rmsRight.store (0.0f);
    peakLeft.store (0.0f);
    peakRight.store (0.0f);
    correlation.store (0.0f);

    for (auto& band : spectrumBandLevels)
        band.store (0.0f);
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

    const bool hasLeft  = numChannels > 0;
    const bool hasRight = numChannels > 1;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float monoSample = 0.0f;

        if (hasLeft)
            monoSample += buffer.getReadPointer (0)[sample];

        if (hasRight)
            monoSample += buffer.getReadPointer (1)[sample];

        if (hasLeft && hasRight)
            monoSample *= 0.5f;

        pushNextSampleIntoFifo (monoSample);
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

float AnimeAnalyzerAudioProcessor::getSpectrumBandLevel (int bandIndex) const
{
    if (bandIndex < 0 || bandIndex >= numSpectrumBands)
        return 0.0f;

    return spectrumBandLevels[(size_t) bandIndex].load();
}

void AnimeAnalyzerAudioProcessor::pushNextSampleIntoFifo (float sample) noexcept
{
    if (fifoIndex < fftSize)
    {
        fftFifo[(size_t) fifoIndex++] = sample;

        if (fifoIndex == fftSize)
            performFFTAnalysis();
    }
}

void AnimeAnalyzerAudioProcessor::performFFTAnalysis()
{
    std::copy (fftFifo.begin(), fftFifo.end(), fftBuffer.begin());
    window.multiplyWithWindowingTable (fftBuffer.data(), (size_t) fftSize);

    fft.performRealOnlyForwardTransform (fftBuffer.data());

    const int numMagnitudes = fftSize / 2;

    for (int bin = 1; bin < numMagnitudes; ++bin)
    {
        const auto real = fftBuffer[(size_t) bin * 2];
        const auto imag = fftBuffer[(size_t) bin * 2 + 1];
        magnitudes[(size_t) bin] = std::sqrt (real * real + imag * imag) / static_cast<float> (fftSize);
    }

    updateSpectrumBands (magnitudes.data(), numMagnitudes);

    fifoIndex = 0;
}

void AnimeAnalyzerAudioProcessor::updateSpectrumBands (const float* magnitudes, int numMagnitudes)
{
    if (currentSampleRate <= 0.0 || magnitudes == nullptr || numMagnitudes <= 0)
        return;

    const double minFreq = 20.0;
    const double maxFreq = 20000.0;
    const double logMin  = std::log10 (minFreq);
    const double logMax  = std::log10 (maxFreq);

    const double nyquist     = currentSampleRate * 0.5;
    const double binFreqStep = nyquist / static_cast<double> (numMagnitudes);

    for (int band = 0; band < numSpectrumBands; ++band)
    {
        const double bandStart = logMin + (logMax - logMin)
                               * (static_cast<double> (band) / numSpectrumBands);
        const double bandEnd   = logMin + (logMax - logMin)
                               * (static_cast<double> (band + 1) / numSpectrumBands);

        const double freqLow  = std::pow (10.0, bandStart);
        const double freqHigh = std::pow (10.0, bandEnd);

        // Map frequency range to FFT bin indices.
        int binStart = (int) std::floor (freqLow  / binFreqStep);
        int binEnd   = (int) std::ceil  (freqHigh / binFreqStep);

        // Clamp into valid range and make sure we always have at least one bin.
        binStart = juce::jlimit (1, numMagnitudes - 1, binStart);
        binEnd   = juce::jlimit (binStart, numMagnitudes - 1, binEnd);

        double magnitudeSum = 0.0;
        int binCount = 0;

        for (int bin = binStart; bin <= binEnd; ++bin)
        {
            magnitudeSum += static_cast<double> (magnitudes[bin]);
            ++binCount;
        }

        const double magnitude = (binCount > 0)
                                   ? magnitudeSum / static_cast<double> (binCount)
                                   : 0.0;

        const float dbValue = juce::Decibels::gainToDecibels (static_cast<float> (magnitude), -100.0f);
        const float normalized = juce::jlimit (0.0f, 1.0f,
                                               juce::jmap (dbValue, -80.0f, 0.0f, 0.0f, 1.0f));

        spectrumBandLevels[(size_t) band].store (normalized);
    }
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
