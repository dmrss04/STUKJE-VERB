cmake_minimum_required(VERSION 3.22)

project(StukjeVerb VERSION 1.0.0)

# ── Point this at your JUCE clone ──────────────────────────────────────────
# Option A: JUCE lives next to this folder  →  ../JUCE
# Option B: set JUCE_DIR when calling cmake:
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(ReverbDistortion
    COMPANY_NAME            "STUKJE"
    PLUGIN_MANUFACTURER_CODE Stkj
    PLUGIN_CODE             Rvdt
    FORMATS                 VST3 AU Standalone
    PRODUCT_NAME            "STUKJE VERB"
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES         "Fx" "Reverb"
    AU_MAIN_TYPE            kAudioUnitType_Effect
)

juce_generate_juce_header(ReverbDistortion)

target_sources(ReverbDistortion
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(ReverbDistortion
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(ReverbDistortion
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
