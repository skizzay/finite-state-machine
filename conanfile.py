from pathlib import Path
from conans import ConanFile


def _FsmConan__read_contents_of_file(filename):
    license = Path(__file__).parent.joinpath(filename)
    with license.open() as fd:
        return fd.read()


class FsmConan(ConanFile):
    name = "fsm"
    version = "0.0.1"
    license = "MIT License"
    url = "https://github.com/skizzay/finite-state-machine"
    description = __read_contents_of_file("README.md")
    build_requires = "catch2/2.2.2@bincrafters/stable", "cmake_installer/3.11.2@conan/stable"
    no_copy_source = True
    exports_sources = "src/**/*.h"
    exports = "LICENSE", "README.md"

    def package(self):
        self.copy("*.h", dst="include", src="src", keep_path=True)

    def package_id(self):
        self.info.header_only()
