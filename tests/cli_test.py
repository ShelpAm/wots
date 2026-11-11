"""Exercise the CLI against isolated filesystem fixtures."""
import pathlib
import subprocess
import sys
import tempfile
import unittest

EXE = sys.argv.pop(1)


class CliTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="wots-tests-")
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        self.dotfiles = self.root / "dotfiles"
        self.package = self.dotfiles / "p"
        self.install = self.root / "install"
        self.package.mkdir(parents=True)
        self.install.mkdir()
        (self.package / "a").write_text("new")

    def run_wots(self, *options):
        return subprocess.run(
            [EXE, *options, str(self.dotfiles), str(self.install), "p"],
            capture_output=True, text=True, timeout=5,
        )

    def test_foreign_links_are_preserved(self):
        for name in ("external", "dotfiles-other/a", "missing"):
            with self.subTest(target=name):
                target = self.root / name
                link = self.install / "a"
                link.symlink_to(target)
                self.assertNotEqual(self.run_wots("-n").returncode, 0)
                self.assertEqual(link.readlink(), target)
                link.unlink()

    def test_owned_relative_and_dangling_links_can_be_replaced(self):
        link = self.install / "a"
        link.symlink_to("../dotfiles/deleted")
        self.assertEqual(self.run_wots("-n").returncode, 0)
        self.assertEqual(link.resolve(), self.package / "a")
        self.assertEqual(self.run_wots("-n").returncode, 0)

    def test_late_conflict_preserves_earlier_link(self):
        (self.package / "z").write_text("new z")
        link = self.install / "a"
        link.symlink_to(self.package / "a")
        (self.install / "z").write_text("keep")
        self.assertNotEqual(self.run_wots("-n").returncode, 0)
        self.assertEqual(link.readlink(), self.package / "a")
        self.assertEqual((self.install / "z").read_text(), "keep")

    def test_dry_run_preserves_links_and_files(self):
        link = self.install / "a"
        link.symlink_to("../dotfiles/p/a")
        self.assertEqual(self.run_wots().returncode, 0)
        self.assertEqual(link.readlink(), pathlib.Path("../dotfiles/p/a"))

    def test_dot_prefix_and_existing_directories(self):
        (self.package / "dot-config" / "demo").mkdir(parents=True)
        (self.package / "dot-config" / "demo" / "settings").write_text("config")
        (self.install / ".config" / "demo").mkdir(parents=True)
        self.assertEqual(self.run_wots("-n").returncode, 0)
        link = self.install / ".config" / "demo" / "settings"
        self.assertTrue(link.is_symlink())
        self.assertEqual(link.read_text(), "config")

    def test_bad_arguments_fail_without_crashing(self):
        for args in (["-v", "-n", "-v"], ["-v", "-n", str(self.dotfiles)],
                     ["--unknown", str(self.dotfiles), str(self.install), "p"]):
            with self.subTest(args=args):
                result = subprocess.run([EXE, *args], capture_output=True, timeout=5)
                self.assertGreater(result.returncode, 0)
        self.assertEqual(subprocess.run([EXE, "--help"], capture_output=True).returncode, 0)


if __name__ == "__main__":
    unittest.main()
