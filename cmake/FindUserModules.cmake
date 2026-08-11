# cmake/FindUserModules.cmake
#
# Finds and collects custom user modules for the XMPP server.
#
# Input Variables:
#   USER_MODULES_DIR - Additional search directory for external modules.
#
# Output Variables:
#   UserModules_FOUND         - Set to TRUE if user modules path/modules are discovered.
#   USER_MODULE_DIRS          - List of discovered user module directories.
#   USER_MODULE_TARGETS       - List of CMake targets added by user modules.

include(FindPackageHandleStandardArgs)

set(USER_MODULE_DIRS "")
set(DISCOVERED_MODULE_TARGETS "")

# Define candidate search paths
set(SEARCH_PATHS
    "${CMAKE_SOURCE_DIR}/external"
    "${USER_MODULES_DIR}"
    $ENV{USER_MODULES_DIR}
)

# Iterate through search locations and find subdirectories with a CMakeLists.txt
foreach(SEARCH_PATH IN LISTS SEARCH_PATHS)
    if(EXISTS "${SEARCH_PATH}" AND IS_DIRECTORY "${SEARCH_PATH}")
        # Search for one level of subdirectories containing a CMakeLists.txt
        file(GLOB CHILDREN RELATIVE "${SEARCH_PATH}" "${SEARCH_PATH}/*")
        foreach(CHILD ${CHILDREN})
            set(FULL_CHILD_PATH "${SEARCH_PATH}/${CHILD}")
            if(IS_DIRECTORY "${FULL_CHILD_PATH}" AND EXISTS "${FULL_CHILD_PATH}/CMakeLists.txt")
                list(APPEND USER_MODULE_DIRS "${FULL_CHILD_PATH}")
            endif()
        endforeach()
    endif()
endforeach()

# Remove duplicate paths if any
if(USER_MODULE_DIRS)
    list(REMOVE_DUPLICATES USER_MODULE_DIRS)
endif()

# Helper macro/function to register a target created inside a user module
macro(xtrpg_register_user_module TARGET_NAME)
    list(APPEND DISCOVERED_MODULE_TARGETS ${TARGET_NAME})
    message(STATUS "[XTRPG Module Found]: ${TARGET_NAME}")
endmacro()

# Set output variable
set(USER_MODULE_TARGETS ${DISCOVERED_MODULE_TARGETS} CACHE INTERNAL "Discovered User Module Targets")

find_package_handle_standard_args(UserModules
    REQUIRED_VARS USER_MODULE_DIRS
    HANDLE_COMPONENTS
)

mark_as_advanced(USER_MODULE_DIRS)