#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"

class AnimeAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor&);
    ~AnimeAnalyzerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateFromProcessor();

    AnimeAnalyzerAudioProcessor& audioProcessor;

    float leftRms   = 0.0f;
    float rightRms  = 0.0f;
    float leftPeak  = 0.0f;
    float rightPeak = 0.0f;
    float peakHoldLeft  = 0.0f;
    float peakHoldRight = 0.0f;
    float correlation   = 0.0f;

    const float meterAttack  = 0.35f;
    const float meterRelease = 0.08f;
    const float peakHoldDecay = 0.92f;
    const float correlationSmoothing = 0.25f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessorEditor)
};
