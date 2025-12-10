cmake_minimum_required(VERSION 3.15)

project(ANIME_ANALYZER VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Fetch JUCE from GitHub (use develop to avoid macOS 15 CGWindowListCreateImage issues)
include(FetchContent)

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG develop
)

FetchContent_MakeAvailable(juce)

juce_add_plugin(ANIME_ANALYZER
    COMPANY_NAME "BRGFUCKERY"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_COPY_DIR "${CMAKE_BINARY_DIR}/plugin_artifacts"
    PLUGIN_MANUFACTURER_CODE BRGF
    PLUGIN_CODE AnAz
    FORMATS VST3 AU
    PRODUCT_NAME "ANIME-ANALYZER"
)

target_sources(ANIME_ANALYZER
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(ANIME_ANALYZER
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ANIME_ANALYZER
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_dsp
        juce::juce_core
)
