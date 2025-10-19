#include "rdpp_common/Logging.hpp"

using namespace rdpp::common;

void log::printdbg(const char *str, std::source_location loc) {
    log::print(loc, str);
}

void log::printrel(const char *str, std::source_location loc) {
    log::print(loc, str);
}


