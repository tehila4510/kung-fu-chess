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
set ENGINE_TEST_SOURCES=tests\test_main.cpp tests\RuleEngineTest.cpp tests\RealTimeArbiterTest.cpp tests\GameEngineTest.cpp tests\BoardMapperTest.cpp tests\ControllerTest.cpp tests\BoardIOTest.cpp tests\GraphicsBoardLayoutTest.cpp tests\GraphicsConfigTest.cpp tests\GraphicsAssetPathsTest.cpp tests\MatchQueueTest.cpp src\server\MatchQueue.cpp %ENGINE_SOURCES%
set GRAPHICS_STL_SOURCES=src\graphics\AssetPaths.cpp src\graphics\BoardLayout.cpp src\graphics\FileBoardSource.cpp src\graphics\BoardLayoutLoader.cpp src\graphics\FileConfigSource.cpp src\graphics\GraphicsConfigLoader.cpp
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if /i "%~1"=="server" (
    if not exist third_party\websocketpp\websocketpp\server.hpp (
        echo ERROR: third_party\websocketpp not found.
        echo Run the clone commands from README.md "Server dependencies" section.
        exit /b 1
    )
    if not exist third_party\asio\asio\include\asio.hpp (
        echo ERROR: third_party\asio not found ^(need tag asio-1-28-0 layout^).
        echo Run the clone commands from README.md "Server dependencies" section.
        exit /b 1
    )
    if not exist third_party\json\single_include\nlohmann\json.hpp (
        echo ERROR: third_party\json not found.
        echo Run the clone commands from README.md "Server dependencies" section.
        exit /b 1
    )
    if not exist third_party\sqlite\sqlite3.c (
        echo ERROR: third_party\sqlite amalgamation not found.
        echo Run the download commands from README.md "Server dependencies" section.
        exit /b 1
    )
    if not exist third_party\bcrypt\src\bcrypt.c (
        echo ERROR: third_party\bcrypt not found.
        echo Run the clone/copy commands from README.md "Server dependencies" section.
        exit /b 1
    )

    set GCC=%GPP:g++.exe=gcc.exe%
    if not exist "!GCC!" (
        where gcc >nul 2>&1
        if errorlevel 1 (
            echo gcc not found. Needed to compile sqlite3.c / bcrypt C sources.
            exit /b 1
        )
        set GCC=gcc
    )

    echo Compiling sqlite3...
    "!GCC!" -c third_party\sqlite\sqlite3.c -o build\sqlite3.o -O2 -DSQLITE_THREADSAFE=1
    if errorlevel 1 exit /b 1

    echo Compiling bcrypt...
    "!GCC!" -c third_party\bcrypt\src\bcrypt.c -o build\bcrypt.o -O2 -Ithird_party\bcrypt\include
    if errorlevel 1 exit /b 1
    "!GCC!" -c third_party\bcrypt\src\crypt_blowfish.c -o build\crypt_blowfish.o -O2 -Ithird_party\bcrypt\include
    if errorlevel 1 exit /b 1
    "!GCC!" -c third_party\bcrypt\src\crypt_gensalt.c -o build\crypt_gensalt.o -O2 -Ithird_party\bcrypt\include
    if errorlevel 1 exit /b 1
    "!GCC!" -c third_party\bcrypt\src\wrapper.c -o build\bcrypt_wrapper.o -O2 -Ithird_party\bcrypt\include
    if errorlevel 1 exit /b 1

    set SERVER_SOURCES=src\server_main.cpp src\server\WebSocketServer.cpp src\server\MatchQueue.cpp src\server\MatchRoom.cpp src\server\GameSession.cpp src\bus\EventBus.cpp src\bus\GameEvent.cpp src\bus\MoveLogSubscriber.cpp src\bus\SoundSubscriber.cpp src\bus\RatingSubscriber.cpp src\auth\UserRepository.cpp src\auth\UserService.cpp src\auth\AuthController.cpp src\auth\PasswordHasher.cpp src\auth\Elo.cpp src\protocol\Algebraic.cpp src\protocol\CommandParser.cpp src\protocol\StateSerializer.cpp src\model\Cell.cpp src\model\Board.cpp src\model\GameState.cpp src\model\Piece.cpp src\model\Position.cpp src\rules\PieceRules.cpp src\rules\RuleEngine.cpp src\realtime\RealTimeArbiter.cpp src\engine\GameEngine.cpp src\io\BoardParser.cpp
    set SERVER_FLAGS=-std=c++17 -Iinclude -Ithird_party\websocketpp -Ithird_party\asio\asio\include -Ithird_party\json\single_include -Ithird_party\sqlite -Ithird_party\bcrypt\include -DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ -D_WEBSOCKETPP_CPP11_THREAD_ -D_WIN32_WINNT=0x0601 -Wall -Wextra -Wpedantic
    rem Use !var! — %%var%% is empty inside this if-block (EnableDelayedExpansion).
    "%GPP%" !SERVER_FLAGS! !SERVER_SOURCES! build\sqlite3.o build\bcrypt.o build\crypt_blowfish.o build\crypt_gensalt.o build\bcrypt_wrapper.o -o build\KungFuChessServer.exe -lws2_32 -lwsock32 -lwinmm -ladvapi32
    if errorlevel 1 exit /b 1
    echo Built: build\KungFuChessServer.exe
    echo Listening on ws://localhost:9002 — press Ctrl+C to stop.
    echo.
    build\KungFuChessServer.exe
    exit /b %errorlevel%
)

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
    set GRAPHICS_ENGINE_SOURCES=src\model\Cell.cpp src\model\Board.cpp src\model\GameState.cpp src\model\Piece.cpp src\model\Position.cpp src\rules\PieceRules.cpp src\rules\RuleEngine.cpp src\realtime\RealTimeArbiter.cpp src\engine\GameEngine.cpp src\input\BoardMapper.cpp src\input\Controller.cpp src\bus\EventBus.cpp src\bus\GameEvent.cpp src\bus\SoundSubscriber.cpp src\bus\MoveHistorySubscriber.cpp src\protocol\Algebraic.cpp
    set GRAPHICS_SOURCES=src\graphics_main.cpp src\GraphicsApplication.cpp src\view\Img.cpp src\view\Renderer.cpp src\graphics\Animation.cpp src\graphics\AnimationLoader.cpp src\graphics\AnimationCache.cpp src\graphics\PieceVisual.cpp src\graphics\GraphicsConfigLoader.cpp src\graphics\AssetPaths.cpp src\graphics\BoardLayout.cpp src\graphics\FileFrameSource.cpp src\graphics\FileBoardSource.cpp src\graphics\FileConfigSource.cpp src\graphics\BoardLayoutLoader.cpp !GRAPHICS_ENGINE_SOURCES!
    cl /nologo /EHsc /std:c++17 /Iinclude /I!OPENCV_INC! /Fo:build\ !GRAPHICS_SOURCES! /Fe:build\KungFuChessGraphics.exe /link /LIBPATH:!OPENCV_BIN! opencv_world451.lib user32.lib gdi32.lib winmm.lib
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
