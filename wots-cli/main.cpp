#include <print>
#include <spdlog/spdlog.h>
#include <wots-cli/wots.hpp>

int main(int argc, char *argv[])
{
    spdlog::set_level(spdlog::level::debug);

    if (argc < 4) {
        std::println("usage: {} <dotfiles-dir> <install-dir> <packages>...",
                     *argv);
        std::exit(EXIT_FAILURE);
    }

    ++argv; // Skip argv[0]
    std::string_view dotfiles_dir{*argv++};
    std::string_view install_dir{*argv++};
    std::vector<std::string_view> packages;
    while (*argv != nullptr) {
        packages.emplace_back(*argv++);
    }

    try {
        wots::perform_wots(dotfiles_dir, install_dir, packages);
    }
    // TODO(shelpam): exceptions should be extended with more information.
    catch (wots::File_conflict_error e) {
        spdlog::error("File conflict detected");
    }
    catch (wots::Unsupported_filetype_error e) {
        spdlog::error("Unsupported filetype");
    }
    catch (std::exception const &e) {
        spdlog::error(e.what());
    }
}
