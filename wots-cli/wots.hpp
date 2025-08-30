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

/// @param p Beginning of paths to be checked.
/// @param end End path to be checked
/// @return If p is under some symlink, or p is a symlink.
bool is_under_symlink(fs::path p, fs::path const &end);

/// Directories can be merged, while same dest filepath is regarded as conflict.
class File_conflict_error {};

class Unsupported_filetype_error {};

struct Prework_result {
    std::map<fs::path, bool> should_unfold;
    std::unordered_map<fs::path, std::vector<std::string_view>>
        origin_packages_of_dir; // Packages who contain `path`
};

/// @return The prework result, see `Prework_result`.
Prework_result prework(fs::path const &dotfiles_dir,
                       fs::path const &install_dir,
                       std::vector<std::string_view> const &packages);

class Task;
std::vector<std::unique_ptr<Task>>
calculate_tasks(fs::path const &dotfiles_dir, fs::path const &install_dir,
                Prework_result const &result);

void unwots(fs::path const &dotfiles_dir,
            std::vector<std::unique_ptr<Task>> const &tasks);

void perform_wots(fs::path const &dotfiles_dir, fs::path const &install_dir,
                  std::vector<std::string_view> const &packages);

/// @throws Unsupported_filetype_error if packages have unsupported
/// filetype(s).
void detect_unsupported_filetype(fs::path const &dotfiles_dir,
                                 std::vector<std::string_view> const &packages);

/// @throws File_conflict_error if packages have conflict files.
void detect_file_conflicts(fs::path const &dotfiles_dir,
                           std::vector<std::string_view> const &packages);

} // namespace wots
