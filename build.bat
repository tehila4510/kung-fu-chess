@echo off
setlocal EnableDelayedExpansion

set GPP=C:\msys64\mingw64\bin\g++.exe
if not exist "%GPP%" (
    where g++ >nul 2>&1
    if errorlevel 1 (
        echo g++ not found. Install MSYS2/MinGW or add g++ to PATH.
        exit /b 1
    )
    set GPP=g++
)

if not exist build mkdir build

set COMMON_FLAGS=-std=c++17 -Iinclude -Itests -Wall -Wextra -Wpedantic

rem Legacy Board/Game/MoveRules sources have been removed. The engine layer
rem (model/rules/realtime/engine/io/input) backs both the app and the tests.
set ENGINE_SOURCES=src\model\Cell.cpp src\model\Board.cpp src\model\GameState.cpp src\model\Piece.cpp src\model\Position.cpp src\rules\PieceRules.cpp src\rules\RuleEngine.cpp src\realtime\RealTimeArbiter.cpp src\engine\GameEngine.cpp src\io\BoardParser.cpp src\io\BoardPrinter.cpp src\input\BoardMapper.cpp src\input\Controller.cpp src\texttests\ScriptParser.cpp src\texttests\ScriptRunner.cpp
set ENGINE_TEST_SOURCES=tests\test_main.cpp tests\RuleEngineTest.cpp tests\RealTimeArbiterTest.cpp tests\GameEngineTest.cpp tests\BoardMapperTest.cpp tests\ControllerTest.cpp tests\BoardIOTest.cpp tests\GraphicsBoardLayoutTest.cpp tests\GraphicsConfigTest.cpp tests\GraphicsAssetPathsTest.cpp %ENGINE_SOURCES%
set GRAPHICS_STL_SOURCES=src\graphics\AssetPaths.cpp src\graphics\BoardLayout.cpp src\graphics\FileBoardSource.cpp src\graphics\BoardLayoutLoader.cpp src\graphics\FileConfigSource.cpp src\graphics\GraphicsConfigLoader.cpp
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if /i "%~1"=="test" (
    "%GPP%" %COMMON_FLAGS% %ENGINE_TEST_SOURCES% %GRAPHICS_STL_SOURCES% -o build\KungFuChessEngineTests.exe
    if errorlevel 1 exit /b 1
    echo Built: build\KungFuChessEngineTests.exe
    echo.
    build\KungFuChessEngineTests.exe
    exit /b %errorlevel%
)

if /i "%~1"=="graphics-test" (
    set OPENCV_INC=OpenCV_451\include
    set OPENCV_BIN=OpenCV_451\bin

    if not exist "!OPENCV_INC!\opencv2\opencv.hpp" (
        echo ERROR: OpenCV headers not found at !OPENCV_INC!
        exit /b 1
    )
    if not exist "!OPENCV_BIN!\opencv_world451.lib" (
        echo ERROR: OpenCV library not found at !OPENCV_BIN!\opencv_world451.lib
        exit /b 1
    )

    set VCVARS=
    if exist "%VSWHERE%" (
        for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set VSINSTALL=%%i
        if defined VSINSTALL (
            if exist "!VSINSTALL!\VC\Auxiliary\Build\vcvars64.bat" set VCVARS=!VSINSTALL!\VC\Auxiliary\Build\vcvars64.bat
        )
    )
    if not defined VCVARS (
        echo ERROR: Visual Studio C++ toolchain not found.
        echo OpenCV_451 from the course starter requires MSVC ^(cl.exe^), not MinGW g++.
        exit /b 1
    )

    call "!VCVARS!"
    set GRAPHICS_TEST_ENGINE_SOURCES=src\model\Cell.cpp src\model\Board.cpp src\model\GameState.cpp src\model\Piece.cpp src\model\Position.cpp src\rules\PieceRules.cpp src\rules\RuleEngine.cpp src\realtime\RealTimeArbiter.cpp src\engine\GameEngine.cpp src\input\BoardMapper.cpp src\input\Controller.cpp
    set GRAPHICS_TEST_SOURCES=tests\test_main.cpp tests\GraphicsAnimationTest.cpp tests\GraphicsAnimationCacheTest.cpp tests\GraphicsPieceVisualTest.cpp tests\GraphicsImgTest.cpp tests\GraphicsRendererTest.cpp tests\GraphicsFileFrameSourceTest.cpp src\view\Img.cpp src\view\Renderer.cpp src\graphics\Animation.cpp src\graphics\AnimationLoader.cpp src\graphics\AnimationCache.cpp src\graphics\PieceVisual.cpp src\graphics\GraphicsConfigLoader.cpp src\graphics\AssetPaths.cpp src\graphics\BoardLayout.cpp src\graphics\FileFrameSource.cpp src\graphics\FileBoardSource.cpp src\graphics\FileConfigSource.cpp src\graphics\BoardLayoutLoader.cpp !GRAPHICS_TEST_ENGINE_SOURCES!
    cl /nologo /EHsc /std:c++17 /Iinclude /Itests /I!OPENCV_INC! /Fo:build\ !GRAPHICS_TEST_SOURCES! /Fe:build\KungFuChessGraphicsTests.exe /link /LIBPATH:!OPENCV_BIN! opencv_world451.lib user32.lib gdi32.lib
    if errorlevel 1 exit /b 1
    copy /Y "!OPENCV_BIN!\opencv_world451.dll" build\ >nul
    echo Built: build\KungFuChessGraphicsTests.exe
    echo.
    build\KungFuChessGraphicsTests.exe
    exit /b %errorlevel%
)

