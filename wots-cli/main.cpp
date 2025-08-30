#include <wots-cli/main.h>

#include <print>
#include <spdlog/spdlog.h>
#include <wots/wots.hpp>

void usage(char **argv)
{
    std::println("usage: {} [<options>...] <dotfiles-dir> <install-dir> "
                 "<packages>...",
                 *argv);
    std::println("OPTIONS: -v --verbose  turn on debugging mode\n"
                 "         -n --dry-run  do not create symlinks, but print the "
                 "symlinks tasks\n");
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        usage(argv);
        std::exit(EXIT_FAILURE);
    }

    spdlog::set_level(spdlog::level::info);

    ++argv; // Skip argv[0]

    while (*argv != nullptr && **argv == '-') {
        std::string_view arg{*argv++};
        if (arg == "-v" || arg == "--verbose") {
            spdlog::set_level(spdlog::level::debug);
        }
        else if (arg == "-n" || arg == "--dry-run") {
            dry_run = true;
        }
    }

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
