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

    // Title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    auto titleArea = bounds.removeFromTop (40);
    g.drawText ("ANIME-ANALYZER", titleArea,
                juce::Justification::centred, false);

    // Spectrum area
    auto spectrumArea = bounds.reduced (40, 20);

    const float spectrumWidth  = (float) spectrumArea.getWidth();
    const float spectrumHeight = (float) spectrumArea.getHeight();

    const float columnWidth  = spectrumWidth  / (float) numSpectrumBands;
    const float cellHeight   = spectrumHeight / (float) numSpectrumCells;

    // -------------------------------------------------------------------------
    // GRID LINES
    // -------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------
    // dB SCALE LABELS ON THE LEFT
    // -------------------------------------------------------------------------

    {
        constexpr float minDb = -80.0f;
        constexpr float maxDb =   0.0f;

        static const float tickValues[] = { 0.0f, -10.0f, -20.0f, -30.0f, -40.0f, -50.0f, -60.0f };

        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (12.0f, juce::Font::plain));

        for (float tickDb : tickValues)
        {
            const float norm = juce::jmap (tickDb, minDb, maxDb, 0.0f, 1.0f);
            const float y = spectrumArea.getBottom() - norm * spectrumHeight;

            juce::Rectangle<int> textBounds ((int) spectrumArea.getX() - 40,
                                             (int) (y - 8.0f),
                                             35,
                                             16);
            g.drawFittedText (juce::String ((int) tickDb), textBounds,
                              juce::Justification::centredRight, 1);
        }
    }

    // -------------------------------------------------------------------------
    // FREQUENCY LABELS UNDER EACH BAR (CENTER FREQUENCY PER BAND)
    // -------------------------------------------------------------------------

    {
        const double minFreq = 20.0;
        const double maxFreq = 20000.0;
        const double logMin  = std::log10 (minFreq);
        const double logMax  = std::log10 (maxFreq);

        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (10.0f, juce::Font::plain));

        for (int band = 0; band < numSpectrumBands; ++band)
        {
            const double bandStart = logMin + (logMax - logMin) * (static_cast<double> (band) / numSpectrumBands);
            const double bandEnd   = logMin + (logMax - logMin) * (static_cast<double> (band + 1) / numSpectrumBands);
            const double centreLog  = 0.5 * (bandStart + bandEnd);
            const double centreFreq = std::pow (10.0, centreLog);

            juce::String labelText;
            if (centreFreq >= 1000.0)
                labelText = juce::String (juce::roundToInt ((float) (centreFreq / 1000.0f))) + "k";
            else
                labelText = juce::String (juce::roundToInt ((float) centreFreq));

            const float xCentre = spectrumArea.getX() + (band + 0.5f) * columnWidth;

            juce::Rectangle<int> labelBounds ((int) (xCentre - columnWidth * 0.5f),
                                              spectrumArea.getBottom() + 4,
                                              (int) columnWidth,
                                              16);
            g.drawFittedText (labelText, labelBounds,
                              juce::Justification::centred, 1);
        }
    }

    // -------------------------------------------------------------------------
    // SQUARE-CELL DEMON GRID
    // -------------------------------------------------------------------------

    // Current frame (can be static or animated)
    juce::Image frame;
    if (! gifFrames.isEmpty())
        frame = gifFrames[currentGifFrameIndex];

    const float cellImageSize = juce::jmin (columnWidth, cellHeight) * 0.9f; // keep a bit of margin

    // One column of stacked square cells per band
    for (int band = 0; band < numSpectrumBands; ++band)
    {
        const float level = juce::jlimit (0.0f, 1.0f, displayBandLevels[(size_t) band]);
        const int activeCells = juce::jlimit (0, numSpectrumCells,
                                              (int) std::ceil (level * (float) numSpectrumCells));

        const float x = spectrumArea.getX() + band * columnWidth;

        // Background for the whole column
        juce::Rectangle<float> columnBounds (x, (float) spectrumArea.getY(),
                                             columnWidth, spectrumHeight);
        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.fillRect (columnBounds);

        if (frame.isValid() && activeCells > 0)
        {
            for (int cell = 0; cell < activeCells; ++cell)
            {
                const float cellTop      = spectrumArea.getBottom() - (cell + 1) * cellHeight;
                const float cellCentreY  = cellTop + cellHeight * 0.5f;

                const float imageX = x + (columnWidth - cellImageSize) * 0.5f;
                const float imageY = cellCentreY - cellImageSize * 0.5f;

                juce::Rectangle<float> imageBounds (imageX, imageY, cellImageSize, cellImageSize);

                // Keep the original aspect ratio of the image, only reduce in size, never stretch.
                g.drawImage (frame,
                             imageBounds,
                             juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
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
