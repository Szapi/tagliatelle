#pragma once

#include <functional>
#include <optional>

namespace tagliatelle
{

    template<class Fn, class Ret>
    concept SimpleInvocable = requires (Fn& f)
    {
        std::function<Ret()>{ std::move(f) };
    };

    template<class Result>
    class LazyEvaluator
    {
    public:
        LazyEvaluator(SimpleInvocable<Result> auto&& expr) : expr{std::move(expr)} {}

        // Non-copyable
        LazyEvaluator(const LazyEvaluator&) = delete;
        LazyEvaluator& operator=(const LazyEvaluator&) = delete;
        
        // Maybe movable
        LazyEvaluator(LazyEvaluator&&) = default;
        LazyEvaluator& operator=(LazyEvaluator&&) = default;

        const Result& Get()
        {
            if (!result)
                result = expr();
            return *result;
        }

        const Result& operator()()
        {
            return Get();
        }

    private:
        std::function<Result()> expr;
        std::optional<Result> result{};
    };
    
} // namespace tagliatelle
