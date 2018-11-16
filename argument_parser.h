#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "argument.h"
#include "primitives_module.h"
#include "cdif/cdif.h"

namespace ap {
    template <typename TArgs>
    class ArgumentParser
    {
        private:
            cdif::Container m_container;
            std::vector<Argument<TArgs>> m_optional_args;
            std::vector<Argument<TArgs>> m_positional_args;

            std::optional<Argument<TArgs>> get_matching_optional(const std::string& arg)
            {
                auto iter = std::find_if(
                    m_optional_args.begin(),
                    m_optional_args.end(),
                    [&arg] (const auto& argument)
                    {
                        return std::any_of(
                            argument.switches.begin(),
                            argument.switches.end(),
                            [&arg] (auto& item) { return item == arg; });
                    });

                if (iter == m_optional_args.end())
                    return {};

                iter->consumed = true;
                return *iter;
            }

            std::optional<TArgs> parse_optional_args(TArgs& args, const char* argv[], int argc)
            {
                std::size_t consumed_positional_args = 0;

                for (auto i = 1; i < argc; i++)
                {
                    auto optional_arg = get_matching_optional(argv[i]);

                    if (optional_arg)
                    {
                        if (optional_arg.value().has_argument && i >= argc - 1)
                            return {};
                        auto arg = optional_arg.value().has_argument ? argv[++i] : argv[i];
                        optional_arg.value().parser(m_container, args, arg);
                    } else {
                        if (consumed_positional_args >= m_positional_args.size())
                            return {};
                        m_positional_args[consumed_positional_args++]
                            .parser(m_container, args, argv[i]);
                    }
                }

                if (consumed_positional_args != m_positional_args.size())
                    return {};

                return args;
            }

            std::optional<TArgs> parse_positional_args(TArgs& args)
            {
                for (auto& item : m_optional_args)
                {
                    if (!item.consumed)
                        item.parser(m_container, args, "");
                }

                return args;
            }

        public:
            ArgumentParser(const std::vector<Argument<TArgs>>& optional_args, const std::vector<Argument<TArgs>>& positional_args)
                : m_container(),
                  m_optional_args(optional_args),
                  m_positional_args(positional_args)
            {
                m_container.registerModule<PrimitivesModule>();
            }

            ~ArgumentParser() = default;

            ArgumentParser(const ArgumentParser&) = default;
            ArgumentParser(ArgumentParser&&) = default;

            ArgumentParser& operator=(const ArgumentParser&) = default;
            ArgumentParser& operator=(ArgumentParser&&) = default;

            std::optional<TArgs> parse(int argc, const char* argv[])
            {
                TArgs args;
                auto parsedOptionals = parse_optional_args(args, argv, argc);
                if (!parsedOptionals)
                    return {};
                return parse_positional_args(args);
            }

            std::string usage(const std::string& program_name) const
            {
                std::stringstream out;

                out << "Usage: " << program_name << " ";
                for (auto& item : m_optional_args)
                    out << "[" << item << "] ";

                for (auto& item: m_positional_args)
                    out << "<" << item.name << "> ";

                return out.str();
            }
    };
}
