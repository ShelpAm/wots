#include <print>
#include <spdlog/spdlog.h>
#include <wots/wots.hpp>

void usage(char **argv)
{
    std::println("usage: {} [<options>...] <dotfiles-dir> <install-dir> "
                 "<packages>...",
                 *argv);
    std::println("OPTIONS: -v --verbose     turn on debugging mode\n"
                 "         -n --no-dry-run  create symlinks, instead of "
                 "printing the symlinks tasks\n");
}

int main(int argc, char *argv[])
{
    if (argc == 2 && (std::string_view(argv[1]) == "--help" ||
                      std::string_view(argv[1]) == "-h")) {
        usage(argv);
        return EXIT_SUCCESS;
    }
    auto program = argv;
    if (argc < 4) {
        usage(argv);
        std::exit(EXIT_FAILURE);
    }

    spdlog::set_level(spdlog::level::info);

    spdlog::debug("argc: {}", argc);

    ++argv; // Skip argv[0]

    bool dry_run{true};
    while (*argv != nullptr && **argv == '-') {
        std::string_view arg{*argv++};
        if (arg == "--") {
            break;
        }
        if (arg == "-v" || arg == "--verbose") {
            spdlog::set_level(spdlog::level::debug);
        }
        else if (arg == "-n" || arg == "--no-dry-run") {
            dry_run = false;
        }
        else {
            spdlog::error("Unknown option: {}", arg);
            return EXIT_FAILURE;
        }
    }

    if (argc - (argv - program) < 3) {
        usage(program);
        return EXIT_FAILURE;
    }

    std::string_view dotfiles_dir{*argv++};
    std::string_view install_dir{*argv++};
    std::vector<std::string_view> packages;
    while (*argv != nullptr) {
        spdlog::debug("Reading packages *argv: {}", *argv);
        packages.emplace_back(*argv++);
    }

    try {
        wots::perform_wots(dotfiles_dir, install_dir, packages, dry_run);
    }
    // TODO(shelpam): exceptions should be extended with more information.
    catch (wots::File_conflict_error e) {
        spdlog::error("File conflict detected");
        return EXIT_FAILURE;
    }
    catch (wots::Unsupported_filetype_error e) {
        spdlog::error("Unsupported filetype");
        return EXIT_FAILURE;
    }
    catch (std::exception const &e) {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }
}
