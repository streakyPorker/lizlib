include_guard()

include(cmakes/utils.cmake)

FetchContent_DeclareGitHubWithMirror(gtest
        google/googletest v1.13.0
        MD5=a1279c6fb5bf7d4a5e0d0b2a4adb39ac
        )

FetchContent_MakeAvailableWithArgs(gtest
        BUILD_GMOCK=OFF
        INSTALL_GTEST=OFF
        )
