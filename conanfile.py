import os
import json
from conan import ConanFile, tools
from conan.tools.cmake import CMake, cmake_layout

BOOST_COMPONENTS = [
    "atomic",
    "chrono",
    "container",
    "context",
    "contract",
    "coroutine",
    "date_time",
    "exception",
    "fiber",
    "filesystem",
    "graph",
    "graph_parallel",
    "iostreams",
    "json",
    "locale",
    "log",
    "math",
    "mpi",
    "nowide",
    "program_options",
    "python",
    "random",
    "regex",
    "serialization",
    "stacktrace",
    "system",
    "test",
    "thread",
    "timer",
    "type_erasure",
    "wave",
]

USED_BOOST_COMPONENTS = [
    "atomic",
    "chrono",
    "container", # required by atomic
    "date_time",
    "exception", # required by atomic
    "filesystem",
    "locale",
    "program_options",
    "random",
    "regex",
    "system", # required by "chrono"
    "thread",
]

SDK_DIR = "sdk"

class QiConan(ConanFile):
    name = "qi"
    license = "BSD-3-Clause"
    url = "https://github.com/aldebaran/libqi"
    description = "Middle-ware framework for NAOqi"

    requires = [
        "boost/[~1.78]",
        "openssl/[~3]",
    ]

    test_requires = [
        "gtest/cci.20210126",
    ]

    exports = "project.json"
    exports_sources = [
        "CMakeLists.txt",
        "project.json",
        "cmake/*",
        "qi/*",
        "ka/*",
        "src/*",
        "examples/*",
        "tests/*"
    ]

    generators = "CMakeToolchain"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "with_boost_locale": [True, False],
        "with_systemd": [True, False]
    }

    default_options = {
        "with_boost_locale": True,
        "with_systemd": False,
        "boost/*:shared": True,
        "openssl/*:shared": True,
    }

    # Disable every components of Boost unless we actively use them.
    default_options.update({
        f"boost/*:without_{_name}": False
        if _name in USED_BOOST_COMPONENTS else True
        for _name in BOOST_COMPONENTS
    })

    def init(self):
        project_cfg_file_path = os.path.join(self.recipe_folder, "project.json")
        project_cfg_data = tools.files.load(self, project_cfg_file_path)
        self.cfg = json.loads(project_cfg_data)

    def set_version(self):
        if self.version:
            return

        version = self.cfg["version"]

        # For development version, try adding a suffix with the revision SHA1.
        if version.endswith('-dev'):
            git = tools.scm.Git(self)
            try:
                revision = git.get_commit()[:8]
                if git.is_dirty():
                    revision += ".dirty"
            # Exception: most likely the recipe folder is not a Git repository (for
            # instance it is a tarball of sources), abort adding the revision.
            except Exception as ex:
                self.output.info("Cannot get version information from git repository, will use a date revision instead"
                                 + f"(exception: {ex})")
                import datetime
                now = datetime.datetime.now()
                revision = now.strftime("%Y%m%d%H%M%S")
        if revision:
            version += f".{revision}"

        self.version = version

    def layout(self):
        # Configure the format of the build folder name, based on the value of some variables.
        self.folders.build_folder_vars = [
            "settings.os",
            "settings.arch",
            "settings.compiler",
            "settings.build_type",
        ]

        # The cmake_layout() sets the folders and cpp attributes to follow the
        # structure of a typical CMake project.
        cmake_layout(self)

        # Project root directory is an include directory.
        qi_src = self.cpp.source.components["qi"]
        qi_src.includedirs = [""]

        ka_src = self.cpp.source.components["ka"]
        ka_src.includedirs = [""]

        testssl_src = self.cpp.source.components["testssl"]
        testssl_src.includedirs = [""]
        testssl_src.includedirs = [
            os.path.join("tests", "messaging", "libtestssl")
        ]

        testsession_src = self.cpp.source.components["testsession"]
        testsession_src.includedirs = [
            os.path.join("tests", "messaging", "libtestsession")
        ]

        qi_build = self.cpp.build.components["qi"]
        qi_build.includedirs = [""] # for generated headers
        qi_build.builddirs = [""]

        testssl_build = self.cpp.build.components["testssl"]
        testsession_build = self.cpp.build.components["testsession"]

        libdir = os.path.join(SDK_DIR, "lib")
        for comp in qi_build, testssl_build, testsession_build:
            comp.libdirs = [libdir]

    def validate(self):
        # Require at least C++17
        if self.settings.compiler.cppstd:
            tools.build.check_min_cppstd(self, "17", gnu_extensions=True)

    def build(self):
        cmake = CMake(self)
        cmake.configure(
            variables={
                "QI_VERSION": self.version,
                "WITH_BOOST_LOCALE": self.options.with_boost_locale,
                "WITH_SYSTEMD": self.options.with_systemd,
            }
        )
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install(component="runtime")
        cmake.install(component="devel")

    def package_info(self):
        # Rename the default global target.
        self.cpp_info.set_property("cmake_target_name", "qi::all_libraries")

        # ka
        ka = self.cpp_info.components["ka"]
        ka.libs = []
        ka.requires = [
            "boost::headers",
            "boost::date_time",
            "boost::system",
            "boost::thread"
        ]

        # qi
        qi = self.cpp_info.components["qi"]
        qi.libs = ["qi"]
        qi.requires = [
            "ka",
            "boost::headers",
            "boost::atomic",
            "boost::chrono",
            "boost::date_time",
            "boost::filesystem",
            "boost::program_options",
            "boost::random",
            "boost::regex",
            "boost::thread",
            "openssl::crypto",
            "openssl::ssl"
        ]

        # testssl
        testssl = self.cpp_info.components["testssl"]
        testssl.libs = ["testssl"]
        testssl.requires = ["qi"]

        # testsession
        testsession = self.cpp_info.components["testsession"]
        testsession.libs = ["testsession"]
        testsession.requires = ["qi", "testssl"]

        # qimodule
        #
        # The builddirs needs to be set on a component, even though it should
        # belong to the entire package. This is because if set on the cpp_info
        # member directly, Conan does not process it. By default, we choose to
        # set it on the "qi" component.
        #
        # Setting the builddirs enables consumers to find the qimodule package
        # configuration file through find_package when using the CMakeToolchain
        # generator (it adds the directories to the CMAKE_PREFIX_PATH).
        #
        # Adding the package_folder to the builddirs does not work when not in
        # the build directory because Conan automatically and silently ignores
        # that value, so we have to set the subdirectory to the qimodule
        # package configuration file as the builddir instead.
        qi.builddirs.append(os.path.join("lib", "cmake", "qimodule"))
