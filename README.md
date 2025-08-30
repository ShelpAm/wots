# Wots

This is a tool designed for dotfile management. The name is originated from `GNU Stow`, as a reverse
of it.

I created this tool because `stow` has bug in dotfile management with tree folding. See more in
<https://github.com/aspiers/stow/issues/120>.

## Principles

* User created directories shouldn't be removed, since they're not owned by wots. What wots should
  do is just patching the install directory.


## Installation

### Prerequisite

Before compiling wots, make sure you have the following installed:
* [C++23](https://en.cppreference.com/w/cpp/23.html) compatible compiler
* [git](https://git-scm.com/)
* [xmake](https://xmake.io/)

### Building

Clone the repository and build with **xmake**:
```bash
git clone https://github.com/ShelpAm/wots.git
cd wots
xmake
```

Find built executables in `./build/`.

### Testing

To run tests:
```bash
xmake run tests
```
