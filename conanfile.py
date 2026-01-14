from conan import ConanFile

class WotsRecipe(ConanFile):
    name = "wots"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"

    default_options = {
        "spdlog/*:use_std_fmt": True,
    }

    requires = (
        "spdlog/1.16.0",
        "gtest/1.17.0",
    )
    generators = (
        "CMakeDeps",
        "CMakeToolchain",
    )
