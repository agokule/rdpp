#pragma once

#include <iostream>
#include <source_location>
#include <format>
#include <tuple>

namespace rdpp::common::log {

void printdbg(const char *str, std::source_location loc = std::source_location::current());

void printrel(const char *str, std::source_location loc = std::source_location::current());

template<class ...Args>
void print(const std::source_location& loc, const std::string& msg) {

    std::cout << std::format("file: {}({}:{}) in function: \"{}\": {}\n",
                             loc.file_name(),
                             loc.line(),
                             loc.column(),
                             loc.function_name(),
                             msg);
}

template<class ...Args>
void print_with_fmt_impl(const std::format_string<Args...>& fmt, 
                   std::tuple<Args...>& args, 
                   const std::source_location& loc = std::source_location::current()) {
    auto msg = std::apply([&fmt](auto&&... a) {
        return std::format(fmt, std::forward<decltype(a)>(a)...);
    }, std::move(args));
    print(loc, msg);
}

template<class ...Args>
void printdbg(const std::format_string<Args...> fmt, 
              std::tuple<Args...>&& args, const std::source_location loc = std::source_location::current()) {
    #ifdef DEBUG
    print_with_fmt_impl(fmt, args, loc);
    #endif
}

template<class ...Args>
void printrel(const std::format_string<Args...> fmt, 
              std::tuple<Args...>&& args, const std::source_location loc = std::source_location::current()) {
    print_with_fmt_impl(fmt, args, loc);
}

void debug_assert(
    bool condition,
    std::string_view message,
    const std::source_location &location = std::source_location::current()
);

void release_assert(
    bool condition,
    std::string_view message,
    const std::source_location &location = std::source_location::current()
);

void debug_assert_fatal(
    bool condition,
    std::string_view message,
    const std::source_location &location = std::source_location::current()
);

void release_assert_fatal(
    bool condition,
    std::string_view message,
    const std::source_location &location = std::source_location::current()
);

}

