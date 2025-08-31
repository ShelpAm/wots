#include <fstream>
#include <gtest/gtest.h>
#include <wots/wots.hpp>

using namespace wots;

void write_to_file(fs::path const &path, std::string_view s)
{
    std::ofstream ofs(path);
    ofs << s;
}

TEST(Wots, FileConflict)
{
    // tmp/wots/dotfiles/pack/a
    // tmp/wots/dotfiles/pack/b
    // tmp/wots/dotfiles/pack2/b
    auto tmp = fs::temp_directory_path() / "wots";
    auto dotfiles_dir = tmp / "dotfiles";
    auto const &install_dir = tmp;
    fs::create_directories(dotfiles_dir / "pack");
    fs::create_directories(dotfiles_dir / "pack2");
    write_to_file(dotfiles_dir / "pack" / "a", "a");
    write_to_file(dotfiles_dir / "pack" / "b", "b");
    write_to_file(dotfiles_dir / "pack2" / "b", "2,b");

    EXPECT_THROW(perform_wots(dotfiles_dir, install_dir, {"pack", "pack2"}),
                 File_conflict_error);
}

TEST(Wots, ConflictWithExsitingFile)
{
    auto tmp = fs::temp_directory_path() / "wots";
    auto dotfiles_dir = tmp / "dotfiles";
    auto const &install_dir = tmp;
    fs::create_directories(dotfiles_dir / "pack");
    write_to_file(dotfiles_dir / "pack" / "a", "from pack");
    write_to_file(install_dir / "a", "already exists");

    EXPECT_THROW(perform_wots(dotfiles_dir, install_dir, {"pack"}),
                 File_conflict_error);
}

TEST(Wots, ShouldUnfoldfile)
{
    auto tmp = fs::temp_directory_path() / "wots";
    auto dotfiles_dir = tmp / "dotfiles";
    auto const &install_dir = tmp;
    fs::create_directories(dotfiles_dir / "bin" / "test");
    write_to_file(dotfiles_dir / "bin" / "test" / "dot-bin", "");
    perform_wots(dotfiles_dir, install_dir, {"bin"});
    EXPECT_TRUE(fs::exists(install_dir / ".bin"));
}
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
