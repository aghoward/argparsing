#pragma once

#include <functional>
#include <string>
#include <optional>
#include <vector>

#include "argparsing/argument.h"
#include "argparsing/argument_parser.h"
#include "cdif/cdif.h"

namespace ap {
    namespace impl {
        template <typename T>
        std::function<T(std::string)> default_conversion_factory(const cdif::Container& ctx)
        {
            return ctx.resolve<std::function<T(std::string)>>();
        }
    }

    template <typename TArgs>
    class ArgumentParserBuilder
    {
        private:
            std::vector<Argument<TArgs>> m_args;

            template <typename TMember, typename TFactory>
            auto add_parameter(
                const std::string& name,
                const std::vector<std::string>& switches,
                const std::string& description,
                bool hasArgument,
                TMember TArgs::*member,
                TMember defaultValue,
                TFactory&& conversion_factory,
                bool freestanding)
            -> std::enable_if_t<std::is_convertible_v<
                decltype(std::declval<TFactory>()(std::declval<cdif::Container>())(std::declval<std::string>())),
                TMember>>
            {
                m_args.emplace_back(
                    name,
                    switches,
                    description,
                    hasArgument,
                    [defaultValue, member, conversion_factory]
                    (const cdif::Container& ctx, TArgs& args, const std::string& input)
                    {
                        if constexpr (std::is_same_v<TMember, bool>)
                            args.*member = input.empty() ? defaultValue : !defaultValue;
                        else
                        {
                            auto conversion = conversion_factory(ctx);
                            args.*member = input.empty() ? defaultValue : conversion(input);
                        }
                    },
                    freestanding);
            }


        public:
            ArgumentParserBuilder()
                : m_args()
            {
            }

            template <typename TMember, typename TFactory>
            auto add_optional(
                const std::string& name,
                TMember TArgs::*member,
                TMember defaultValue,
                const std::vector<std::string>& switches,
                const std::string& description,
                TFactory&& conversion_factory,
                bool freestanding = false)
            -> std::enable_if_t<std::is_convertible_v<
                decltype(std::declval<TFactory>()(std::declval<cdif::Container>())(std::declval<std::string>())),
                TMember>,
            ArgumentParserBuilder<TArgs>&>
            {
                add_parameter(
                    name,
                    switches,
                    description,
                    !std::is_same_v<TMember, bool>,
                    member,
                    defaultValue,
                    conversion_factory,
                    freestanding);

                return *this;
            }

            template <typename TMember>
            ArgumentParserBuilder<TArgs>& add_optional(
                const std::string& name,
                TMember TArgs::*member,
                TMember defaultValue,
                const std::vector<std::string>& switches,
                const std::string& description,
                bool freestanding = false)
            {
                return add_optional(name, member, defaultValue, switches, description, impl::default_conversion_factory<TMember>, freestanding);
            }

            template <typename TMember, typename TFactory>
            auto add_positional(
                const std::string& name,
                TMember TArgs::*member,
                TMember defaultValue,
                const std::string& description,
                TFactory&& conversion_factory)
            -> std::enable_if_t<std::is_convertible_v<
                decltype(std::declval<TFactory>()(std::declval<cdif::Container>())(std::declval<std::string>())),
                TMember>,
            ArgumentParserBuilder<TArgs>&>
            {
                add_parameter(
                    name,
                    {},
                    description,
                    false,
                    member,
                    defaultValue,
                    conversion_factory,
                    false);

                return *this;
            }

            template <typename TMember>
            ArgumentParserBuilder<TArgs>& add_positional(
                const std::string& name,
                TMember TArgs::*member,
                TMember defaultValue,
                const std::string& description)
            {
                return add_positional(name, member, defaultValue, description, impl::default_conversion_factory<TMember>);
            }

            ArgumentParser<TArgs> build() const
            {
                return ArgumentParser<TArgs>(m_args);
            }
    };
}
