# =====================================================================
#               test_dmini Test Application
# =====================================================================
cmake_minimum_required(VERSION 3.18)

# ======================================================================
#               test_dmini Application Configuration
# ======================================================================
# Name of the application
set(DMOD_MODULE_NAME test_dmini)

# Version is inherited from parent
if(NOT DEFINED DMOD_MODULE_VERSION)
    set(DMOD_MODULE_VERSION "0.1")
endif()

# Author
set(DMOD_AUTHOR_NAME "Patryk Kubiak")

# Stack size for the application
set(DMOD_STACK_SIZE 1024)

# ======================================================================
#               Build test_dmini Application
# ======================================================================
dmod_add_executable(${DMOD_MODULE_NAME} ${DMOD_MODULE_VERSION}
    test_dmini.c
)

# Include dmini headers
target_include_directories(${DMOD_MODULE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_BINARY_DIR}  # For dmini_defs.h
)

# Note: dmini is loaded dynamically at runtime via dmod_loader
# No static linking needed - modules communicate via DMOD API
