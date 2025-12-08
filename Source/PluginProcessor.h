#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <array>

class AnimeAnalyzerAudioProcessor : public juce::AudioProcessor
{
public:
    AnimeAnalyzerAudioProcessor();
    ~AnimeAnalyzerAudioProcessor() override = default;

    //==============================================================
    // Core AudioProcessor overrides
    //==============================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================
    // Editor
    //==============================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================
    // Metadata
    //==============================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================
    // Programs
    //==============================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================
    // State
    //==============================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================
    // Meter helpers
    //==============================================================
    float getRmsLevel (int channel) const;

private:
    void updateLevelSmoother (int channel, float newLevel);

    std::array<std::atomic<float>, 2> rmsLevels { 0.0f, 0.0f };
    float smoothingCoefficient = 0.2f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessor)
};
