include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(Catch2)

add_executable(unit-tests 
    test_monkey_moore.cpp 
    test_search_engine.cpp)

target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain monkey-core)

include(Catch)
catch_discover_tests(unit-tests)