cmake_minimum_required(VERSION 3.22)
project(DJOGuitarStudio VERSION 1.0.0)

# JUCE is expected as a sibling folder or supplied with -DJUCE_DIR=/path/to/JUCE
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
endif()
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(DJOGuitarStudio
    COMPANY_NAME "Independent Audio Tools"
    BUNDLE_ID "com.independentaudio.djoguitarstudio"
    PLUGIN_MANUFACTURER_CODE Idat
    PLUGIN_CODE Djgs
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "DJO Guitar Studio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(DJOGuitarStudio)
target_sources(DJOGuitarStudio PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_compile_features(DJOGuitarStudio PRIVATE cxx_std_17)
target_link_libraries(DJOGuitarStudio PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
