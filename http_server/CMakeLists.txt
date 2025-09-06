#####################################################################
# Build as a static library                                         #
#####################################################################
set(LIBRARY_NAME http_server)
set(SRC_FILES http_server.c)
set(PROJECT_CFLAGS -W -Wall -Wextra)

add_library(${LIBRARY_NAME} ${SRC_FILES})
target_compile_options(${LIBRARY_NAME} PRIVATE ${PROJECT_CFLAGS} $<$<CONFIG:Debug>:-Og -gdwarf-2>)
target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/mongoose)
target_link_libraries(${LIBRARY_NAME} PRIVATE mongoose)