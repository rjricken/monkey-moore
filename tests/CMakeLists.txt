find_package(Catch2 3 REQUIRED)

add_executable(unit-tests 
    test_text_utils.cpp
    test_monkey_moore.cpp 
    test_search_engine.cpp)

target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain monkey-core)

include(Catch)
catch_discover_tests(unit-tests)