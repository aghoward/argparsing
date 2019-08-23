#pragma once

#include <cstdint>
#include <string>

#include "cdif/cdif.h"

namespace ap {
    class PrimitivesModule : public cdif::IModule
    {
        public:
            void load(cdif::Container& ctx) override
            {
                ctx.bind<unsigned long long, std::string>(
                    [] (std::string v) { return std::stoull(v, 0, 0); })
                    .build();
                ctx.bind<unsigned long, std::string>(
                    [] (std::string v) { return std::stoul(v, 0, 0); })
                    .build();

                ctx.bind<short, std::string>(
                    [] (std::string v) { return static_cast<short>(std::stoul(v, 0, 0)); })
                    .build();

                ctx.bind<int, std::string>(
                    [] (std::string v) { return std::stoi(v, 0, 0); })
                    .build();

                ctx.bind<unsigned int, std::string>(
                    [] (std::string v) { return static_cast<unsigned int>(std::stoul(v, 0, 0)); })
                    .build();

                ctx.bind<unsigned short, std::string>(
                    [] (std::string v) { return static_cast<unsigned short>(std::stoul(v, 0, 0)); })
                    .build();

                ctx.bind<long, std::string>(
                    [] (std::string v) { return std::stol(v, 0, 0); })
                    .build();
                ctx.bind<long long, std::string>(
                    [] (std::string v) { return std::stoll(v, 0, 0); })
                    .build();

                ctx.bind<std::string, std::string>(
                    [] (std::string v) { return v; })
                    .build();
            }
    };
}
