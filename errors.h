#pragma once

namespace ap {
    enum class ParsingError {
        UnknownArgument,
        MissingPositionalArgument,
        TooManyPositionalArguments,
        MissingValueForArgument
    };
}
