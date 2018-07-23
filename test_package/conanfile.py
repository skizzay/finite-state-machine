import os

from conans import ConanFile, CMake, tools


class FsmTestConan(ConanFile):
    generators = "cmake"

    def __init__(self, *args, **kwargs):
        super(FsmTestConan, self).__init__(*args, **kwargs)
        self.cmake = None

    def build(self):
        self.cmake = CMake(self)
        # Current dir is "test_package/build/<build_id>" and CMakeLists.txt is
        # in "test_package"
        self.cmake.configure()
        self.cmake.build()

    def test(self):
        if not tools.cross_building(self.settings):
            self.cmake.test()
