cmake_minimum_required(VERSION 3.20)
project(xtrpg_cpp_server LANGUAGES CXX)

# Auto-detect vcpkg if VCPKG_ROOT environment variable is set
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Packages - vcpkg provides standard CMake target configs
find_package(asio CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Append cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Allow user to pass custom path on CLI: cmake -B build -DUSER_MODULES_DIR="/path/to/my_modules"
set(USER_MODULES_DIR "" CACHE PATH "Directory containing custom XMPP user modules")

# Find user modules
find_package(UserModules QUIET)

# Modular Build Options
option(ENABLE_STORAGE_MEMORY "Include in-memory storage backend" ON)
option(ENABLE_STORAGE_SQLITE "Include SQLite storage backend" OFF)
option(ENABLE_STORAGE_POSTGRES "Include PostgreSQL storage backend" OFF)

# XTRPG Network Library
add_library(xtrpg_network STATIC
    src/network/SocketConnectionListener.cpp
    src/network/TcpConnection.cpp
)
target_include_directories(xtrpg_network PUBLIC include)
target_link_libraries(xtrpg_network PUBLIC
    asio::asio
    OpenSSL::SSL
    OpenSSL::Crypto
)
if(WIN32)
    # Windows-specific OS network primitives for raw sockets
    # Define minimum Windows version (0x0601 = Windows 7, 0x0A00 = Windows 10)
    # 0x0A00 unlocks modern Windows socket features for Asio
    target_compile_definitions(xtrpg_network PUBLIC
        _WIN32_WINNT=0x0A00
        WINVER=0x0A00
    )
    target_link_libraries(xtrpg_network PUBLIC ws2_32 wsock32 crypt32)
elseif(UNIX AND NOT APPLE)
    # Platform-specific OS network primitives for raw sockets
    target_link_libraries(xtrpg_network PUBLIC pthread dl)
endif()

# Core Server Target
add_library(xtrpg_core STATIC
    src/core/Jid.cpp
    src/core/StanzaRouter.cpp
)
target_include_directories(xtrpg_core PUBLIC include)

# Link imported targets across platforms

# Platform-specific OS network primitives for raw sockets

# Conditional Storage Compilation

# Include discovered modules into the build
if(UserModules_FOUND)
    message(STATUS "Integrating User Modules:")
    foreach(MODULE_DIR IN LISTS USER_MODULE_DIRS)
        get_filename_component(DIR_NAME ${MODULE_DIR} NAME)
        message(STATUS "  -> Adding module subdirectory: ${DIR_NAME}")

        # Add module directory with a unique binary build path
        add_subdirectory(${MODULE_DIR} ${CMAKE_BINARY_DIR}/external_modules/${DIR_NAME})
    endforeach()
endif()

# Executable Target
add_executable(xtrpg_cpp_server apps/main.cpp)
target_link_libraries(xtrpg_cpp_server PRIVATE xtrpg_core xtrpg_network)

# If modules produce targets registered via xmpp_register_user_module
if(USER_MODULE_TARGETS)
    target_link_libraries(xtrpg_cpp_server PRIVATE ${USER_MODULE_TARGETS})
endif()
