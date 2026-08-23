#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <skoll/feed/replay.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("replay preserves message order") {
    const std::string path = "replay-order-test.jsonl";

    { // scope
        std::ofstream file_writer(path);
        file_writer << "first\n";
        file_writer << "second\n";
        file_writer << "third\n";
    } // here the writer goes out of scope and it's destructor is called

    std::vector<std::string> messages;

    skoll::feed::Replay replay(
        path,
        [&messages](const std::string &message) {
            messages.push_back(message);
        }
    );

    replay.run();

    REQUIRE(messages.size() == 3);
    CHECK(messages[0] == "first");
    CHECK(messages[1] == "second");
    CHECK(messages[2] == "third");

    std::filesystem::remove(path);
}

TEST_CASE("replay skips empty lines") {
    const std::string path = "replay-empty-lines-test.jsonl";

    {
        std::ofstream file_writer(path);
        file_writer << "first\n";
        file_writer << '\n';
        file_writer << "second\n";
    }

    std::vector<std::string> messages;

    skoll::feed::Replay replay(
        path,
        [&messages](const std::string &message) {
            messages.push_back(message);
        }
    );

    replay.run();

    REQUIRE(messages.size() == 2);
    CHECK(messages[0] == "first");
    CHECK(messages[1] == "second");

    std::filesystem::remove(path);
}

TEST_CASE("replay throws when file does not exist") {
    skoll::feed::Replay replay(
        "nonexistent.jsonl",
        [](const std::string &) {
            // do nothing
        }
    );

    REQUIRE_THROWS_AS(replay.run(), std::runtime_error);
}