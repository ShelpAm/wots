#include <wots-cli/wots.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wots-cli/main.h>
#include <wots-cli/task.hpp>

using namespace wots;

fs::path wots::replace_all_subpath_prefix(fs::path const &path,
                                          std::string_view prefix,
                                          std::string_view replacement)
{
    fs::path result;

    for (auto const &part : path) {
        std::string s = part.string();
        if (s.starts_with(prefix)) { // starts_with(prefix)
            s.replace(0, prefix.size(), replacement);
        }
        result /= s;
    }

    return result;
}

static fs::path replace_dots(fs::path const &p)
{
    return replace_all_subpath_prefix(p, "dot-", ".");
}

void wots::perform_wots(fs::path const &dotfiles_dir,
                        fs::path const &install_dir,
                        std::vector<std::string_view> const &packages)
{
    detect_unsupported_filetype(dotfiles_dir, packages);
    detect_file_conflicts(dotfiles_dir, packages);

    // Precalculate
    auto [should_unfold, origin_packages_of_path] =
        prework(dotfiles_dir, packages);

    // Perform actual linkages
    std::vector<std::unique_ptr<Task>> tasks;
    for (auto const &[rel_path, should] : should_unfold) {
        if (!should) {
            continue;
        }
        spdlog::debug("should_unfold: {}", rel_path.string());
        tasks.push_back(std::make_unique<Make_directory>(
            replace_dots(install_dir / rel_path)));
        for (auto package : origin_packages_of_path[rel_path]) {
            auto package_root = dotfiles_dir / package;
            auto unfolded_dir = package_root / rel_path;
            for (auto const &child : fs::directory_iterator(unfolded_dir)) {
                if (should_unfold[fs::relative(child.path(), package_root)]) {
                    continue;
                }
                auto to = fs::absolute(unfolded_dir / child.path().filename());
                auto link = install_dir / rel_path / child.path().filename();
                link = replace_dots(link);
                if (fs::exists(link)) {
                    if (!fs::is_symlink(link) || fs::read_symlink(link) != to) {
                        throw std::runtime_error(std::format(
                            "{} already exists, wots cann't override it.",
                            link.string()));
                    }
                }
                else {
                    tasks.push_back(std::make_unique<Make_symlink>(link, to));
                }
            }
        }
    }

    if (dry_run) {
        spdlog::info("In dry-run mode, won't create symlinks. The followings "
                     "are symlinks to be created:");
    }
    for (auto const &task : tasks) {
        if (!dry_run) {
            task->run();
        }
    }
}

void wots::detect_file_conflicts(fs::path const &dotfiles_dir,
                                 std::vector<std::string_view> const &packages)
{
    std::unordered_set<fs::path> s;
    for (auto const &package : packages) {
        auto package_root = dotfiles_dir / package;
        for (auto const &entry :
             fs::recursive_directory_iterator(package_root)) {
            // Same directory names aren't regarded as conflict.
            if (entry.is_directory()) {
                continue;
            }
            auto p = fs::relative(entry, package_root);
            if (s.contains(p)) {
                spdlog::debug("detected conflict: {}", p.string());
                throw File_conflict_error{};
            }
            s.insert(p);
        }
    }
}

void wots::detect_unsupported_filetype(
    fs::path const &dotfiles_dir, std::vector<std::string_view> const &packages)
{
    for (auto const &package : packages) {
        for (auto const &entry :
             fs::recursive_directory_iterator(dotfiles_dir / package)) {
            // Same directory names aren't regarded as conflict.
            if (!entry.is_directory() && !entry.is_regular_file()) {
                throw Unsupported_filetype_error{};
            }
        }
    }
}

wots::Prework_result
wots::prework(fs::path const &dotfiles_dir,
              std::vector<std::string_view> const &packages)
{
    // For relative paths
    std::map<fs::path, bool> should_unfold;
    std::unordered_map<fs::path, std::vector<std::string_view>>
        origin_packages_of_path;
    std::unordered_map<fs::path, std::size_t> n_dotted_children;
    for (auto package : packages) {
        auto package_root = dotfiles_dir / package;
        for (auto const &entry :
             fs::recursive_directory_iterator(package_root)) {
            auto relative_path = fs::relative(entry, package_root);
            should_unfold.insert({relative_path, false});
            if (entry.is_directory()) {
                origin_packages_of_path[relative_path].push_back(package);
            }
            if (entry.path().filename().string().starts_with("dot-")) {
                spdlog::debug("dot- found in {}", entry.path().string());
                for (auto p = entry.path().parent_path(); p != package_root;
                     p = p.parent_path()) {
                    spdlog::debug("parenting: {}", p.string());
                    ++n_dotted_children[fs::relative(p, package_root)];
                }
            }
        }
    }
    for (auto &[rel_path, should_unfold] : should_unfold) {
        should_unfold = origin_packages_of_path[rel_path].size() > 1 ||
                        n_dotted_children[rel_path] > 0;
        spdlog::debug("{}: n_origin_packages={} n_dotted_children={}",
                      rel_path.string(),
                      origin_packages_of_path[rel_path].size(),
                      n_dotted_children[rel_path]);
    }
    // Package roots also should unfold, no matter what.
    should_unfold["."] = true;
    origin_packages_of_path["."] = packages;

    return {.should_unfold = should_unfold,
            .origin_packages_of_path = origin_packages_of_path};
}
