#pragma once

#include "ClassUtils.hpp"
#include "FormatUtils.hpp"
#include "LazyEvaluator.hpp"

namespace tagliatelle::logging
{
    enum Level
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    template<class E>
    concept DelayedLogExpression = requires(E e)
    {
        { e() } -> std::same_as<std::string>;
    };

    // A consumer-facing (consumer ~ code that wants to leave logs), smart interface for
    // efficient log message formatting and forwarding to actual logger backends.
    class Endpoint
    {
    public:
        using LogSink = std::function<void(Level, std::function<std::string_view()>)>;

        inline Endpoint(LogSink impl) : impl{ std::move(impl) } {}
        MOVE_ONLY(Endpoint);

        inline void Log(Level level, std::string_view msg)
        {
            auto forwarder = [=]() -> std::string_view { return msg; };
            impl(level, std::ref(forwarder));
        }

        // Convenience methods for simple logging:
        inline void Debug  (std::string_view msg) { Log(Level::Debug,   msg); }
        inline void Info   (std::string_view msg) { Log(Level::Info,    msg); }
        inline void Warning(std::string_view msg) { Log(Level::Warning, msg); }
        inline void Error  (std::string_view msg) { Log(Level::Error,   msg); }
        inline void Fatal  (std::string_view msg) { Log(Level::Fatal,   msg); }
        
        // Allows the logger backend to delay and potentially skip the evaluation of the expression.
        // Example usage:
        // endpoint.Log(Debug, [&]{ *Expensive calculations*; return *string*; });
        template<DelayedLogExpression E>
        void Log(Level level, E&& expr)
        {
            std::string msg;
            LazyEvaluator<std::string_view> lazyEvaluator = [&]() -> std::string_view
            {
                msg = expr();
                return msg;
            };
            impl(level, std::ref(lazyEvaluator));
        }

        // Convenience methods for delayed log expressions
        void Debug  (DelayedLogExpression auto&& expr) { Log(Level::Debug,   expr); }
        void Info   (DelayedLogExpression auto&& expr) { Log(Level::Info,    expr); }
        void Warning(DelayedLogExpression auto&& expr) { Log(Level::Warning, expr); }
        void Error  (DelayedLogExpression auto&& expr) { Log(Level::Error,   expr); }
        void Fatal  (DelayedLogExpression auto&& expr) { Log(Level::Fatal,   expr); }

    private:
        LogSink impl;
    };

} // namespace tagliatelle::logging
