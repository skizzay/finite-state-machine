from pathlib import Path
from conans import ConanFile


def _FsmConan__read_contents_of_file(filename):
    license = Path(__file__).parent.joinpath(filename)
    with license.open() as fd:
        return fd.read()


class FsmConan(ConanFile):
    name = "fsm"
    version = "0.1.0"
    license = _FsmConan__read_contents_of_file("LICENSE")
    url = "https://github.com/skizzay/finite-state-machine"
    description = _FsmConan__read_contents_of_file("README.md")
    requires = "utilz/0.0.1@skizzay/stable"
    no_copy_source = True
    exports_sources = "src/**/*.h"
    exports = "LICENSE", "README.md"
    # No settings/options are necessary, this is header only

    def package(self):
        self.copy("*.h", dst="include", src="src", keep_path=True)
