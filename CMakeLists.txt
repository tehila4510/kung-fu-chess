cmake_minimum_required(VERSION 3.10)
project(KungFuChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests)

# Legacy Board/Game/MoveRules sources have been removed. The engine layer
# (model/rules/realtime/engine/io/input) backs both the app and the tests.
set(ENGINE_SOURCES
    src/model/Cell.cpp
    src/model/Board.cpp
    src/model/GameState.cpp
    src/model/Piece.cpp
    src/model/Position.cpp
    src/rules/PieceRules.cpp
    src/rules/RuleEngine.cpp
    src/realtime/RealTimeArbiter.cpp
    src/engine/GameEngine.cpp
    src/io/BoardParser.cpp
    src/io/BoardPrinter.cpp
    src/input/BoardMapper.cpp
    src/input/Controller.cpp
    src/texttests/ScriptParser.cpp
    src/texttests/ScriptRunner.cpp
)

# App executable: composition root in src/main.cpp wires the layers together.
add_executable(KungFuChess
    src/main.cpp
    ${ENGINE_SOURCES}
)

set(GRAPHICS_STL_SOURCES
    src/graphics/AssetPaths.cpp
    src/graphics/BoardLayout.cpp
    src/graphics/FileBoardSource.cpp
    src/graphics/BoardLayoutLoader.cpp
    src/graphics/FileConfigSource.cpp
    src/graphics/GraphicsConfigLoader.cpp
)

add_executable(KungFuChessEngineTests
    tests/test_main.cpp
    tests/RuleEngineTest.cpp
    tests/RealTimeArbiterTest.cpp
    tests/GameEngineTest.cpp
    tests/BoardMapperTest.cpp
    tests/ControllerTest.cpp
    tests/BoardIOTest.cpp
    tests/GraphicsBoardLayoutTest.cpp
    tests/GraphicsConfigTest.cpp
    tests/GraphicsAssetPathsTest.cpp
    tests/MatchQueueTest.cpp
    src/server/MatchQueue.cpp
    ${ENGINE_SOURCES}
    ${GRAPHICS_STL_SOURCES}
)

enable_testing()
add_test(NAME KungFuChessEngineTests COMMAND KungFuChessEngineTests)

# --- WebSocket server (optional third_party deps) -----------------------------
set(WEBSOCKETPP_DIR "${PROJECT_SOURCE_DIR}/third_party/websocketpp")
set(ASIO_DIR "${PROJECT_SOURCE_DIR}/third_party/asio/asio/include")
set(JSON_DIR "${PROJECT_SOURCE_DIR}/third_party/json/single_include")
set(SQLITE_DIR "${PROJECT_SOURCE_DIR}/third_party/sqlite")
set(BCRYPT_DIR "${PROJECT_SOURCE_DIR}/third_party/bcrypt")

if(EXISTS "${WEBSOCKETPP_DIR}/websocketpp/server.hpp"
   AND EXISTS "${ASIO_DIR}/asio.hpp"
   AND EXISTS "${JSON_DIR}/nlohmann/json.hpp"
   AND EXISTS "${SQLITE_DIR}/sqlite3.c"
   AND EXISTS "${BCRYPT_DIR}/src/bcrypt.c")
    add_library(sqlite3_amalgamation STATIC "${SQLITE_DIR}/sqlite3.c")
    target_include_directories(sqlite3_amalgamation PUBLIC "${SQLITE_DIR}")
    target_compile_definitions(sqlite3_amalgamation PRIVATE SQLITE_THREADSAFE=1)

    add_library(bcrypt_vendored STATIC
        "${BCRYPT_DIR}/src/bcrypt.c"
        "${BCRYPT_DIR}/src/crypt_blowfish.c"
        "${BCRYPT_DIR}/src/crypt_gensalt.c"
        "${BCRYPT_DIR}/src/wrapper.c"
    )
    target_include_directories(bcrypt_vendored PUBLIC "${BCRYPT_DIR}/include")

    add_executable(KungFuChessServer
        src/server_main.cpp
        src/server/WebSocketServer.cpp
        src/server/MatchQueue.cpp
        src/server/MatchRoom.cpp
        src/server/GameSession.cpp
        src/bus/EventBus.cpp
        src/bus/GameEvent.cpp
        src/bus/MoveLogSubscriber.cpp
        src/bus/SoundSubscriber.cpp
        src/bus/RatingSubscriber.cpp
        src/auth/UserRepository.cpp
        src/auth/UserService.cpp
        src/auth/AuthController.cpp
        src/auth/PasswordHasher.cpp
        src/auth/Elo.cpp
        src/protocol/Algebraic.cpp
        src/protocol/CommandParser.cpp
        src/protocol/StateSerializer.cpp
        src/model/Cell.cpp
        src/model/Board.cpp
        src/model/GameState.cpp
        src/model/Piece.cpp
        src/model/Position.cpp
        src/rules/PieceRules.cpp
        src/rules/RuleEngine.cpp
        src/realtime/RealTimeArbiter.cpp
        src/engine/GameEngine.cpp
        src/io/BoardParser.cpp
    )
    target_include_directories(KungFuChessServer PRIVATE
        ${WEBSOCKETPP_DIR}
        ${ASIO_DIR}
        ${JSON_DIR}
        ${SQLITE_DIR}
        ${BCRYPT_DIR}/include
    )
    target_compile_definitions(KungFuChessServer PRIVATE
        ASIO_STANDALONE
        _WEBSOCKETPP_CPP11_STL_
        _WEBSOCKETPP_CPP11_THREAD_
        _WIN32_WINNT=0x0601
    )
    target_link_libraries(KungFuChessServer PRIVATE sqlite3_amalgamation bcrypt_vendored)
    if(WIN32)
        target_link_libraries(KungFuChessServer PRIVATE ws2_32 wsock32 winmm advapi32)
    endif()
