add_library(uwebsockets INTERFACE)
target_link_libraries(uwebsockets INTERFACE usockets deflate ssl crypto z)
