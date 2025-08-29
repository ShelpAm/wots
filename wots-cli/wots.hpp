#include <filesystem>
#include <map>
#include <string_view>
#include <vector>

namespace wots {

namespace fs = std::filesystem;

fs::path replace_all_subpath_prefix(fs::path const &path,
                                    std::string_view prefix,
                                    std::string_view replacement);

// Directories can be merged, while same dest filepath is regarded as conflict.
class File_conflict_error {};

class Unsupported_filetype_error {};

void perform_wots(fs::path const &dotfiles_dir, fs::path const &install_dir,
                  std::vector<std::string_view> const &packages);

// @brief: Throws Unsupported_filetype_error if packages have conflict
// files.
void detect_unsupported_filetype(fs::path const &dotfiles_dir,
                                 std::vector<std::string_view> const &packages);

// @brief: Throws File_conflict_error if packages have conflict files.
void detect_file_conflicts(fs::path const &dotfiles_dir,
                           std::vector<std::string_view> const &packages);

// [[nodiscard]] std::map<fs::path, fs::path>
// get_mapping(std::vector<std::string_view> const &packages) const;

// @brief Processes all children or delays to the parent.
// @return If this directory can be folded
// bool do_wots(fs::path const &current, fs::path const &package_root);

// class Wots {
//   public:
//     Wots(std::string dotfiles_dir, std::string target_dir) noexcept;
//
//     void wots(std::vector<std::string_view> const &packages);
//
//   private:
//     fs::path dotfiles_dir_;
//     fs::path install_dir_;
//
//     // @brief: Throws Unsupported_filetype_error if packages have conflict
//     // files.
//     void detect_unsupported_filetype(
//         std::vector<std::string_view> const &packages) const;
//
//     // @brief: Throws File_conflict_error if packages have conflict files.
//     void
//     detect_file_conflicts(std::vector<std::string_view> const &packages)
//     const;
//
//     // [[nodiscard]] std::map<fs::path, fs::path>
//     // get_mapping(std::vector<std::string_view> const &packages) const;
//
//     bool should_unfold(fs::path const &dir);
//
//     // @brief Processes all children or delays to the parent.
//     // @return If this directory can be folded
//     bool do_wots(fs::path const &current, fs::path const &package_root);
// };

} // namespace wots
