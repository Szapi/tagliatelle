#pragma once

#include <mutex>
#include <type_traits>

namespace tagliatelle
{

    template<class T, class Mutex = std::mutex>
        requires (std::is_same_v<T, std::remove_reference_t<T>>)
    class MutexGuarded
    {
    public:
        template<class...Args>
        MutexGuarded(Args&&... args) : guarded{std::forward<Args>(args)...} {}

        template<class Op>
        decltype(auto) operator()(Op&& op)
        {
            std::lock_guard lock{mutex};
            return op(guarded);
        }
        
        template<class Op>
        decltype(auto) operator()(Op&& op) const
        {
            std::lock_guard lock{mutex};
            return op(guarded);
        }

    private:
        Mutex mutex{};
        T guarded;
    };
    
} // namespace tagliatelle
