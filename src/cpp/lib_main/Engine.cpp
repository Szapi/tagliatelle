#include "Engine.hpp"

#include "Utils.hpp"

namespace tagliatelle
{

std::expected<std::unique_ptr<Engine>, std::string> Engine::CreateAndRun(const EngineArgs& args)
{
    return std::make_unique<Engine>(args);
}

Engine::Engine(const EngineArgs& args) : logger{args.logger}
{
    logger.Info("Engine created");
    this->mainMessageThread = std::make_unique<std::jthread>([&](std::stop_token stopToken) -> void
    {
        while(true)
        {
            pendingMessage.wait(false);
            if (stopToken.stop_requested())
                return;

            PopAndHandleMessage();
        }
    });
}

Engine::~Engine()
{
    PushMessage(tagliatelle::commands::Shutdown{});
    mainMessageThread->join();
    mainMessageThread.reset();
    logger.Info("Engine destroyed");
}

void Engine::PushMessage(Message message)
{
    messageQueue([&](std::queue<Message>& queue)
    {
        queue.emplace(std::move(message));
        pendingMessage.test_and_set();
    });
    pendingMessage.notify_one();
}

void Engine::PopAndHandleMessage()
{
    const auto message = messageQueue([&](std::queue<Message>& queue) -> Message
    {
        Message message = std::move(queue.front());
        queue.pop();
        if (queue.empty())
            pendingMessage.clear();
        return message;
    });

    if (std::holds_alternative<tagliatelle::commands::Shutdown>(message))
    {
        logger.Info("Engine shutdown requested");
        mainMessageThread->request_stop();
        pendingMessage.test_and_set();
        return;
    }

    // TODO handle commands
}
    
} // namespace tagliatelle
