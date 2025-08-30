#pragma once
#include <filesystem>
#include <map>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace wots {

namespace fs = std::filesystem;

fs::path replace_all_subpath_prefix(fs::path const &path,
                                    std::string_view prefix,
                                    std::string_view replacement);

// Directories can be merged, while same dest filepath is regarded as conflict.
class File_conflict_error {};

class Unsupported_filetype_error {};

struct Prework_result {
    std::map<fs::path, bool> should_unfold;
    std::unordered_map<fs::path, std::vector<std::string_view>>
        origin_packages_of_path; // Packages who contain `path`
};

Prework_result prework(fs::path const &dotfiles_dir,
                       std::vector<std::string_view> const &packages);

void perform_wots(fs::path const &dotfiles_dir, fs::path const &install_dir,
                  std::vector<std::string_view> const &packages);

// @throws: Unsupported_filetype_error if packages have unsupported filetype(s).
void detect_unsupported_filetype(fs::path const &dotfiles_dir,
                                 std::vector<std::string_view> const &packages);

// @throws: File_conflict_error if packages have conflict files.
void detect_file_conflicts(fs::path const &dotfiles_dir,
                           std::vector<std::string_view> const &packages);

} // namespace wots
