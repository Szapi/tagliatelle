#include "TagliatelleApp.hpp"

#include <thread>
#include <memory>

namespace
{
    std::unique_ptr<std::jthread> mainMessageThread;
}

namespace tagliatelle
{

void TagliatelleApp::Start_MainMessageLoop()
{
    if (mainMessageThread)
        throw; // TODO throw real exception

    mainMessageThread = std::make_unique<std::jthread>([&](std::stop_token stopToken)
    {
        while(true)
        {
            pendingMessage.wait(false);
            if (stopToken.stop_requested())
                return;

            [[maybe_unused]] Message message = messageQueue([&](MessageQueue& queue)
            {
                Message message = std::move(queue.front());
                queue.pop();
                if (queue.empty())
                    pendingMessage.clear();
                return message;
            });
            // TODO Handle message
        }
    });
}

void TagliatelleApp::Stop_MainMessageLoop()
{
    if (mainMessageThread)
    {
        mainMessageThread->request_stop();
        pendingMessage.test_and_set();
        mainMessageThread->join();
        mainMessageThread.reset();
        pendingMessage.clear();
    }
}

void TagliatelleApp::Push(Message message)
{
    messageQueue([&](MessageQueue& queue)
    {
        queue.emplace(std::move(message));
        pendingMessage.test_and_set();
        pendingMessage.notify_all();
    });
}
    
} // namespace tagliatelle
