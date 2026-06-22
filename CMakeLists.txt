cmake_minimum_required(VERSION 3.22)
project(Ratosaurus VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.13
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Ratosaurus
    COMPANY_NAME "Ratosaurus Audio"
    BUNDLE_ID "com.ratosaurusaudio.ratosaurus"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Rato
    PLUGIN_CODE Rats
    FORMATS VST3 Standalone
    PRODUCT_NAME "Ratosaurus"
    VST3_CATEGORIES Dynamics Distortion
)

target_sources(Ratosaurus
    PRIVATE
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp
        Source/DSP/EnvelopeDetector.h
        Source/DSP/PeakDetector.h
        Source/DSP/RMSDetector.h
        Source/DSP/GainComputer.h
        Source/DSP/CompressorEngine.h
        Source/DSP/CompressorEngine.cpp
        Source/DSP/CharacterEngine.h
        Source/DSP/CharacterEngine.cpp
        Source/DSP/HarmonicsEngine.h
        Source/DSP/HarmonicsEngine.cpp
        Source/DSP/OversamplingEngine.h
        Source/DSP/OversamplingEngine.cpp
        Source/DSP/DCBlocker.h
        Source/DSP/LevelMeter.h
        Source/DSP/VUMeterBallistics.h
        Source/Presets.h
        # --- Stage 10A: Visual Identity infrastructure ---
        Source/UI/RatosaurusTheme.h
        Source/UI/RatosaurusLookAndFeel.h
        Source/UI/RatosaurusLookAndFeel.cpp
        Source/UI/RatoKnob.h
        Source/UI/RatoComboBox.h
        Source/UI/RatoMeter.h
        Source/UI/RatoMeter.cpp
        Source/UI/CalibratedVUMeter.h
        Source/UI/CalibratedVUMeter.cpp
        Source/UI/RatoLedSelector.h
        Source/UI/RatoLedSelector.cpp
        Source/UI/RatoSatTiles.h
        Source/UI/RatoSatTiles.cpp
        Source/UI/RatoFonts.h
        Source/UI/RatoFonts.cpp
        Source/UI/AssetCache.h
        Source/UI/AssetCache.cpp
)

# --- Stage 10C: SVG assets bundled as BinaryData -----------------------------
# Placeholder geometric SVGs only. Real art lands later.
# Generated header: BinaryData.h with BinaryData::<name>_svg + _svgSize symbols.
juce_add_binary_data(RatosaurusAssets
    SOURCES
        Assets/logo.svg
        Assets/rato_logo.svg
        Assets/signature_shchur.svg
        Assets/icons/off.svg
        Assets/icons/tape.svg
        Assets/icons/tube.svg
        Assets/icons/fet.svg
        Assets/icons/rat.svg
        Assets/fonts/OswaldVariable.ttf
        Assets/fonts/PirataOne.ttf
        Assets/fonts/ShareTechMono.ttf
        Assets/faceplate_v4.svg
        Assets/knob_base_v4.svg
        Assets/knob_rotor_v4.svg
)
# -----------------------------------------------------------------------------

target_compile_definitions(Ratosaurus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Ratosaurus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        RatosaurusAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
