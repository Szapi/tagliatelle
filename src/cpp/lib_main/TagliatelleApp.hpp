#pragma once

#include <atomic>
#include <variant>
#include <queue>

#include "Utils.hpp"
#include "MutexGuarded.hpp"
#include "MainCommands.hpp"


namespace tagliatelle
{
    class TagliatelleApp
    {
        SINGLETON(TagliatelleApp);

    public:
        void Start_MainMessageLoop();
        void Stop_MainMessageLoop();

        using Message = std::variant<
            tagliatelle::commands::ImportEventsFromFiles,
            tagliatelle::commands::ClearEvents
        >;
        using MessageQueue = std::queue<Message>;
        
        void Push(Message command);
        
    private:
        MutexGuarded<MessageQueue> messageQueue{};
        std::atomic_flag pendingMessage;
    };
    
} // namespace tagliatelle
