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
    ${ENGINE_SOURCES}
    ${GRAPHICS_STL_SOURCES}
)

enable_testing()
add_test(NAME KungFuChessEngineTests COMMAND KungFuChessEngineTests)

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
