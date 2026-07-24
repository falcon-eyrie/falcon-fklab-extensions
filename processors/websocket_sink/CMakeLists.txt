add_library(websocket_sink STATIC "websocket_sink.cpp")

FetchContent_Declare(
    uWebSockets
    URL https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.79.0.zip
)

FetchContent_Declare(
    uSockets
    URL https://github.com/uNetworking/uSockets/archive/refs/tags/v0.8.8.zip
)

FetchContent_MakeAvailable(uWebSockets uSockets)

target_compile_definitions(websocket_sink PUBLIC LIBUS_USE_OPENSSL)

target_include_directories(websocket_sink PUBLIC
    ${uwebsockets_SOURCE_DIR}/src
    ${usockets_SOURCE_DIR}/src
)

target_sources(websocket_sink PRIVATE
    ${usockets_SOURCE_DIR}/src/bsd.c
    ${usockets_SOURCE_DIR}/src/context.c
    ${usockets_SOURCE_DIR}/src/loop.c
    ${usockets_SOURCE_DIR}/src/socket.c
    ${usockets_SOURCE_DIR}/src/eventing/epoll_kqueue.c
    ${usockets_SOURCE_DIR}/src/crypto/openssl.c
    ${usockets_SOURCE_DIR}/src/crypto/sni_tree.cpp
)

target_link_libraries(websocket_sink PUBLIC z ssl crypto)
