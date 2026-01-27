#include <catch2/catch_test_macros.hpp>
#include "mmoore/monkey_moore.hpp"

TEST_CASE("Monkey-Moore core algorithm", "[core]") {
    std::vector<uint8_t> data = {'d', 'd', 'd', 'c', 'c', 'a', 'c', 'a', 't', 'c', 'h', 'a', 'a', 't' };
    std::transform(data.begin(), data.end(), data.begin(), [](uint8_t &i) { return i + 3; });

    SECTION("Basic test") {
        std::vector<CharType> keyword = {'c', 'a', 't', 'c', 'h'};
        MonkeyMoore<uint8_t> searcher(keyword);

        auto results = searcher.search(data.data(), data.size());
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].first == 6);
        REQUIRE(results[0].second['a'] == static_cast<uint8_t>('a' + 3));
    }
}