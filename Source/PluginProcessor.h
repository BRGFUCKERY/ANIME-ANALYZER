#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>
#include <vector>

class AnimeAnalyzerAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numSpectrumBands = 31;

    AnimeAnalyzerAudioProcessor();
    ~AnimeAnalyzerAudioProcessor() override;

    //==============================================================================
    const juce::String getName() const override { return "ANIME-ANALYZER"; }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Simple RMS getter for UI (0.0 - 1.0 approx)
    float getRmsLevel (int channel) const;
    float getPeakLevel (int channel) const;
    float getCorrelation() const { return correlation.load(); }

    float getSpectrumBandLevel (int bandIndex) const;
    static constexpr int getNumSpectrumBands() { return numSpectrumBands; }

private:
    double currentSampleRate { 44100.0 };

    static constexpr int fftOrder = 11; // 2048 samples
    static constexpr int fftSize  = 1 << fftOrder;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { static_cast<size_t> (fftSize), juce::dsp::WindowingFunction<float>::hann, false };

    std::vector<float> fftFifo;
    std::vector<float> fftBuffer;
    std::vector<float> magnitudes;
    int fifoIndex { 0 };

    std::array<std::atomic<float>, numSpectrumBands> spectrumBandLevels {};

    std::atomic<float> rmsLeft  { 0.0f };
    std::atomic<float> rmsRight { 0.0f };
    std::atomic<float> peakLeft  { 0.0f };
    std::atomic<float> peakRight { 0.0f };
    std::atomic<float> correlation { 0.0f };

    void pushNextSampleIntoFifo (float sample) noexcept;
    void performFFTAnalysis();
    void updateSpectrumBands (const float* magnitudes, int numMagnitudes);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessor)
};
