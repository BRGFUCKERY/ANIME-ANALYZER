#include "PluginEditor.h"

//==============================================================================
AnimeAnalyzerAudioProcessorEditor::AnimeAnalyzerAudioProcessorEditor (AnimeAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (820, 420);
    startTimerHz (30); // ~30 FPS for smooth meters
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

    const auto makeFont = [] (float height, int style = juce::Font::plain)
    {
        return juce::Font (height, style);
    };

    // Neon border and header
    juce::ColourGradient backdrop (juce::Colour::fromRGB (10, 12, 20), 0.0f, 0.0f,
                                   juce::Colour::fromRGB (20, 22, 40), 0.0f, (float) bounds.getHeight(),
                                   false);
    g.setGradientFill (backdrop);
    g.fillRect (bounds);

    g.setColour (juce::Colours::darkslategrey.withAlpha (0.8f));
    g.drawRect (bounds.reduced (4), 2.0f);

    auto header = bounds.removeFromTop (60).reduced (20, 10);
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (makeFont (26.0f, juce::Font::bold));
    g.drawText ("Anime Analyzer | DigiCheck-inspired meters", header, juce::Justification::centredLeft, false);

    g.setFont (makeFont (13.0f));
    g.setColour (juce::Colours::lightgrey);
    g.drawText ("Stereo RMS, peak, and phase correlation", bounds.removeFromTop (24).reduced (20, 0),
                juce::Justification::centredLeft, false);

    auto content = bounds.reduced (18);
    auto meterArea = content.removeFromLeft ((int) (content.getWidth() * 0.65f)).reduced (10);
    auto correlationArea = content.reduced (10);

    const auto columnWidth = meterArea.getWidth() / 2;
    auto leftArea  = meterArea.removeFromLeft (columnWidth).reduced (10);
    auto rightArea = meterArea.reduced (10);

    auto drawMeter = [&g, &makeFont] (juce::Rectangle<int> area, float rms, float peak, float peakHold, const juce::String& label)
    {
        rms = juce::jlimit (0.0f, 1.2f, rms);
        peak = juce::jlimit (0.0f, 1.2f, peak);
        peakHold = juce::jlimit (0.0f, 1.2f, peakHold);

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (area.toFloat(), 6.0f);
        g.setColour (juce::Colours::darkgrey);
        g.drawRoundedRectangle (area.toFloat(), 6.0f, 1.5f);

        auto bar = area.reduced (12);

        // Gradient meter fill
        const int filledHeight = static_cast<int> (bar.getHeight() * rms);
        juce::Rectangle<int> filled (
            bar.getX(),
            bar.getBottom() - filledHeight,
            bar.getWidth(),
            filledHeight);

        juce::ColourGradient gradient (juce::Colours::darkgreen, (float) filled.getX(), (float) filled.getBottom(),
                                       juce::Colours::red, (float) filled.getX(), (float) filled.getY(), false);
        gradient.addColour (0.5, juce::Colours::yellow);
        g.setGradientFill (gradient);
        g.fillRect (filled);

        // Peak hold line
        const int peakY = bar.getBottom() - static_cast<int> (bar.getHeight() * peakHold);
        g.setColour (juce::Colours::aqua);
        g.drawLine ((float) bar.getX(), (float) peakY, (float) bar.getRight(), (float) peakY, 2.0f);

        // Current peak marker
        const int peakCurrentY = bar.getBottom() - static_cast<int> (bar.getHeight() * peak);
        g.setColour (juce::Colours::lightblue.withAlpha (0.7f));
        g.drawLine ((float) bar.getX(), (float) peakCurrentY, (float) bar.getRight(), (float) peakCurrentY, 1.0f);

        g.setColour (juce::Colours::whitesmoke);
        g.setFont (makeFont (16.0f, juce::Font::bold));
        g.drawText (label, area.removeFromTop (24), juce::Justification::centred, false);

        g.setFont (makeFont (13.0f));
        g.setColour (juce::Colours::lightgrey);
        g.drawText (juce::String::formatted ("RMS: %.2f", rms), area.removeFromBottom (26), juce::Justification::centred, false);
        g.drawText (juce::String::formatted ("Peak: %.2f", peak), area.removeFromBottom (22), juce::Justification::centred, false);
    };

    drawMeter (leftArea,  leftRms,  leftPeak,  peakHoldLeft,  "LEFT");
    drawMeter (rightArea, rightRms, rightPeak, peakHoldRight, "RIGHT");

    // Correlation meter inspired by DigiCheck's Stereo Correlation Meter
    auto corrBox = correlationArea.reduced (10);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (corrBox.toFloat(), 8.0f);
    g.setColour (juce::Colours::darkgrey);
    g.drawRoundedRectangle (corrBox.toFloat(), 8.0f, 1.5f);

    auto corrLabel = corrBox.removeFromTop (24);
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (makeFont (16.0f, juce::Font::bold));
    g.drawText ("PHASE CORRELATION", corrLabel, juce::Justification::centred, false);

    auto corrMeter = corrBox.reduced (20, 18);
    g.setColour (juce::Colours::darkgrey);
    g.fillRect (corrMeter);

    const float corrClamped = juce::jlimit (-1.0f, 1.0f, correlation);
    const int centreX = corrMeter.getX() + corrMeter.getWidth() / 2;
    const int zeroY = corrMeter.getCentreY();
    const int markerY = zeroY;
    const int markerHeight = corrMeter.getHeight();

    // draw grid
    g.setColour (juce::Colours::darkslategrey);
    g.drawVerticalLine (centreX, (float) corrMeter.getY(), (float) corrMeter.getBottom());
    g.drawVerticalLine (corrMeter.getX(), (float) corrMeter.getY(), (float) corrMeter.getBottom());
    g.drawVerticalLine (corrMeter.getRight(), (float) corrMeter.getY(), (float) corrMeter.getBottom());

    // filled bar showing correlation
    const int corrFill = static_cast<int> ((corrMeter.getWidth() / 2) * corrClamped);
    juce::Rectangle<int> corrFillRect = corrFill >= 0
        ? juce::Rectangle<int> (centreX, corrMeter.getY(), corrFill, corrMeter.getHeight())
        : juce::Rectangle<int> (centreX + corrFill, corrMeter.getY(), -corrFill, corrMeter.getHeight());

    juce::ColourGradient corrGrad (juce::Colours::red, (float) corrMeter.getX(), (float) corrMeter.getCentreY(),
                                   juce::Colours::green, (float) corrMeter.getRight(), (float) corrMeter.getCentreY(), false);
    corrGrad.addColour (0.5, juce::Colours::yellow);
    g.setGradientFill (corrGrad);
    g.fillRect (corrFillRect);

    // marker line at current correlation
    g.setColour (juce::Colours::aqua);
    const int markerX = centreX + corrFill;
    g.drawLine ((float) markerX, (float) corrMeter.getY(), (float) markerX, (float) corrMeter.getBottom(), 2.0f);

    g.setColour (juce::Colours::lightgrey);
    g.setFont (makeFont (12.0f));
    g.drawText ("-1", corrMeter.removeFromLeft (40), juce::Justification::centredLeft, false);
    g.drawText ("0", juce::Rectangle<int> (centreX - 10, markerY - markerHeight / 2, 20, markerHeight),
                juce::Justification::centred, false);
    g.drawText ("+1", corrMeter.removeFromRight (40), juce::Justification::centredRight, false);
    g.drawText (juce::String::formatted ("Current: %.2f", corrClamped), corrBox.removeFromBottom (24),
                juce::Justification::centred, false);
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
    const float newLeftRms   = audioProcessor.getRmsLevel (0);
    const float newRightRms  = audioProcessor.getRmsLevel (1);
    const float newLeftPeak  = audioProcessor.getPeakLevel (0);
    const float newRightPeak = audioProcessor.getPeakLevel (1);
    const float newCorrelation = audioProcessor.getCorrelation();

    auto smooth = [] (float current, float target, float attack, float release)
    {
        if (target > current)
            return attack * target + (1.0f - attack) * current;
        return release * target + (1.0f - release) * current;
    };

    leftRms  = juce::jlimit (0.0f, 1.2f, smooth (leftRms,  newLeftRms,  meterAttack, meterRelease));
    rightRms = juce::jlimit (0.0f, 1.2f, smooth (rightRms, newRightRms, meterAttack, meterRelease));

    leftPeak  = juce::jlimit (0.0f, 1.2f, smooth (leftPeak,  newLeftPeak,  meterAttack, meterRelease));
    rightPeak = juce::jlimit (0.0f, 1.2f, smooth (rightPeak, newRightPeak, meterAttack, meterRelease));

    peakHoldLeft  = juce::jmax (newLeftPeak,  peakHoldLeft * peakHoldDecay);
    peakHoldRight = juce::jmax (newRightPeak, peakHoldRight * peakHoldDecay);

    correlation = juce::jlimit (-1.0f, 1.0f,
                                correlation * (1.0f - correlationSmoothing)
                                + newCorrelation * correlationSmoothing);

    repaint();
}
