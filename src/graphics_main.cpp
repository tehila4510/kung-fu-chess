// Graphics entry point: load assets, start the interactive board application.

#include "GraphicsApplication.h"
#include "graphics/AssetPaths.h"
#include "graphics/BoardLayoutLoader.h"
#include "view/Img.h"

#include <iostream>
#include <stdexcept>

int main() {
    try {
        const graphics::BoardLayout layout =
            graphics::BoardLayoutLoader{}.load(graphics::AssetPaths::boardCsv());

        view::Img background;
        background.read(graphics::AssetPaths::boardImage());

        GraphicsApplication app(layout, std::move(background));
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        view::Img::destroyWindows();
        return 1;
    }
}
