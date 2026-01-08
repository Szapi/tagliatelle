#include <chrono>
#include <stdint.h>
#include <type_traits>

namespace
{

    static_assert(std::is_same_v<std::size_t, uint64_t>);

} // namespace
