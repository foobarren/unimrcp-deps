cmake_minimum_required(VERSION 2.6)
project(speech-demo)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib/linux)

add_executable(ttsDemo demo/tts-demo.cpp)
target_link_libraries(ttsDemo realTimeUnity ssl crypto opus pthread)

add_executable(realtimeDemo demo/realtime-demo.cpp)
target_link_libraries(realtimeDemo realTimeUnity ssl crypto opus pthread)

add_executable(asrDemo demo/asr-demo.cpp)
target_link_libraries(asrDemo realTimeUnity ssl crypto opus pthread)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/demo) 

