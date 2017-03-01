
from conans import ConanFile, CMake, tools
import os


class BoostGraphWrapperConan(ConanFile):
    name = "boost-graph-wrapper"
    version = "0.0"
    license = "MIT"
    url = "<Package recipe repository url here, for issues about the package>"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports = "conanfile.py", "CMakeLists.txt", "boost-graph-wrapper/*", "tests/*"

    def requirements(self):
        self.requires.add("Boost/1.60.0@lasote/stable")
        self.requires.add("spdlog/0.9.0@memsharded/stable")

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin") # From bin to bin
        self.copy("*.dylib*", dst="bin", src="lib") # From lib to bin

    def build(self):
        cmake = CMake(self.settings)
        shared = "-DBUILD_SHARED_LIBS=ON" if self.options.shared else ""
        build_tests = "-DBUILD_TEST:BOOL=ON" if self.scope.BUILD_TEST else ""

        self.run('cmake "%s" %s %s %s' % (self.conanfile_directory, cmake.command_line, build_tests, build_tests))
        self.run("cmake --build . %s" % cmake.build_config)
        if build_tests:
            self.run("ctest -C {}".format(self.settings.build_type))

    def package(self):
        self.copy("*.h", dst="include", src="boost-graph-wrapper")
        self.copy("*boost-graph-wrapper.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["boost-graph-wrapper"]

