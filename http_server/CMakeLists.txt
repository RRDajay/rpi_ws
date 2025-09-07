#####################################################################
# Build as a static library                                         #
#####################################################################
set(LIBRARY_NAME http_server)
set(SRC_FILES http_server.c)
set(PROJECT_CFLAGS -W -Wall -Wextra)

# Set output directory for the binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(${LIBRARY_NAME} ${SRC_FILES})
target_compile_options(${LIBRARY_NAME} PRIVATE ${PROJECT_CFLAGS} $<$<CONFIG:Debug>:-Og>)
target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/mongoose)
target_link_libraries(${LIBRARY_NAME} PRIVATE mongoose)

add_custom_target(COPY_RESOURCE_TO_RUNTIME_OUTPUT_DIRECTORY ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/http_server/resource
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
    COMMENT "Copying ${CMAKE_CURRENT_SOURCE_DIR}/resource to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource"
)
