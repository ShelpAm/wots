#include <wots/wots.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wots-cli/main.h>
#include <wots/task.hpp>

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

static fs::path replace_dots(fs::path const &path)
{
    return replace_all_subpath_prefix(path, "dot-", ".");
}

void wots::perform_wots(fs::path const &dotfiles_dir,
                        fs::path const &install_dir,
                        std::vector<std::string_view> const &packages)
{
    detect_unsupported_filetype(dotfiles_dir, packages);
    detect_file_conflicts(dotfiles_dir, packages);

    auto prework_result = prework(dotfiles_dir, install_dir, packages);
    auto tasks = calculate_tasks(dotfiles_dir, install_dir, prework_result);
    unwots(dotfiles_dir, tasks);

    if (dry_run) {
        spdlog::info("Now is in dry-run mode, wots won't create symlinks.");
    }
    // Runs actual tasks
    for (auto const &task : tasks) {
        task->log();
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
                spdlog::error("detected conflict: {}", p.string());
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
wots::prework(fs::path const &dotfiles_dir, fs::path const &install_dir,
              std::vector<std::string_view> const &packages)
{
    // For relative paths
    std::map<fs::path, bool> should_unfold;
    std::unordered_map<fs::path, std::vector<std::string_view>>
        origin_packages_of_dir;
    std::unordered_map<fs::path, std::size_t> n_dotted_children;
    for (auto package : packages) {
        auto package_root = dotfiles_dir / package;
        for (auto const &entry :
             fs::recursive_directory_iterator(package_root)) {
            auto relative_path = fs::relative(entry, package_root);
            should_unfold.insert({relative_path, false});
            if (entry.is_directory()) {
                origin_packages_of_dir[relative_path].push_back(package);
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
    // Path already existing should be unfolded.
    for (auto &[rel_path, should_unfold] : should_unfold) {
        should_unfold = origin_packages_of_dir[rel_path].size() > 1 ||
                        n_dotted_children[rel_path] > 0;
        auto link = install_dir / replace_dots(rel_path);
        if (fs::is_directory(link) && !is_under_symlink(link, install_dir)) {
            should_unfold |= true;
        }
        spdlog::debug("{}: origin_packages_of_dir={} n_dotted_children={}",
                      rel_path.string(),
                      origin_packages_of_dir[rel_path].size(),
                      n_dotted_children[rel_path]);
    }
    // Package roots also should unfold, no matter what.
    should_unfold["."] = true;
    origin_packages_of_dir["."] = packages;

    for (auto const &[p, s] : should_unfold) {
        if (s) {
            spdlog::debug("should_unfold: {}", p.string());
        }
    }

    return {.should_unfold = should_unfold,
            .origin_packages_of_dir = origin_packages_of_dir};
}

std::vector<std::unique_ptr<Task>>
wots::calculate_tasks(fs::path const &dotfiles_dir, fs::path const &install_dir,
                      Prework_result const &prework_result)
{
    auto [should_unfold, origin_packages_of_dir] = prework_result;

    std::vector<std::unique_ptr<Task>> tasks;
    for (auto const &[rel_path, should] : should_unfold) {
        if (!should) {
            continue;
        }
        tasks.push_back(std::make_unique<Make_directory>(
            install_dir / replace_dots(rel_path)));
        for (auto package : origin_packages_of_dir[rel_path]) {
            auto package_root = dotfiles_dir / package;
            auto unfolded_dir = package_root / rel_path;
            for (auto const &child : fs::directory_iterator(unfolded_dir)) {
                if (should_unfold[fs::relative(child.path(), package_root)]) {
                    continue;
                }
                auto to = unfolded_dir / child.path().filename();
                to = fs::absolute(to);
                auto from = install_dir /
                            replace_dots(rel_path / child.path().filename());
                tasks.push_back(std::make_unique<Make_symlink>(from, to));
            }
        }
    }
    return tasks;
}

void wots::unwots(fs::path const &dotfiles_dir,
                  std::vector<std::unique_ptr<Task>> const &tasks)
{
    for (auto const &task : tasks) {
        if (auto *t =
                dynamic_cast<Make_symlink *>(task.get())) { // Maybe slow here
            auto from = t->from();
            auto abs_to = fs::absolute(t->to());
            auto abs_dotfiles_dir = fs::absolute(dotfiles_dir);
            bool from_is_symlink{fs::is_symlink(from)};
            bool from_owned{
                abs_to.string().starts_with(abs_dotfiles_dir.string())};
            spdlog::debug("{} is_symlink={} owned={}", from.string(),
                          from_is_symlink, from_owned);
            if (from_is_symlink && from_owned) {
                spdlog::info("Removing old link: {}", from.string());
                fs::remove(from);
            }
            else if (fs::exists(from)) {
                spdlog::error("Linking {}: {} already exists and is not "
                              "symlink or owned, wots can't override it.",
                              abs_to.string(), from.string());
                throw File_conflict_error{};
            }
        }
    }
}

bool wots::is_under_symlink(fs::path p, fs::path const &end)
{
    while (p != end) {
        if (fs::is_symlink(p)) {
            return true;
        }
        p = p.parent_path();
    }
    return false;
}
