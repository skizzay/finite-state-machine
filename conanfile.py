from pathlib import Path
from conans import ConanFile, CMake


def _FsmConan__read_contents_of_file(filename):
    license = Path(__file__).parent.joinpath(filename)
    with license.open() as fd:
        return fd.read()


class FsmConan(ConanFile):
    name = "fsm"
    version = "0.0.1"
    license = __read_contents_of_file("LICENSE")
    url = "https://github.com/skizzay/finite-state-machine"
    description = __read_contents_of_file("README.md")
    build_requires = "catch2/2.2.2@bincrafters/stable", "cmake_installer/3.11.2@conan/stable"
    settings = "build_type"
    generators = "cmake"
    no_copy_source = True
    export_sources = "src/*", "CMakeLists.txt", "test/*"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        self.copy("*.h", "include")

    def package_id(self):
        self.info.header_only()
