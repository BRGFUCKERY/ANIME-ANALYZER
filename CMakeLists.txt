cmake_minimum_required(VERSION 3.15)

project(ANIME-ANALYZER VERSION 1.0.0)

# JUCE
set(CMAKE_CXX_STANDARD 17)

# Minimal JUCE-style stubs live in this repository, so expose them as
# interface libraries for the targets below.
function(juce_add_plugin targetName)
    add_library(${targetName} SHARED)
    target_include_directories(${targetName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

foreach(moduleName IN ITEMS juce_audio_utils juce_audio_processors juce_audio_basics juce_audio_formats juce_graphics juce_gui_basics juce_dsp juce_core)
    if (NOT TARGET juce::${moduleName})
        add_library(juce::${moduleName} INTERFACE IMPORTED)
        target_include_directories(juce::${moduleName} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endforeach()

# ============================================================
#  Plugin definition
# ============================================================
juce_add_plugin(ANIME-ANALYZER
    COMPANY_NAME "GOREKLIP"
    PLUGIN_MANUFACTURER_CODE GORL
    PLUGIN_CODE AnAz
    PLUGIN_NAME "ANIME-ANALYZER"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # Which formats to build
    FORMATS VST3 AU Standalone

    PRODUCT_NAME "ANIME-ANALYZER"
)

# ============================================================
#  Plugin Sources
# ============================================================
target_sources(ANIME-ANALYZER PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# ============================================================
#  Linking
# ============================================================
target_link_libraries(ANIME-ANALYZER PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_dsp
    juce::juce_core
)

# Enable VST3
# Ensure VST3 builds do not attempt to replace VST2 plugins
# when older binaries are present.
target_compile_definitions(ANIME-ANALYZER PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
)
