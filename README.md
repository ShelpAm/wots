# Wots

This is a tool designed for dotfile management. The name is originated from `GNU Stow`, as a reverse
of it.

I created this tool because `stow` has bug in dotfile management with tree folding. See more in
<https://github.com/aspiers/stow/issues/120>.

## Principles

* User-created directories shouldn't be removed, since they're not owned by wots. What wots should
  do is just patching the install directory.

## Installation

### Prerequisites

Before compiling wots, make sure you have the following installed:
* [C++23](https://en.cppreference.com/w/cpp/23.html) compatible compiler
* [git](https://git-scm.com/)
* Build system and dependency manager
    * [xmake](https://xmake.io/)
    * [CMake](https://cmake.org/) and [Conan](https://conan.io/)

### Building

Clone the repository and build with **xmake**:
```bash
git clone https://github.com/ShelpAm/wots.git
cd wots
xmake
```

Clone the repository and build with **cmake** and **conan**:
```bash
git clone https://github.com/ShelpAm/wots.git
cd wots
conan install . -of build -b missing
cmake --preset=<your-preset>
cmake --build build
```

Find built executables in `./build/`.

### Testing

To run tests:
```bash
xmake run tests
```

With CMake, tests are enabled by default (requires GTest and Python 3):
```bash
cmake --build build
ctest --test-dir build --output-on-failure
```
Use `-DBUILD_TESTING=OFF` when configuring a build without test dependencies.

### Usage

By default, wots only previews the planned changes:
```bash
wots ~/.dotfiles ~ swaync
```
Pass `-n` / `--no-dry-run` to apply them:
```bash
wots -n ~/.dotfiles ~ swaync
```
Existing symlinks are replaced only when their resolved targets are inside the
dotfiles directory. Conflicts are checked before old links are removed, and
failures return a nonzero exit status. This preflight check does not provide
rollback for filesystem errors during execution.
