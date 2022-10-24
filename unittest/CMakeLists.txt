include(FetchContent)

enable_testing()
include(CTest)

######### Google Test ###########
set(GIT_REPO_URL "https://github.com" CACHE STRING "git repository url")
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://gitclone.com/github.com/google/googletest.git
        GIT_TAG release-1.11.0
        GIT_SHALLOW TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(install_gtest OFF)
set(install_gmock OFF)

FetchContent_MakeAvailable(googletest)

#################################
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message("build test")
    # Add test executables
    file(GLOB_RECURSE TEST_SOURCES "${PROJECT_SOURCE_DIR}/unittest/*test.cpp")

    foreach (test_source ${TEST_SOURCES})
        # Create a human readable name.
        get_filename_component(test_filename ${test_source} NAME)
        string(REPLACE ".cpp" "" test_name ${test_filename})
				message(${test_name})
				message(${PROJECT_SOURCE_DIR})
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} gtest_main ast-interpreter-lib)
        target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/include)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach (test_source ${TEST_SOURCES})
endif (${CMAKE_BUILD_TYPE} MATCHES "Debug")