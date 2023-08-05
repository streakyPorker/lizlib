include_guard()

include(cmakes/utils.cmake)

FetchContent_DeclareGitHubWithMirror_Nochecksum(liburing
        axboe/liburing liburing-2.4
        )

FetchContent_MakeAvailableWithArgs(liburing)
