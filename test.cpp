#include <iostream>
#include <logger.hpp>

using namespace markusjx::logging;

int main() {
    StaticLogger::create(MODE_CONSOLE, DEBUG, SYNC);
    StaticLogger::debug("Hello there");
    StaticLogger::debugf("Hello %s, id: %d", "abc", 1);
    StaticLogger::debugStream << "Abc";

    StaticLogger::warning("Some warning");
    StaticLogger::warningf("Warning test %s, %d", "Warningf", 1234);
    StaticLogger::warningStream << "Warning stream test";

    StaticLogger::error("Some error");
    StaticLogger::error("Exception thrown:", std::runtime_error("some error"));
    StaticLogger::errorf("Error test: %s", "err");
    StaticLogger::errorStream << "Error" << ' ' << "stream" << ' ' << "test";

    StaticLogger::create(MODE_CONSOLE, WARNING, SYNC);
    StaticLogger::debug("Should not be displayed");
    StaticLogger::warning("Should be displayed");
    StaticLogger::error("Should be displayed");

    StaticLogger::create(MODE_CONSOLE, ERROR, SYNC);
    StaticLogger::debug("Should not be displayed");
    StaticLogger::warning("Should not be displayed");
    StaticLogger::error("Should be displayed");

    StaticLogger::create(MODE_CONSOLE, NONE, SYNC);
    StaticLogger::debug("Should not be displayed");
    StaticLogger::warning("Should not be displayed");
    StaticLogger::error("Should not be displayed");

    StaticLogger::create(MODE_CONSOLE, DEBUG, DEFAULT);
    StaticLogger::debug("Sync mode");
    StaticLogger::warning("Sync mode");
    StaticLogger::error("Sync mode");

    StaticLogger::create(MODE_CONSOLE, DEBUG, ASYNC);
    StaticLogger::debug("Async mode");
    StaticLogger::warning("Async mode");
    StaticLogger::error("Async mode");

    return 0;
}
