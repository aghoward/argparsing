#pragma once

#include <functional>
#include <string>
#include <optional>
#include <vector>

#include "argument.h"
#include "argument_parser.h"
#include "cdif/cdif.h"

namespace ap {
    template <typename TArgs>
    class ArgumentParserBuilder
    {
        private:
            std::vector<Argument<TArgs>> m_optional_args;
            std::vector<Argument<TArgs>> m_positional_args;

            template <typename TMember>
            void add_parameter(
                std::vector<Argument<TArgs>>& arg_list,
                const std::string& name,
                const std::vector<std::string>& switches,
                const std::string& description,
                bool hasArgument,
                TMember TArgs::*member,
                TMember defaultValue)
            {
                arg_list.emplace_back(
                    name,
                    switches,
                    description,
                    hasArgument,
                    [defaultValue, member]
                    (const cdif::Container& ctx, TArgs& args, const std::string& input)
                    {
                        if constexpr (std::is_same_v<TMember, bool>)
                            args.*member = input.empty() ? defaultValue : !defaultValue;
                        else
                        {
                            auto conversion = ctx.resolve<std::function<TMember (std::string)>>();
                            args.*member = input.empty() ? defaultValue : conversion(input);
                        }
                    });
            }


        public:
            ArgumentParserBuilder()
                : m_optional_args(),
                  m_positional_args()
            {
            }

            template <typename TMember>
            ArgumentParserBuilder<TArgs>& add_optional(
                const std::string& name,
                TMember TArgs::*member,
                TMember defaultValue,
                const std::vector<std::string>& switches,
                const std::string& description)
            {
                add_parameter(
                    m_optional_args,
                    name,
                    switches,
                    description,
                    !std::is_same_v<TMember, bool>,
                    member,
                    defaultValue);

                return *this;
            }

            template <typename TMember>
            ArgumentParserBuilder<TArgs>& add_positional(
                const std::string& name,
                TMember TArgs::*member,
                TMember defaultValue,
                const std::string& description)
            {
                add_parameter(
                    m_positional_args,
                    name,
                    {},
                    description,
                    false,
                    member,
                    defaultValue);

                return *this;
            }

            ArgumentParser<TArgs> build() const
            {
                return ArgumentParser<TArgs>(m_optional_args, m_positional_args);
            }
    };
}
