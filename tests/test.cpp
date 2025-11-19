#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <wots/wots.hpp>

using namespace wots;

void write_file(fs::path const &path, std::string_view s)
{
    std::ofstream ofs(path);
    ofs << s;
}

std::string read_file(fs::path const &path)
{
    std::ifstream ifs(path);
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.rdbuf()->str();
}

class TempDirTest : public testing::Test {
  public:
    static fs::path const temp_dir;
    static fs::path const dotfiles_dir;
    static fs::path const install_dir;

    TempDirTest(TempDirTest const &) = delete;
    TempDirTest(TempDirTest &&) = delete;
    TempDirTest &operator=(TempDirTest const &) = delete;
    TempDirTest &operator=(TempDirTest &&) = delete;
    ~TempDirTest() override
    {
        fs::remove_all(temp_dir);
    }

  protected:
    TempDirTest()
    {
        fs::create_directories(dotfiles_dir);
        fs::create_directories(install_dir);
    }

    class Directory_structure {
      public:
        Directory_structure(std::set<std::pair<fs::path, std::string>> files,
                            std::set<fs::path> dirs)
            : files_(std::move(files)), dirs_(std::move(dirs))
        {
            for (auto [path, _] : files_) {
                while (path.has_parent_path()) {
                    path = path.parent_path();
                    dirs_.insert(path);
                }
            }
        }

        void cast_to_fs(fs::path const &base) const
        {
            for (auto const &dir : dirs_) {
                fs::create_directories(base / dir);
            }
            for (auto const &[file, content] : files_) {
                write_file(base / file, content);
            }
        }

        [[nodiscard]] bool exists_in(fs::path const &base) const
        {
            return std::ranges::all_of(dirs_,
                                       [&base](auto const &dir) {
                                           return fs::exists(base / dir);
                                       }) &&
                   std::ranges::all_of(files_, [&base](auto const &pair) {
                       auto const &[file, expected_content] = pair;
                       return fs::exists(base / file) &&
                              read_file(base / file) == expected_content;
                   });
        }

      private:
        // Path, Content pairs
        // Paths are relative to dotfiles_dir and install_dir.
        std::set<std::pair<fs::path, std::string>> files_;
        std::set<fs::path> dirs_;
    };
};

fs::path const TempDirTest::temp_dir{fs::temp_directory_path() / "wots"};
fs::path const TempDirTest::dotfiles_dir{temp_dir / "dotfiles"};
fs::path const TempDirTest::install_dir{TempDirTest::temp_dir / "install"};

TEST_F(TempDirTest, SelfTest)
{
    Directory_structure const packs({{"p1/lc", "li hai"}}, {});
    packs.cast_to_fs(dotfiles_dir);
    EXPECT_TRUE(packs.exists_in(dotfiles_dir));
    perform_wots(dotfiles_dir, install_dir, {"p1"});
    EXPECT_TRUE(fs::exists(install_dir / "lc"));
    EXPECT_TRUE(read_file(install_dir / "lc") == "li hai");
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
    write_file(dotfiles_dir / "pack" / "a", "a");
    write_file(dotfiles_dir / "pack" / "b", "b");
    write_file(dotfiles_dir / "pack2" / "b", "2,b");

    EXPECT_THROW(perform_wots(dotfiles_dir, install_dir, {"pack", "pack2"}),
                 File_conflict_error);
}

TEST(Wots, ConflictWithExsitingFile)
{
    auto tmp = fs::temp_directory_path() / "wots";
    auto dotfiles_dir = tmp / "dotfiles";
    auto const &install_dir = tmp;
    fs::create_directories(dotfiles_dir / "pack");
    write_file(dotfiles_dir / "pack" / "a", "from pack");
    write_file(install_dir / "a", "already exists");

    EXPECT_THROW(perform_wots(dotfiles_dir, install_dir, {"pack"}),
                 File_conflict_error);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
