#include "rdpp_common/Logging.hpp"
#include <cstdlib>

using namespace rdpp::common;

void log::printdbg(const char *str, std::source_location loc) {
#ifdef DEBUG
    log::print(loc, str);
#endif // DEBUG
}

void log::printrel(const char *str, std::source_location loc) {
    log::print(loc, str);
}

void assert_fail(std::string_view message, bool fatal, const std::source_location& loc) {
    std::cerr << std::format("Assertion failed in {}({}:{}) in function: \"{}\": {}\n",
                             loc.file_name(),
                             loc.line(),
                             loc.column(),
                             loc.function_name(),
                             message);

    if (fatal)
        abort();
}

void log::debug_assert(
    bool condition,
    std::string_view message,
    const std::source_location &location) {
#ifdef DEBUG
  if (condition)
    return;

  assert_fail(message, false, location);
#endif // DEBUG
}


void log::release_assert(
    bool condition,
    std::string_view message,
    const std::source_location& location) {
  if (condition)
    return;
    
  assert_fail(message, false, location);
}

void log::debug_assert_fatal(
    bool condition,
    std::string_view message,
    const std::source_location &location) {
#ifdef DEBUG
  if (condition)
    return;

  assert_fail(message, true, location);
#endif // DEBUG
}


void log::release_assert_fatal(
    bool condition,
    std::string_view message,
    const std::source_location& location) {
  if (condition)
    return;
    
  assert_fail(message, true, location);
}

