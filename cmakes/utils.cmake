
include_guard()

include(FetchContent)

macro(parse_var arg key value)
    string(REGEX REPLACE "^(.+)=(.+)$" "\\1;\\2" REGEX_RESULT ${arg})
    list(GET REGEX_RESULT 0 ${key})
    list(GET REGEX_RESULT 1 ${value})
endmacro()

function(FetchContent_MakeAvailableWithArgs dep)
    MESSAGE("fetching dependency : ${dep}")
    if (NOT ${dep}_POPULATED)
        # 这个命令应当在FetchContent_Declare(${dep})后被调用
        # Declare仅仅是声明依赖，这个方法才会真正下载依赖
        # 这个方法可以定义${dep}_POPULATED、${dep}_SOURCE_DIR、${dep}_BINARY_DIR 三个变量
        FetchContent_Populate(${dep})
        foreach (arg IN LISTS ARGN)
            parse_var(${arg} key value)
            set(${key}_OLD ${${key}})
            set(${key} ${value} CACHE INTERNAL "")
        endforeach ()

        # EXCLUDE_FROM_ALL 代表将${dep}_SOURCE_DIR排除在all target之外
        if (EXISTS ${${dep}_SOURCE_DIR}/CMakeLists.txt)
            add_subdirectory(${${dep}_SOURCE_DIR} ${${dep}_BINARY_DIR} EXCLUDE_FROM_ALL)
        else ()
            MESSAGE("need to manually install ${dep}")
        endif ()


        foreach (arg IN LISTS ARGN)
            parse_var(${arg} key value)
            set(${key} ${${key}_OLD} CACHE INTERNAL "")
        endforeach ()
    endif ()
endfunction()


function(FetchContent_DeclareWithMirror dep url hash)
    FetchContent_Declare(${dep}
            URL ${DEPS_FETCH_PROXY}${url}
            URL_HASH ${hash}
            )
endfunction()


function(FetchContent_DeclareWithMirror_Nochecksum dep url)
    FetchContent_Declare(${dep}
            URL ${DEPS_FETCH_PROXY}${url}
            )
endfunction()

function(FetchContent_DeclareGitHubWithMirror dep repo tag hash)
    FetchContent_DeclareWithMirror(${dep}
            https://github.com/${repo}/archive/${tag}.zip
            ${hash}
            )
endfunction()

function(FetchContent_DeclareGitHubWithMirror_Nochecksum dep repo tag)
    FetchContent_DeclareWithMirror_Nochecksum(${dep}
            https://github.com/${repo}/archive/${tag}.zip
            )
endfunction()
