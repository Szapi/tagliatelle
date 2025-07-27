#pragma once

#include <string>
#include <unordered_set>

namespace tagliatelle::commands
{

    struct ImportEventsFromFiles
    {
        std::unordered_set<std::string> filePaths;
    };

    struct ClearEvents {};
    
} // namespace tagliatelle
