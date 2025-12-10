#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>
#include <vector>

//==============================================================================
AnimeAnalyzerAudioProcessorEditor::AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (820, 420);
    loadDemonGif();
    startTimerHz (30);
}

AnimeAnalyzerAudioProcessorEditor::~AnimeAnalyzerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void AnimeAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto bounds = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (24.0f, {}, juce::Font::bold)));
    g.drawText ("ANIME-ANALYZER", bounds.removeFromTop (40), juce::Justification::centred, false);

    auto spectrumArea = getLocalBounds().reduced (20).withTrimmedTop (40);
    const float cellWidth  = spectrumArea.getWidth()  / (float) numSpectrumBands;
    const float cellHeight = spectrumArea.getHeight() / (float) numSpectrumCells;

    // Grid lines
    g.setColour (juce::Colours::white);
    for (int b = 0; b <= numSpectrumBands; ++b)
    {
        const float x = spectrumArea.getX() + b * cellWidth;
        g.drawLine (x, (float) spectrumArea.getY(), x, (float) spectrumArea.getBottom());
    }

    g.drawLine ((float) spectrumArea.getX(), (float) spectrumArea.getY(),
                (float) spectrumArea.getRight(), (float) spectrumArea.getY());
    g.drawLine ((float) spectrumArea.getX(), (float) spectrumArea.getBottom(),
                (float) spectrumArea.getRight(), (float) spectrumArea.getBottom());

    // Frequency labels (approx log spacing)
    const std::vector<std::pair<juce::String, float>> freqLabels {
        { "20",   20.0f },
        { "100",  100.0f },
        { "1k",   1000.0f },
        { "10k",  10000.0f },
        { "20k",  20000.0f }
    };

    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    for (const auto& label : freqLabels)
    {
        const float position = juce::jmap (std::log10 (label.second / minFreq),
                                           0.0f, std::log10 (maxFreq / minFreq),
                                           (float) spectrumArea.getX(), (float) spectrumArea.getRight());
        g.drawText (label.first, (int) position - 12, spectrumArea.getBottom() + 4, 40, 20,
                    juce::Justification::centredLeft, false);
    }

    auto frame = gifFrames.isEmpty() ? juce::Image() : gifFrames[currentGifFrameIndex];

    for (int b = 0; b < numSpectrumBands; ++b)
    {
        const float level = displayBandLevels[(size_t) b];
        const int litCells = (int) std::round (level * (float) numSpectrumCells);

        for (int c = 0; c < numSpectrumCells; ++c)
        {
            const float x = spectrumArea.getX() + b * cellWidth;
            const float y = spectrumArea.getBottom() - (c + 1) * cellHeight;
            juce::Rectangle<float> cellBounds { x, y, cellWidth, cellHeight };

            if (c < litCells)
            {
                g.drawImage (frame, cellBounds, juce::RectanglePlacement::stretchToFit);
            }
            else
            {
                g.setColour (juce::Colours::black.withAlpha (0.8f));
                g.fillRect (cellBounds);
                g.setColour (juce::Colours::white);
            }
        }
    }
}

void AnimeAnalyzerAudioProcessorEditor::resized()
{
}

//==============================================================================
void AnimeAnalyzerAudioProcessorEditor::timerCallback()
{
    updateFromProcessor();
    advanceGifAnimation (1.0 / 30.0);
    repaint();
}

void AnimeAnalyzerAudioProcessorEditor::updateFromProcessor()
{
    for (int i = 0; i < numSpectrumBands; ++i)
    {
        const float target = audioProcessor.getSpectrumBandLevel (i);
        displayBandLevels[(size_t) i] =
            juce::jlimit (0.0f, 1.0f,
                displayBandLevels[(size_t) i] * meterDecay +
                (1.0f - meterDecay) * target);
    }
}

void AnimeAnalyzerAudioProcessorEditor::advanceGifAnimation (double deltaSeconds)
{
    if (gifFrames.isEmpty())
        return;

    gifTimeAccumulatorSeconds += deltaSeconds;
    if (gifTimeAccumulatorSeconds >= gifFrameDurationSeconds)
    {
        gifTimeAccumulatorSeconds -= gifFrameDurationSeconds;
        currentGifFrameIndex = (currentGifFrameIndex + 1) % gifFrames.size();
    }
}

void AnimeAnalyzerAudioProcessorEditor::loadDemonGif()
{
    auto* data = BinaryData::demon_girl_gif;
    auto  size = BinaryData::demon_girl_gifSize;

    juce::MemoryInputStream stream (data, size, false);
    juce::GIFImageFormat gif;

    if (auto image = gif.decodeImage (stream); image.isValid())
        gifFrames.add (image);

    if (gifFrames.isEmpty())
    {
        juce::Image fallback (juce::Image::RGB, 10, 10, true);
        fallback.clear (fallback.getBounds(), juce::Colours::red);
        gifFrames.add (fallback);
    }
}
