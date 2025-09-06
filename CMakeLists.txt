cmake_minimum_required(VERSION 3.10.0)
project(webserver)
#####################################################################
# Setup the toolchain                                               #
#####################################################################
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL ON FORCE)
enable_language(C CXX ASM)

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Coverage enabled")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    else()
        message(WARNING "Coverage enabled but compiler does not support it")
    endif()
endif()

#####################################################################
# Build mongoose as a static library                                #
#####################################################################
add_library(mongoose mongoose/mongoose.c)
target_include_directories(mongoose SYSTEM PRIVATE mongoose)

#Mongoose compile options
set(MONGOOSE_CFLAGS -W -Wall -Wextra)

# Mongoose compile definitions. See https://mongoose.ws/documentation/#build-options
set(MONGOOSE_CDEFS 
    -DMG_HTTP_DIRLIST_TIME_FMT="%Y/%m/%d %H:%M:%S" -DMG_ENABLE_LINES=1 
    -DMG_ENABLE_IPV6=1 -DMG_ENABLE_SSI=1)

set(MONGOOSE_CDEFS_EXTRA -DMG_TLS=MG_TLS_BUILTIN)

target_compile_definitions(mongoose PRIVATE ${MONGOOSE_CDEFS} ${MONGOOSE_CDEFS_EXTRA})
target_compile_options(mongoose PRIVATE ${MONGOOSE_CFLAGS})

#####################################################################
# Build the project                                                 #
#####################################################################
set(SRC_FILES main.cpp)
set(PROJECT_CFLAGS -W -Wall -Wextra)

add_subdirectory(http_server)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_CFLAGS})

target_include_directories(${PROJECT_NAME} PRIVATE 
        ${CMAKE_SOURCE_DIR}/mongoose
        ${CMAKE_SOURCE_DIR}/http_server)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE mongoose
    PRIVATE http_server)

target_link_options(${PROJECT_NAME} PRIVATE
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections)

#####################################################################
# Add custom post-build commands                                    #
#####################################################################
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>)

if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND lcov --capture --directory . --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage-report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
    )
endif()