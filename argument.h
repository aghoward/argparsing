#pragma once

#include <string>
#include <vector>
#include <functional>

#include "cdif/cdif.h"

namespace ap {
    template <typename TArgs>
    struct Argument
    {
        std::string name;
        std::vector<std::string> switches;
        std::string description;
        bool has_argument;
        std::function<void (const cdif::Container&, TArgs&, const std::string&)> parser;

        bool consumed;

        Argument(
            const std::string& _name,
            const std::vector<std::string>& _switches,
            const std::string& _description,
            bool _has_argument,
            std::function<void (const cdif::Container&, TArgs&, const std::string&)> _parser)
            :
                name(_name),
                switches(_switches),
                description(_description),
                has_argument(_has_argument),
                parser(_parser),
                consumed(false)
        {
        }
    };

    template <typename TIter>
    std::ostream& join(std::ostream& out, TIter start, TIter end, const std::string& delim)
    {
        auto val = std::string();
        for (auto& iter = start; iter != end-1; iter++)
            out << *iter << delim;
        out << *(end - 1);
        return out;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& out, const Argument<T>& arg)
    {
        if (!arg.switches.empty())
            join(out, arg.switches.begin(), arg.switches.end(), "|");

        if (arg.has_argument)
            out << " <" << arg.name << ">";
        return out;
    }
}
