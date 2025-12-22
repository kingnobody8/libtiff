# CMakeLists.txt for building libtiff fuzzers
# This allows building fuzzers as part of the normal cmake build process

cmake_minimum_required(VERSION 3.10)

# Only build fuzzers if explicitly requested or if building with sanitizers
option(BUILD_FUZZERS "Build fuzzing targets" OFF)
option(FUZZER_STANDALONE "Build fuzzers as standalone executables for testing" OFF)

if(NOT BUILD_FUZZERS)
    return()
endif()

# List of all fuzzers
set(FUZZERS
    tiff_read_rgba_fuzzer
    tiff_read_strip_fuzzer
    tiff_read_dir_fuzzer
    tiff_print_dir_fuzzer
    tiff_color_fuzzer
    tiff_write_read_fuzzer
    tiff_codec_fuzzer
    tiff_bigtiff_fuzzer
    tiff_multipage_fuzzer
    tiff_custom_tag_fuzzer
    tiff_ojpeg_fuzzer
    tiff_predictor_fuzzer
    tiff_open_options_fuzzer
)

# Fuzzer engine - use libFuzzer by default, or allow custom engine
if(NOT DEFINED LIB_FUZZING_ENGINE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(LIB_FUZZING_ENGINE "-fsanitize=fuzzer")
    else()
        message(WARNING "LIB_FUZZING_ENGINE not set and not using Clang. Fuzzers may not build correctly.")
        set(LIB_FUZZING_ENGINE "")
    endif()
endif()

foreach(fuzzer ${FUZZERS})
    add_executable(${fuzzer} ${fuzzer}.cc)

    target_include_directories(${fuzzer} PRIVATE
        ${CMAKE_SOURCE_DIR}/libtiff
        ${CMAKE_BINARY_DIR}/libtiff
    )

    target_link_libraries(${fuzzer} PRIVATE
        tiffxx
        tiff
    )

    if(FUZZER_STANDALONE)
        # Build as standalone executable for testing
        target_compile_definitions(${fuzzer} PRIVATE STANDALONE)
    else()
        # Link with fuzzing engine
        target_link_options(${fuzzer} PRIVATE ${LIB_FUZZING_ENGINE})
        target_compile_options(${fuzzer} PRIVATE ${LIB_FUZZING_ENGINE})
    endif()

    set_target_properties(${fuzzer} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
    )

    # Install fuzzers to bin directory
    install(TARGETS ${fuzzer} DESTINATION bin)

    # Install options files if they exist
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${fuzzer}.options")
        install(FILES ${fuzzer}.options DESTINATION bin)
    endif()
endforeach()

# Install dictionary and corpus
install(FILES tiff.dict DESTINATION bin)
install(DIRECTORY corpus/ DESTINATION share/libtiff-fuzz-corpus
        FILES_MATCHING PATTERN "*.tiff" PATTERN "*.tif")

# Add a custom target to run all fuzzers briefly (for CI testing)
add_custom_target(run_fuzzers_quick
    COMMENT "Running fuzzers for quick smoke test"
)

foreach(fuzzer ${FUZZERS})
    add_custom_command(TARGET run_fuzzers_quick POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Testing ${fuzzer}..."
        COMMAND $<TARGET_FILE:${fuzzer}> -max_total_time=10 -max_len=65536 ${CMAKE_CURRENT_SOURCE_DIR}/corpus/ || true
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

message(STATUS "Fuzzer targets configured: ${FUZZERS}")