else()
    message(STATUS "Server third_party deps missing - skipping KungFuChessServer (see README)")
endif()

# --- Graphics target (OpenCV) -------------------------------------------------
# This is the ONLY target that depends on OpenCV. It must be built with MSVC,
# because the bundled OpenCV_451 (opencv_world451.lib/.dll) is an MSVC build and
# cannot be linked by MinGW g++. The console app + tests above stay STL-only and
# are still built by build.bat (MinGW). Configure with, e.g.:
#   cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64
#   cmake --build build-msvc --config Debug
#
# Place the downloaded OpenCV_451 folder at ${OPENCV_DIR} (see cpp/README of the
# CTD26 starter). The target is skipped automatically if it is not present.
set(OPENCV_DIR "${PROJECT_SOURCE_DIR}/OpenCV_451" CACHE PATH "Bundled OpenCV root")

if(EXISTS "${OPENCV_DIR}/include")
    set(GRAPHICS_ENGINE_SOURCES
        src/model/Cell.cpp
        src/model/Board.cpp
        src/model/GameState.cpp
        src/model/Piece.cpp
        src/model/Position.cpp
        src/rules/PieceRules.cpp
        src/rules/RuleEngine.cpp
        src/realtime/RealTimeArbiter.cpp
        src/engine/GameEngine.cpp
        src/input/BoardMapper.cpp
        src/input/Controller.cpp
        src/bus/EventBus.cpp
        src/bus/GameEvent.cpp
        src/bus/SoundSubscriber.cpp
        src/bus/MoveHistorySubscriber.cpp
        src/protocol/Algebraic.cpp
    )
    add_executable(KungFuChessGraphics
        src/graphics_main.cpp
        src/GraphicsApplication.cpp
        src/view/Img.cpp
        src/view/Renderer.cpp
        src/graphics/Animation.cpp
        src/graphics/AnimationLoader.cpp
        src/graphics/AnimationCache.cpp
        src/graphics/PieceVisual.cpp
        src/graphics/GraphicsConfigLoader.cpp
        src/graphics/AssetPaths.cpp
        src/graphics/BoardLayout.cpp
        src/graphics/FileFrameSource.cpp
        src/graphics/FileBoardSource.cpp
        src/graphics/FileConfigSource.cpp
        src/graphics/BoardLayoutLoader.cpp
        ${GRAPHICS_ENGINE_SOURCES}
    )
    target_include_directories(KungFuChessGraphics PRIVATE "${OPENCV_DIR}/include")
    target_link_directories(KungFuChessGraphics PRIVATE "${OPENCV_DIR}/bin")
    target_link_libraries(KungFuChessGraphics
        $<$<CONFIG:Debug>:${OPENCV_DIR}/bin/opencv_world451d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${OPENCV_DIR}/bin/opencv_world451.lib>
    )
    if(WIN32)
        target_link_libraries(KungFuChessGraphics PRIVATE winmm)
        add_custom_command(TARGET KungFuChessGraphics POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<$<CONFIG:Debug>:${OPENCV_DIR}/bin/opencv_world451d.dll>"
                "$<$<NOT:$<CONFIG:Debug>>:${OPENCV_DIR}/bin/opencv_world451.dll>"
                "$<TARGET_FILE_DIR:KungFuChessGraphics>")
    endif()

    add_executable(KungFuChessGraphicsTests
        tests/test_main.cpp
        tests/GraphicsAnimationTest.cpp
        tests/GraphicsAnimationCacheTest.cpp
        tests/GraphicsPieceVisualTest.cpp
        tests/GraphicsImgTest.cpp
        tests/GraphicsRendererTest.cpp
        tests/GraphicsFileFrameSourceTest.cpp
        src/view/Img.cpp
        src/view/Renderer.cpp
        src/graphics/Animation.cpp
        src/graphics/AnimationLoader.cpp
        src/graphics/AnimationCache.cpp
        src/graphics/PieceVisual.cpp
        src/graphics/GraphicsConfigLoader.cpp
        src/graphics/AssetPaths.cpp
        src/graphics/BoardLayout.cpp
        src/graphics/FileFrameSource.cpp
        src/graphics/FileBoardSource.cpp
        src/graphics/FileConfigSource.cpp
        src/graphics/BoardLayoutLoader.cpp
        ${GRAPHICS_ENGINE_SOURCES}
    )
    target_include_directories(KungFuChessGraphicsTests PRIVATE "${OPENCV_DIR}/include")
    target_link_directories(KungFuChessGraphicsTests PRIVATE "${OPENCV_DIR}/bin")
    target_link_libraries(KungFuChessGraphicsTests
        $<$<CONFIG:Debug>:${OPENCV_DIR}/bin/opencv_world451d.lib>
        $<$<NOT:$<CONFIG:Debug>>:${OPENCV_DIR}/bin/opencv_world451.lib>
    )
    if(WIN32)
        add_custom_command(TARGET KungFuChessGraphicsTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<$<CONFIG:Debug>:${OPENCV_DIR}/bin/opencv_world451d.dll>"
                "$<$<NOT:$<CONFIG:Debug>>:${OPENCV_DIR}/bin/opencv_world451.dll>"
                "$<TARGET_FILE_DIR:KungFuChessGraphicsTests>")
    endif()
    add_test(NAME KungFuChessGraphicsTests COMMAND KungFuChessGraphicsTests)
else()
    message(STATUS "OpenCV_451 not found at ${OPENCV_DIR} - skipping KungFuChessGraphics target")
endif()
