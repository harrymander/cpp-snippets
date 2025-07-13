# A header-only C++ wrapper for FreeRTOS

add_library(freertos++ INTERFACE)
target_compile_features(freertos++ INTERFACE cxx_std_23)
target_include_directories(freertos++ INTERFACE include)
