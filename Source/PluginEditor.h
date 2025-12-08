#pragma once

#include <JuceHeader.h>
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

    float leftLevel  = 0.0f;
    float rightLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessorEditor)
};
