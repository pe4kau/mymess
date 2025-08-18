#pragma once
#include <string>
#include <optional>

namespace messenger {
std::optional<std::string> getenv_or(const std::string &name, const std::string &fallback);
std::string now_iso();
}
