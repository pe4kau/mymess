cmake_minimum_required(VERSION 3.20)
project(messenger_client_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Sql)

# Sources
set(SRC
    src/main.cpp
    src/ui/chatwindow.cpp
    src/ui/lightboxdialog.cpp
    src/viewmodels/chatlistmodel.cpp
    src/viewmodels/messagelistmodel.cpp
    src/db/localstore.cpp
    src/crypto/ratchetwrapper.cpp
)

set(HEADERS
    include/ui/chatwindow.h
    include/ui/lightboxdialog.h
    include/viewmodels/chatlistmodel.h
    include/viewmodels/messagelistmodel.h
    include/db/localstore.h
    include/crypto/icryptosession.h
    include/crypto/ratchetwrapper.h
    include/inetworkclient.h
    include/utils/message.h
    include/utils/chat.h
    include/thememanager.h
)

set(UI
    ui/ChatWindow.ui
)

qt_add_executable(messenger_client_gui
    ${SRC}
    ${HEADERS}
    ${UI}
    resources/resources.qrc
)

target_include_directories(messenger_client_gui PRIVATE include)

target_link_libraries(messenger_client_gui
    Qt6::Widgets
    Qt6::Sql
)

# Tests
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests
    tests/test_models.cpp
    tests/test_theming.cpp
    src/viewmodels/chatlistmodel.cpp
    src/viewmodels/messagelistmodel.cpp
    src/crypto/ratchetwrapper.cpp
    src/db/localstore.cpp
)
target_include_directories(tests PRIVATE include)
target_link_libraries(tests
    Qt6::Core
    Qt6::Sql
    gtest
    gtest_main
)
add_test(NAME run_tests COMMAND tests)
