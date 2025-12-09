#include "PluginEditor.h"

//==============================================================================
AnimeAnalyzerAudioProcessorEditor::AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
    startTimerHz (30); // ~30 FPS
}

AnimeAnalyzerAudioProcessorEditor::~AnimeAnalyzerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void AnimeAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto bounds = getLocalBounds().reduced (20);

    // Title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("ANIME-ANALYZER", bounds.removeFromTop (30),
                juce::Justification::centred, false);

    auto meterArea = bounds.reduced (40);
    auto leftArea  = meterArea.removeFromLeft (meterArea.getWidth() / 2).reduced (10);
    auto rightArea = meterArea.reduced (10);

    auto drawMeter = [&g] (juce::Rectangle<int> area, float level)
    {
        level = juce::jlimit (0.0f, 1.0f, level);

        // background
        g.setColour (juce::Colours::darkgrey);
        g.fillRect (area);

        // filled bar from bottom
        const int filledHeight = static_cast<int> (area.getHeight() * level);
        juce::Rectangle<int> filled (
            area.getX(),
            area.getBottom() - filledHeight,
            area.getWidth(),
            filledHeight);

        g.setColour (juce::Colours::hotpink);
        g.fillRect (filled);
    };

    drawMeter (leftArea,  leftLevel);
    drawMeter (rightArea, rightLevel);
}

void AnimeAnalyzerAudioProcessorEditor::resized()
{
}

//==============================================================================
void AnimeAnalyzerAudioProcessorEditor::timerCallback()
{
    updateFromProcessor();
}

void AnimeAnalyzerAudioProcessorEditor::updateFromProcessor()
{
    const float newLeft  = audioProcessor.getRmsLevel (0);
    const float newRight = audioProcessor.getRmsLevel (1);

    leftLevel  = juce::jlimit (0.0f, 1.0f,
                               leftLevel  * meterDecay + (1.0f - meterDecay) * newLeft);

    rightLevel = juce::jlimit (0.0f, 1.0f,
                               rightLevel * meterDecay + (1.0f - meterDecay) * newRight);

    repaint();
}
