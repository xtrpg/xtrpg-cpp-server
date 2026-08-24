cmake_minimum_required(VERSION 3.20)

# Auto-detect vcpkg if VCPKG_ROOT environment variable is set
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# If no variable is passed from the environment/command line, use a fallback
if(NOT DEFINED BUILD_VERSION)
    set(BUILD_VERSION "0.0.0")
endif()

# Separate the SemVer string into major, minor, patch parts
string(REPLACE "." ";" VERSION_LIST ${BUILD_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

project(xtrpg_cpp_server VERSION ${BUILD_VERSION} LANGUAGES CXX)

# Generate the actual hpp file into the build binary directory
configure_file("generated/version.hpp.in" ${CMAKE_BINARY_DIR}/generated/version.hpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_XMPP_JID "Build XMPP JID support" ON)

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
# option(ENABLE_STORAGE_MEMORY "Include in-memory storage backend" ON)
# option(ENABLE_STORAGE_SQLITE "Include SQLite storage backend" OFF)
# option(ENABLE_STORAGE_POSTGRES "Include PostgreSQL storage backend" OFF)

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

# Single executable target
add_executable(xtrpg_cpp_server
    apps/main.cpp
)
target_include_directories(xtrpg_cpp_server PRIVATE
    include
    ${CMAKE_BINARY_DIR}/generated
)

target_sources(xtrpg_cpp_server PRIVATE
    src/config/ConfigManager.cpp
    src/network/SocketConnectionListener.cpp
    src/network/TcpConnection.cpp
    src/xmpp/ClientConnectionManager.cpp
    src/xmpp/Jid.cpp)


target_link_libraries(xtrpg_cpp_server PRIVATE
    asio::asio
    OpenSSL::SSL
    OpenSSL::Crypto
)
if(WIN32)
    # Windows-specific OS network primitives for raw sockets
    # Define minimum Windows version (0x0601 = Windows 7, 0x0A00 = Windows 10)
    # 0x0A00 unlocks modern Windows socket features for Asio
    target_compile_definitions(xtrpg_cpp_server PRIVATE
        _WIN32_WINNT=0x0A00
        WINVER=0x0A00
    )
    target_link_libraries(xtrpg_cpp_server PRIVATE ws2_32 wsock32 crypt32)
elseif(UNIX AND NOT APPLE)
    # Platform-specific OS network primitives for raw sockets
    target_link_libraries(xtrpg_cpp_server PRIVATE pthread dl)
endif()

# If modules produce targets registered via xmpp_register_user_module
if(USER_MODULE_TARGETS)
    target_link_libraries(xtrpg_cpp_server PRIVATE ${USER_MODULE_TARGETS})
endif()

# Instruct MSVC to read and output source code strings as UTF-8
if(MSVC)
    target_compile_options(xtrpg_cpp_server PRIVATE /utf-8)
endif()
