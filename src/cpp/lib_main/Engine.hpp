#pragma once

#include <atomic>
#include <expected>
#include <memory>
#include <queue>
#include <thread>
#include <variant>

#include "MainCommands.hpp"
#include "MutexGuarded.hpp"
#include "LoggingEndpoint.hpp"

namespace tagliatelle
{

    struct EngineArgs
    {
        logging::Endpoint& logger;
    };

    // This class is the main entry point for the backend logic.
    // It runs the main message loop to perform commands for the frontend.
    class Engine
    {
    public:
        static std::expected<std::unique_ptr<Engine>, std::string> CreateAndRun(const EngineArgs&);

        Engine(const EngineArgs&); // Use the factory method to instantiate!
        ~Engine();
        IMMOVABLE(Engine);

        using Message = std::variant<
            tagliatelle::commands::ImportEventsFromFiles,
            tagliatelle::commands::ClearEventPool,
            tagliatelle::commands::Shutdown
        >;

        void PushMessage(Message message);

    private:
        

        void PopAndHandleMessage();

    private:
        MutexGuarded<std::queue<Message>> messageQueue{};
        std::atomic_flag pendingMessage;
        std::unique_ptr<std::jthread> mainMessageThread;
        logging::Endpoint& logger;
    };

} // namespace tagliatelle
