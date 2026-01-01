#include "Engine.hpp"

#include "LoggingUtils.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

int main ()
{
    using namespace tagliatelle;

    std::ofstream logFile{"N:\\Kukaba_vele\\test.log"};
    auto logNode = logging::CreateSynchronizedNode(std::cout, logFile);
    logging::Endpoint endpoint{
        [&](auto level, auto&& getMsg)
        {
            std::array<char, 64> buf;
            const auto header = logging::MakeStandardLogEntryHeader(level, buf);
            logNode(header, getMsg(), '\n');
        }
    };

    EngineArgs args { .logger = endpoint };
    auto engine = Engine::CreateAndRun(args);

    std::this_thread::sleep_for(std::chrono::seconds{3});
}