cmake_minimum_required(VERSION 3.22)
project(RETRICA_COMPRESSOR VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Retrica
    COMPANY_NAME "RetricaAudio"
    PLUGIN_MANUFACTURER_CODE Retr
    PLUGIN_CODE Rtrc
    FORMATS VST3 Standalone
    PRODUCT_NAME "Retrica VCA Compressor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(Retrica PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)

target_compile_definitions(Retrica PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Retrica PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

juce_generate_juce_header(Retrica)
