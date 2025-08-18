#include "utils.h"
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace messenger {

std::optional<std::string> getenv_or(const std::string &name, const std::string &fallback) {
    const char* v = std::getenv(name.c_str());
    if (!v) return fallback;
    return std::string(v);
}

std::string now_iso() {
    using namespace std::chrono;
    auto t = system_clock::now();
    std::time_t tt = system_clock::to_time_t(t);
    std::tm tm = *std::gmtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

}
