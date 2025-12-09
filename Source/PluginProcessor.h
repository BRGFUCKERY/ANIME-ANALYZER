#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

class AnimeAnalyzerAudioProcessor : public juce::AudioProcessor
{
public:
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

private:
    std::atomic<float> rmsLeft  { 0.0f };
    std::atomic<float> rmsRight { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessor)
};
