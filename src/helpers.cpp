#include <string>

bool starts_with(const std::string& s, const std::string& p) {
    return s.rfind(p, 0) == 0;
}

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t");
    size_t e = s.find_last_not_of(" \t");
    if (b == std::string::npos) return "";
    return s.substr(b, e - b + 1);
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string replace_extension(const std::string& filename, const std::string& new_ext) {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return filename + new_ext;
    return filename.substr(0, pos) + new_ext;
}

std::string replace_directory(const std::string& filename, const std::string& new_dir) {
    size_t pos = filename.find_last_of("/\\");
    std::string base = (pos == std::string::npos)
        ? filename
        : filename.substr(pos + 1);

    if (new_dir.empty())
        return base;

    if (new_dir.back() == '/' || new_dir.back() == '\\')
        return new_dir + base;

    return new_dir + "/" + base;
}