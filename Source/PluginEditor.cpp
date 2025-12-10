#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>

//==============================================================================
AnimeAnalyzerAudioProcessorEditor::AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (900, 500);
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
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    auto titleArea = bounds.removeFromTop (40);
    g.drawText ("ANIME-ANALYZER", titleArea,
                juce::Justification::centred, false);

    auto spectrumArea = bounds.reduced (40, 20);

    const float spectrumWidth  = (float) spectrumArea.getWidth();
    const float spectrumHeight = (float) spectrumArea.getHeight();

    const float columnWidth  = spectrumWidth  / (float) numSpectrumBands;
    const float cellHeight   = spectrumHeight / (float) numSpectrumCells;

    // Vertical grid lines (one per band)
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    for (int band = 0; band <= numSpectrumBands; ++band)
    {
        const float x = spectrumArea.getX() + band * columnWidth;
        g.drawLine (x, (float) spectrumArea.getY(),
                    x, (float) spectrumArea.getBottom(), 1.0f);
    }

    // Horizontal grid lines
    for (int cell = 0; cell <= numSpectrumCells; ++cell)
    {
        const float y = spectrumArea.getBottom() - cell * cellHeight;
        g.drawLine ((float) spectrumArea.getX(), y,
                    (float) spectrumArea.getRight(), y, 1.0f);
    }

    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (14.0f, juce::Font::bold));

    auto drawFreqLabel = [&] (float freq, const juce::String& text)
    {
        const double minFreq = 20.0;
        const double maxFreq = 20000.0;
        const double logMin  = std::log10 (minFreq);
        const double logMax  = std::log10 (maxFreq);

        const double logF = std::log10 ((double) freq);
        const double t = (logF - logMin) / (logMax - logMin);

        const float x = (float) spectrumArea.getX() + (float) t * spectrumWidth;
        juce::Rectangle<float> labelBounds (x - 20.0f, (float) spectrumArea.getBottom() + 4.0f, 40.0f, 18.0f);
        g.drawFittedText (text, labelBounds.toNearestInt(), juce::Justification::centred, 1);
    };

    drawFreqLabel (20.0f,   "20");
    drawFreqLabel (100.0f,  "100");
    drawFreqLabel (1000.0f, "1k");
    drawFreqLabel (10000.0f,"10k");
    drawFreqLabel (20000.0f,"20k");

    juce::Image frame;
    if (! gifFrames.isEmpty())
        frame = gifFrames[currentGifFrameIndex];

    for (int band = 0; band < numSpectrumBands; ++band)
    {
        const float level = juce::jlimit (0.0f, 1.0f, displayBandLevels[(size_t) band]);
        const float barHeight = level * spectrumHeight;

        const float x = spectrumArea.getX() + band * columnWidth;
        const float y = spectrumArea.getBottom() - barHeight;

        juce::Rectangle<float> columnBounds (x, (float) spectrumArea.getY(), columnWidth, spectrumHeight);
        juce::Rectangle<float> barBounds (x, y, columnWidth, barHeight);

        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.fillRect (columnBounds);

        if (barHeight > 0.0f && frame.isValid())
        {
            g.drawImage (frame,
                         barBounds,
                         juce::RectanglePlacement::stretchToFit);
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
        const float current = displayBandLevels[(size_t) i];

        const float smoothed =
            juce::jlimit (0.0f, 1.0f,
                          current * meterDecay + (1.0f - meterDecay) * target);

        displayBandLevels[(size_t) i] = smoothed;
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
    gifFrames.clear();

    auto* data = BinaryData::demon_girl_gif;
    auto  size = BinaryData::demon_girl_gifSize;

    if (data == nullptr || size <= 0)
    {
        juce::Image fallback (juce::Image::ARGB, 64, 128, true);
        juce::Graphics g (fallback);
        g.fillAll (juce::Colours::red);
        gifFrames.add (fallback);
        return;
    }

    juce::MemoryInputStream stream (data, size, false);
    juce::GIFImageFormat gifFormat;

    if (auto image = gifFormat.decodeImage (stream); image.isValid())
    {
        gifFrames.add (image);
    }

    if (gifFrames.isEmpty())
    {
        juce::Image fallback (juce::Image::ARGB, 64, 128, true);
        juce::Graphics g (fallback);
        g.fillAll (juce::Colours::red);
        gifFrames.add (fallback);
    }
}
