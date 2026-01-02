#include <string>

extern bool starts_with(const std::string& s, const std::string& p);
extern std::string trim(const std::string& s);
extern bool ends_with(const std::string& str, const std::string& suffix);
extern std::string replace_extension(const std::string& filename, const std::string& new_ext);
extern std::string replace_directory(const std::string& filename, const std::string& new_dir);