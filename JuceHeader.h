#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace juce
{
using String = std::string;

//==============================================================================
template <typename... Args>
inline void ignoreUnused (Args&&...) {}

//==============================================================================
class Colour
{
public:
    constexpr Colour() = default;
    constexpr Colour (float rIn, float gIn, float bIn, float aIn = 1.0f)
        : r (rIn), g (gIn), b (bIn), a (aIn) {}

    Colour withAlpha (float newAlpha) const { return Colour { r, g, b, newAlpha }; }

    float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
};

namespace Colours
{
    static const Colour darkslategrey { 0.18f, 0.31f, 0.31f, 1.0f };
    static const Colour white         { 1.0f, 1.0f, 1.0f, 1.0f };
    static const Colour aqua          { 0.0f, 1.0f, 1.0f, 1.0f };
    static const Colour hotpink       { 1.0f, 0.41f, 0.71f, 1.0f };
}

//==============================================================================
class Rectangle
{
public:
    Rectangle() = default;
    Rectangle (int xIn, int yIn, int wIn, int hIn) : x (xIn), y (yIn), width (wIn), height (hIn) {}

    Rectangle reduced (int amount) const { return { x + amount, y + amount, width - 2 * amount, height - 2 * amount }; }

    Rectangle removeFromTop (int amount)
    {
        Rectangle top (x, y, width, amount);
        y += amount;
        height -= amount;
        return top;
    }

    Rectangle removeFromLeft (int amount)
    {
        Rectangle left (x, y, amount, height);
        x += amount;
        width -= amount;
        return left;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getX() const { return x; }
    int getY() const { return y; }

    Rectangle toFloat() const { return *this; }

private:
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

//==============================================================================
class Justification
{
public:
    Justification() = default;
    static Justification centred;
};

inline Justification Justification::centred {};

//==============================================================================
class Font
{
public:
    enum StyleFlags { plain = 0, bold = 1 };
    Font() = default;
    Font (float, int = plain) {}
};

//==============================================================================
class Graphics
{
public:
    void fillAll (Colour) {}
    void setColour (Colour) {}
    void setFont (const Font&) {}
    void drawText (const String&, const Rectangle&, const Justification&) {}
    void fillRoundedRectangle (const Rectangle&, float) {}
};

//==============================================================================
class ScopedNoDenormals
{};

//==============================================================================
template <typename Type>
Type jlimit (Type lower, Type upper, Type value)
{
    return std::max (lower, std::min (upper, value));
}

//==============================================================================
class AudioChannelSet
{
public:
    AudioChannelSet() = default;
    explicit AudioChannelSet (int ch) : channels (ch) {}

    static AudioChannelSet mono()   { return AudioChannelSet { 1 }; }
    static AudioChannelSet stereo() { return AudioChannelSet { 2 }; }

    int size() const { return channels; }

    bool operator== (const AudioChannelSet& other) const { return channels == other.channels; }
    bool operator!= (const AudioChannelSet& other) const { return ! (*this == other); }

private:
    int channels = 2;
};

//==============================================================================
class BusesLayout
{
public:
    AudioChannelSet getMainOutputChannelSet() const { return output; }
    void setMainOutputChannelSet (AudioChannelSet set) { output = set; }

private:
    AudioChannelSet output { AudioChannelSet::stereo() };
};

class BusesProperties
{
public:
    BusesProperties& withInput (const char*, AudioChannelSet set, bool)
    {
        inputSet = set;
        return *this;
    }

    BusesProperties& withOutput (const char*, AudioChannelSet set, bool)
    {
        outputSet = set;
        return *this;
    }

    AudioChannelSet inputSet  { AudioChannelSet::stereo() };
    AudioChannelSet outputSet { AudioChannelSet::stereo() };
};

//==============================================================================
template <typename SampleType>
class AudioBuffer
{
public:
    AudioBuffer() = default;
    AudioBuffer (int channels, int samples) { setSize (channels, samples); }

    void setSize (int channels, int samples)
    {
        data.resize ((size_t) channels);
        for (auto& ch : data)
            ch.assign ((size_t) samples, SampleType {});
    }

    int getNumChannels() const { return (int) data.size(); }
    int getNumSamples() const { return data.empty() ? 0 : (int) data.front().size(); }

    SampleType* getWritePointer (int channel) { return data[(size_t) channel].data(); }
    const SampleType* getReadPointer (int channel) const { return data[(size_t) channel].data(); }

    void clear (int channel, int startSample, int numSamples)
    {
        auto& ch = data[(size_t) channel];
        const int end = std::min ((int) ch.size(), startSample + numSamples);
        for (int i = startSample; i < end; ++i)
            ch[(size_t) i] = SampleType {};
    }

private:
    std::vector<std::vector<SampleType>> data;
};

class MidiBuffer {};

class MemoryBlock : public std::vector<char>
{
public:
    using std::vector<char>::vector;
};

//==============================================================================
class AudioProcessorEditor;

class AudioProcessor
{
public:
    using BusesLayout = juce::BusesLayout;

    explicit AudioProcessor (const BusesProperties& props)
        : inputChannels (props.inputSet.size()), outputChannels (props.outputSet.size()) {}
    virtual ~AudioProcessor() = default;

    virtual void prepareToPlay (double sampleRate, int samplesPerBlock) = 0;
    virtual void releaseResources() = 0;
    virtual bool isBusesLayoutSupported (const BusesLayout& layouts) const = 0;
    virtual void processBlock (AudioBuffer<float>&, MidiBuffer&) = 0;
    virtual AudioProcessorEditor* createEditor() = 0;
    virtual bool hasEditor() const { return true; }

    virtual const String getName() const = 0;
    virtual bool acceptsMidi() const = 0;
    virtual bool producesMidi() const = 0;
    virtual bool isMidiEffect() const = 0;
    virtual double getTailLengthSeconds() const = 0;

    virtual int getNumPrograms() = 0;
    virtual int getCurrentProgram() = 0;
    virtual void setCurrentProgram (int index) = 0;
    virtual const String getProgramName (int index) = 0;
    virtual void changeProgramName (int index, const String& newName) = 0;

    virtual void getStateInformation (MemoryBlock& destData) = 0;
    virtual void setStateInformation (const void* data, int sizeInBytes) = 0;

    int getTotalNumInputChannels() const { return inputChannels; }
    int getTotalNumOutputChannels() const { return outputChannels; }

private:
    int inputChannels = 2;
    int outputChannels = 2;
};

//==============================================================================
class AudioProcessorEditor
{
public:
    explicit AudioProcessorEditor (AudioProcessor*) {}
    virtual ~AudioProcessorEditor() = default;

    virtual void paint (Graphics&) {}
    virtual void resized() {}

    void setOpaque (bool) {}
    void setSize (int, int) {}
    Rectangle getLocalBounds() const { return Rectangle { 0, 0, 600, 400 }; }
    void repaint() {}
};

//==============================================================================
class Timer
{
public:
    virtual ~Timer() = default;
    virtual void timerCallback() {}
    void startTimerHz (int) {}
    void stopTimer() {}
};

//==============================================================================
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassName) \
public:                                                          \
    ClassName (const ClassName&) = delete;                       \
    ClassName& operator= (const ClassName&) = delete;

} // namespace juce

using juce::Rectangle;

