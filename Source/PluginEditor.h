#pragma once

#include <JuceHeader.h>
#include <array>
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
    void updateFromProcessor();
    void advanceGifAnimation (double deltaSeconds);
    void loadDemonGif();

    AnimeAnalyzerAudioProcessor& audioProcessor;

    static constexpr int numSpectrumBands  = AnimeAnalyzerAudioProcessor::getNumSpectrumBands();
    static constexpr int numSpectrumCells  = 24; // vertical grid cells for RME-style look

    std::array<float, numSpectrumBands> displayBandLevels {};
    float meterDecay = 0.75f;

    juce::Array<juce::Image> gifFrames;
    int currentGifFrameIndex = 0;
    double gifTimeAccumulatorSeconds = 0.0;
    double gifFrameDurationSeconds   = 1.0 / 24.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimeAnalyzerAudioProcessorEditor)
};
