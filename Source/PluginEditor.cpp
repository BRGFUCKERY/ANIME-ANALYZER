#include "PluginEditor.h"

AnimeAnalyzerAudioProcessorEditor::AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (300, 200);
    startTimerHz (30);
}

AnimeAnalyzerAudioProcessorEditor::~AnimeAnalyzerAudioProcessorEditor() = default;

void AnimeAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto area = getLocalBounds().reduced (20);
    auto barWidth = area.getWidth() / 4;
    auto gap = barWidth;

    auto leftHeight = static_cast<int> (displayedLeft * static_cast<float> (area.getHeight()));
    auto rightHeight = static_cast<int> (displayedRight * static_cast<float> (area.getHeight()));

    juce::Rectangle<int> leftBar (area.getX() + barWidth / 2, area.getBottom() - leftHeight, barWidth, leftHeight);
    juce::Rectangle<int> rightBar (area.getX() + barWidth / 2 + barWidth + gap, area.getBottom() - rightHeight, barWidth, rightHeight);

    g.setColour (juce::Colours::deeppink);
    g.fillRect (leftBar);

    g.setColour (juce::Colours::aqua);
    g.fillRect (rightBar);

    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawText ("ANIME-ANALYZER", area.removeFromTop (24), juce::Justification::centred);
}

void AnimeAnalyzerAudioProcessorEditor::resized()
{
}

void AnimeAnalyzerAudioProcessorEditor::timerCallback()
{
    constexpr float smoothing = 0.2f;

    const float targetLeft = audioProcessor.getRmsLevel (0);
    const float targetRight = audioProcessor.getRmsLevel (1);

    displayedLeft += smoothing * (targetLeft - displayedLeft);
    displayedRight += smoothing * (targetRight - displayedRight);

    repaint();
}
