add_library(websocket_sink STATIC "websocket_sink.cpp")

FetchContent_Declare(
    uWebSockets
    GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
    GIT_TAG        v20.78.0
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES_RECURSE  TRUE
)
FetchContent_MakeAvailable(uWebSockets)

target_compile_definitions(websocket_sink PUBLIC LIBUS_USE_OPENSSL)

target_include_directories(websocket_sink PUBLIC
    ${uwebsockets_SOURCE_DIR}/src
    ${uwebsockets_SOURCE_DIR}/uSockets/src
)

target_sources(websocket_sink PRIVATE
    ${uwebsockets_SOURCE_DIR}/uSockets/src/bsd.c
    ${uwebsockets_SOURCE_DIR}/uSockets/src/context.c
    ${uwebsockets_SOURCE_DIR}/uSockets/src/loop.c
    ${uwebsockets_SOURCE_DIR}/uSockets/src/socket.c
    ${uwebsockets_SOURCE_DIR}/uSockets/src/eventing/epoll_kqueue.c
    ${uwebsockets_SOURCE_DIR}/uSockets/src/crypto/openssl.c
    ${uwebsockets_SOURCE_DIR}/uSockets/src/crypto/sni_tree.cpp
)

target_link_libraries(websocket_sink PUBLIC z ssl crypto)
