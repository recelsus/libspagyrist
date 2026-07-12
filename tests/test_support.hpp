#pragma once

#include <exception>
#include <iostream>
#include <string_view>

#define SPAGYRIST_CHECK(condition)                                                                  \
    do {                                                                                            \
        if (!(condition)) {                                                                         \
            std::cerr << __FILE__ << ':' << __LINE__ << ": check failed: " << #condition << '\n';   \
            std::terminate();                                                                       \
        }                                                                                           \
    } while (false)

using test_fn = void (*)();

struct test_case {
    std::string_view name;
    test_fn run;
};

