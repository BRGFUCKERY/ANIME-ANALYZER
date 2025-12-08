#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace
{
    constexpr int timerHz = 30;
    constexpr float meterDecay = 0.8f;
}

AnimeAnalyzerAudioProcessorEditor::AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setOpaque (true);
    setSize (420, 260);
    startTimerHz (timerHz);
}

void AnimeAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkslategrey);

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    g.drawText ("ANIME-ANALYZER", getLocalBounds().removeFromTop (40), juce::Justification::centred);

    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (50);

    auto meterArea = area.reduced (10);
    auto leftBar = meterArea.removeFromLeft (meterArea.getWidth() / 2 - 5);
    meterArea.removeFromLeft (10);
    auto rightBar = meterArea;

    const auto drawMeter = [&g] (juce::Rectangle bounds, float level, juce::Colour colour)
    {
        g.setColour (colour.withAlpha (0.35f));
        g.fillRoundedRectangle (bounds.toFloat(), 6.0f);

        auto fillWidth = (int) std::round (bounds.getWidth() * juce::jlimit (0.0f, 1.0f, level));
        juce::Rectangle fill { bounds.getX(), bounds.getY(), fillWidth, bounds.getHeight() };

        g.setColour (colour);
        g.fillRoundedRectangle (fill.toFloat(), 6.0f);
    };

    drawMeter (leftBar, leftLevel, juce::Colours::aqua);
    drawMeter (rightBar, rightLevel, juce::Colours::hotpink);
}

void AnimeAnalyzerAudioProcessorEditor::resized()
{
}

void AnimeAnalyzerAudioProcessorEditor::timerCallback()
{
    updateFromProcessor();
}

void AnimeAnalyzerAudioProcessorEditor::updateFromProcessor()
{
    const float newLeft  = audioProcessor.getRmsLevel (0);
    const float newRight = audioProcessor.getRmsLevel (1);

    leftLevel  = juce::jlimit (0.0f, 1.0f, leftLevel  * meterDecay + (1.0f - meterDecay) * newLeft);
    rightLevel = juce::jlimit (0.0f, 1.0f, rightLevel * meterDecay + (1.0f - meterDecay) * newRight);

    repaint();
}
