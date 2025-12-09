#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class AnimeAnalyzerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor&);
    ~AnimeAnalyzerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    AnimeAnalyzerAudioProcessor& audioProcessor;
    float displayedLeft { 0.0f };
    float displayedRight { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessorEditor)
};