if /i "%~1"=="graphics" (
    set OPENCV_INC=OpenCV_451\include
    set OPENCV_BIN=OpenCV_451\bin

    if not exist "!OPENCV_INC!\opencv2\opencv.hpp" (
        echo ERROR: OpenCV headers not found at !OPENCV_INC!
        echo Make sure OpenCV_451 is in the project root with include\ and bin\ folders.
        exit /b 1
    )
    if not exist "!OPENCV_BIN!\opencv_world451.lib" (
        echo ERROR: OpenCV library not found at !OPENCV_BIN!\opencv_world451.lib
        echo Download the full OpenCV_451 bundle from the course starter ^(include + bin^).
        exit /b 1
    )

    rem OpenCV from the CTD26 starter is MSVC-only; MinGW g++ cannot link it.
    set VCVARS=
    if exist "%VSWHERE%" (
        for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set VSINSTALL=%%i
        if defined VSINSTALL (
            if exist "!VSINSTALL!\VC\Auxiliary\Build\vcvars64.bat" set VCVARS=!VSINSTALL!\VC\Auxiliary\Build\vcvars64.bat
        )
    )
    if not defined VCVARS (
        echo ERROR: Visual Studio C++ toolchain not found.
        echo OpenCV_451 from the course starter requires MSVC ^(cl.exe^), not MinGW g++.
        echo Install "Desktop development with C++" in Visual Studio, then run: build.bat graphics
        exit /b 1
    )

    call "!VCVARS!"
    set GRAPHICS_ENGINE_SOURCES=src\model\Cell.cpp src\model\Board.cpp src\model\GameState.cpp src\model\Piece.cpp src\model\Position.cpp src\rules\PieceRules.cpp src\rules\RuleEngine.cpp src\realtime\RealTimeArbiter.cpp src\engine\GameEngine.cpp src\input\BoardMapper.cpp src\input\Controller.cpp
    set GRAPHICS_SOURCES=src\graphics_main.cpp src\GraphicsApplication.cpp src\view\Img.cpp src\view\Renderer.cpp src\graphics\Animation.cpp src\graphics\AnimationLoader.cpp src\graphics\AnimationCache.cpp src\graphics\PieceVisual.cpp src\graphics\GraphicsConfigLoader.cpp src\graphics\AssetPaths.cpp src\graphics\BoardLayout.cpp src\graphics\FileFrameSource.cpp src\graphics\FileBoardSource.cpp src\graphics\FileConfigSource.cpp src\graphics\BoardLayoutLoader.cpp !GRAPHICS_ENGINE_SOURCES!
    cl /nologo /EHsc /std:c++17 /Iinclude /I!OPENCV_INC! /Fo:build\ !GRAPHICS_SOURCES! /Fe:build\KungFuChessGraphics.exe /link /LIBPATH:!OPENCV_BIN! opencv_world451.lib user32.lib gdi32.lib
    if errorlevel 1 exit /b 1
    copy /Y "!OPENCV_BIN!\opencv_world451.dll" build\ >nul
    echo Built: build\KungFuChessGraphics.exe
    echo.
    build\KungFuChessGraphics.exe
    exit /b %errorlevel%
)

rem Default target: build and run the app (composition root in src\main.cpp).
"%GPP%" %COMMON_FLAGS% src\main.cpp %ENGINE_SOURCES% -o build\KungFuChess.exe
if errorlevel 1 exit /b 1
echo Built: build\KungFuChess.exe
echo.
build\KungFuChess.exe
exit /b %errorlevel%
