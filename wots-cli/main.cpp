#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

void replace_filename_prefix(fs::path &path, std::string_view prefix,
                             std::string_view replacement)
{
    auto filename = path.filename().string();
    if (filename.starts_with(prefix)) {
        filename.replace(0, prefix.size(), replacement);
        path.replace_filename(filename);
    }
}

class Wots {
  public:
    Wots(std::string dotfiles_dir, std::string install_dir) noexcept
        : dotfiles_dir_{std::move(dotfiles_dir)},
          install_dir_{std::move(install_dir)}
    {
    }

    void wots(std::string_view package)
    {
        auto package_root = dotfiles_dir_ / package;
        do_wots(package_root, package_root);
    }

  private:
    fs::path dotfiles_dir_;
    fs::path install_dir_;

    // @brief Processes all children or delays to the parent.
    // @return If this directory can be folded
    bool do_wots(fs::path const &current, fs::path const &package_root)
    {
        bool can_be_folded{true};

        // Inspect each entry
        for (auto const &son : fs::directory_iterator(current)) {
            spdlog::debug("going into {}", son.path().string());
            if (son.is_directory()) {
                can_be_folded &= do_wots(son, package_root);
            }
            else if (son.is_regular_file()) {
                if (son.path().stem().string().starts_with("dot-")) {
                    can_be_folded &= false;
                }
            }
            else {
                throw std::runtime_error{"unsupported filetype"};
            }
        }

        // If children are all undotted, delay the wots process to the parent.
        // Otherwise, wots them now, and make current directory.
        // When in root, wots all children directly.
        if (can_be_folded && current != package_root) {
            return true;
        }

        fs::create_directories(current);
        for (auto const &son : fs::directory_iterator(current)) {
            auto relative = fs::relative(son, package_root);
            replace_filename_prefix(relative, "dot-", ".");
            spdlog::debug("wots: {} -> {}", son.path().string(),
                          (install_dir_ / relative).string());
            fs::create_symlink(son, install_dir_ / relative);
        }
        return false;
    }
};

int main()
{
    spdlog::set_level(spdlog::level::debug);

    std::string dotfiles_dir{"/home/shelpam/.dotfiles"};
    std::string install_dir{"/tmp/"};

    Wots wots(dotfiles_dir, install_dir);

    std::vector<std::string_view> packages{"neovim", "clangd", "tmux", "zsh",
                                           "hyprland"};

    for (auto package : packages) {
        try {
            wots.wots(package);
        }
        catch (std::exception const &e) {
            spdlog::error(e.what());
        }
    }
}
