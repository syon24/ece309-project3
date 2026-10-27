#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "path contains a NUL byte";
        return false;
    }

    try {
        fs::path p{std::string(user_path)};
        if (p.is_absolute() || p.has_root_path()) {
            *reason = "absolute paths are not allowed";
            return false;
        }
        fs::path n = p.lexically_normal();
        if (n.begin() != n.end() && *n.begin() == "..") {
            *reason = "path escapes the workspace";
            return false;
        }
        *out = n == "." ? root() : root() / n;
    } catch (...) {
        *reason = "could not resolve path";
        return false;
    }
    return true;
}
