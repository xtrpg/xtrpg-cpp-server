cmake_minimum_required(VERSION 3.20)

# If no variable is passed from the environment/command line, use a fallback
if(NOT DEFINED BUILD_VERSION)
    set(BUILD_VERSION "0.1.0")
endif()

# Separate the SemVer string into major, minor, patch parts
string(REPLACE "." ";" VERSION_LIST ${BUILD_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_PATCH)

project(xtrpg_cpp_server VERSION ${BUILD_VERSION} LANGUAGES CXX)

# Generate the actual hpp file into the build binary directory
configure_file("generated/version.hpp.in" ${CMAKE_BINARY_DIR}/generated/version.hpp)

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

# Core Server Target
add_library(xtrpg_core STATIC
)
target_include_directories(xtrpg_core PUBLIC include)

# Link imported targets across platforms
target_link_libraries(xtrpg_core PUBLIC
    asio::asio
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Platform-specific OS network primitives for raw sockets
if(WIN32)
    target_link_libraries(xtrpg_core PUBLIC ws2_32 wsock32 crypt32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(xtrpg_core PUBLIC pthread dl)
endif()

# Conditional Storage Compilation
if(ENABLE_STORAGE_SQLITE)
    find_package(SQLite3 REQUIRED)
    target_sources(xtrpg_core PRIVATE src/storage/SQLiteBackend.cpp)
    target_compile_definitions(xtrpg_core PUBLIC HAS_STORAGE_SQLITE)
    target_link_libraries(xtrpg_core PRIVATE SQLite::SQLite3)
endif()

if(ENABLE_STORAGE_MEMORY)
    target_sources(xtrpg_core PRIVATE src/storage/MemoryBackend.cpp)
    target_compile_definitions(xtrpg_core PUBLIC HAS_STORAGE_MEMORY)
endif()

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
target_link_libraries(xtrpg_cpp_server PRIVATE xtrpg_core)
target_include_directories(xtrpg_cpp_server PRIVATE ${CMAKE_BINARY_DIR}/generated)

# If modules produce targets registered via xmpp_register_user_module
if(USER_MODULE_TARGETS)
    target_link_libraries(xmpp_server PRIVATE ${USER_MODULE_TARGETS})
endif()
