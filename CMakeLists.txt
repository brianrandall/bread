cmake_minimum_required(VERSION 3.22)
project(Bread VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
option(BREAD_ENGINE_ONLY "Only build dependency-free circuit tests" OFF)
add_executable(bread_engine_test tests/engine_test.cpp)
enable_testing()
add_test(NAME circuit_engine COMMAND bread_engine_test)
if(BREAD_ENGINE_ONLY)
 return()
endif()
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 51a8a6d7aeae7326956d747737ccf1575e61e209 GIT_SHALLOW FALSE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(Bread
 COMPANY_NAME "Bread Audio" BUNDLE_ID "audio.bread.workbench"
 PLUGIN_MANUFACTURER_CODE Brda PLUGIN_CODE Brd1
 FORMATS VST3 AU Standalone PRODUCT_NAME "BREAD"
 IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
 COPY_PLUGIN_AFTER_BUILD FALSE
 MICROPHONE_PERMISSION_ENABLED TRUE MICROPHONE_PERMISSION_TEXT "BREAD processes live audio through your circuit.")
juce_generate_juce_header(Bread)
target_sources(Bread PRIVATE Source/Plugin.cpp Source/Editor.cpp)
target_compile_definitions(Bread PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Bread PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)
