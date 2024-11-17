cmake_minimum_required(VERSION 3.10)
project(ping_adv VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(ping_core STATIC
    external/iputils/ping/ping_common.c
    external/iputils/iputils_common.c
)

set_target_properties(ping_core PROPERTIES 
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden
)

target_include_directories(ping_core PRIVATE
    external/iputils
    include
)

set(PING_CORE_OBJECTS
    ${CMAKE_CURRENT_BINARY_DIR}/libping_core.a
)

add_custom_command(
    TARGET ping_core
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Built ping_core: $<TARGET_FILE:ping_core>"
)

add_executable(ping_adv
    src/main.cpp
    src/stats/ping_stats.cpp
    src/wrapper/ping_wrapper.cpp
)

target_include_directories(ping_adv PRIVATE
    src
    external/iputils
    include
)

find_library(CAP_LIBRARY NAMES cap REQUIRED)

target_link_libraries(ping_adv PRIVATE
    ping_core
    ${CAP_LIBRARY}
    pthread
)

install(TARGETS ping_adv
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE SETUID
)

install(CODE "message(\"Installing ping_adv with internal iputils implementation\")")