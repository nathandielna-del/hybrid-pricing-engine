# Télécharge et compile GoogleTest automatiquement
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Exécutable de tests, lié à notre librairie et à GoogleTest
add_executable(pricer_tests test_Payoff.cpp test_NormalDistribution.cpp test_BlackScholes.cpp)
target_compile_options(pricer_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(pricer_tests PRIVATE pricer GTest::gtest_main)

# Enregistre chaque test auprès de CTest
include(GoogleTest)
gtest_discover_tests(pricer_tests)