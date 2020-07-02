#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "argument.h"
#include "errors.h"
#include "primitives_module.h"
#include "cdif/cdif.h"
#include "either/either.h"

namespace ap {
    template <typename TArgs>
    class ArgumentParser
    {
        private:
            cdif::Container m_container;
            std::vector<Argument<TArgs>> m_arguments;

            auto get_matching_optional(const std::string& arg) ->
                either<decltype(std::declval<std::vector<Argument<TArgs>>>().begin()), ParsingError>
            {
                auto iter = std::find_if(
                    m_arguments.begin(),
                    m_arguments.end(),
                    [&arg] (const auto& argument)
                    {
                        return argument.is_optional() && std::any_of(
                            argument.switches.begin(),
                            argument.switches.end(),
                            [&arg] (auto& item) { return item == arg; });
                    });

                if (iter == m_arguments.end())
                    return ParsingError::UnknownArgument;

                iter->consumed = true;
                return iter;
            }

            auto get_next_positional() ->
                either<decltype(std::declval<std::vector<Argument<TArgs>>>().begin()), ParsingError>
            {
                auto iter = std::find_if(
                    m_arguments.begin(),
                    m_arguments.end(),
                    [] (const auto& argument)
                    {
                        return !argument.is_optional() && !argument.consumed;
                    });

                if (iter == m_arguments.end())
                    return ParsingError::TooManyPositionalArguments;

                iter->consumed = true;
                return iter;
            }

            auto get_matching_arg(const std::string& argument) ->
                either<decltype(std::declval<std::vector<Argument<TArgs>>>().begin()), ParsingError>
            {
                return get_matching_optional(argument)
                    .foldSecond([&] (auto&&) { return get_next_positional(); });
            }

            void fill_default_values(TArgs& args) const
            {
                using namespace std::string_literals;

                for (const auto& arg : m_arguments)
                    if (!arg.consumed && arg.is_optional())
                        arg.parser(m_container, args, ""s);
            }

            either<TArgs, ParsingError> parse_args(TArgs& args, const char* argv[], int argc)
            {
                for (auto i = 1; i < argc; i++)
                {
                    auto error = get_matching_arg(argv[i]).match(
                        [&] (const auto& argument) -> std::optional<ParsingError>
                        {
                            if (argument->has_argument && i >= argc - 1)
                                return ParsingError::MissingValueForArgument;
                            auto arg = argument->has_argument ? argv[++i] : argv[i];
                            argument->parser(m_container, args, arg);

                            return {};
                        },
                        [] (auto&& error) -> std::optional<ParsingError>
                        {
                            return error;
                        });
                    if (error)
                        return error.value();
                }

                if (!std::all_of(
                        m_arguments.begin(),
                        m_arguments.end(),
                        [] (const auto& pa) {
                            if (!pa.is_optional())
                                return pa.consumed;
                            return true;
                        }))
                    return ParsingError::MissingPositionalArgument;

                fill_default_values(args);

                return args;
            }

            uint32_t get_widest_argument_length() const {
                auto argument_lengths = std::vector<uint32_t>(m_arguments.size());
                std::transform(m_arguments.begin(), m_arguments.end(), argument_lengths.begin(),
                    [] (const auto& arg) { return ap::to_string(arg).size(); });
                std::sort(argument_lengths.begin(), argument_lengths.end());
                return *argument_lengths.rbegin();
            }

        public:
            ArgumentParser(const std::vector<Argument<TArgs>>& arguments)
                : m_container(),
                  m_arguments(arguments)
            {
                m_container.registerModule<PrimitivesModule>();
            }

            ~ArgumentParser() = default;

            ArgumentParser(const ArgumentParser&) = default;
            ArgumentParser(ArgumentParser&&) = default;

            ArgumentParser& operator=(const ArgumentParser&) = default;
            ArgumentParser& operator=(ArgumentParser&&) = default;

            either<TArgs, ParsingError> parse(int argc, const char* argv[])
            {
                TArgs args;
                return parse_args(args, argv, argc);
            }

            std::string usage(const std::string& program_name) const
            {
                std::stringstream out;

                out << "Usage: " << program_name << " ";
                for (auto& item : m_arguments)
                {
                    if (item.is_optional())
                        out << "[" << item << "] ";
                    else
                        out << "<" << item.name << "> ";
                }

                return out.str();
            }

            std::string help(const std::string& program_name) const
            {
                std::stringstream out;

                out << usage(program_name) << std::endl << std::endl;

                auto first_column_width = get_widest_argument_length() + 2;
                auto second_column_width = 120 - first_column_width - 8;

                for (const auto& arg : m_arguments) {
                    out <<
                        std::setw(4) << std::left << " " <<
                        std::setw(first_column_width) << std::left << ap::to_string(arg) <<
                        std::endl << std::setw(first_column_width + 4) << std::left << " " <<
                        std::setw(second_column_width) << std::left << arg.description <<
                        std::endl;
                }

                return out.str();
            }

            std::string get_error_message(const ParsingError& error) const
            {
                using namespace std::string_literals;

                if (error == ParsingError::UnknownArgument)
                    return "Unknown argument provided"s;
                if (error == ParsingError::MissingPositionalArgument)
                    return "Too few positional arguments provided"s;
                if (error == ParsingError::TooManyPositionalArguments)
                    return "Too many positional arguments provided"s;
                if (error == ParsingError::MissingValueForArgument)
                    return "Argument missing required value"s;

                return "Unknown error"s;
            }

            template <typename TModule>
            void register_module()
            {
                m_container.registerModule<TModule>();
            }
    };
}
