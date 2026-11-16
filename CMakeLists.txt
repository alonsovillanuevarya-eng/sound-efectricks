cmake_minimum_required(VERSION 3.22)
project(SoundEfectricks VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SoundEfectricks
    COMPANY_NAME "Sound Era"
    BUNDLE_ID "com.soundera.soundefectricks"
    PLUGIN_MANUFACTURER_CODE Sndr
    PLUGIN_CODE Sefx
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "SOUND EFECTRICKS"
    DESCRIPTION "Creative step sequencer and granular effects"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(SoundEfectricks)

target_sources(SoundEfectricks PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/CrystalEngine.cpp
)

target_compile_definitions(SoundEfectricks PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SoundEfectricks PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
