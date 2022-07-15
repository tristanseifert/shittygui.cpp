/**
 * @file
 *
 * @brief SDL2 output example
 */
#include <shittygui/Screen.h>
#include <SDL.h>

#include <atomic>
#include <cstdio>
#include <iostream>
#include <memory>

/// Window dimensions
constexpr static const shittygui::Size kWindowSize{800, 480};

/// Flag set for as long as we should keep running the UI task
std::atomic_bool gRun{true};

/**
 * Application entry point
 */
int main(const int argc, const char **argv) {
    std::shared_ptr<shittygui::Screen> screen;

    // get a SDL window and renderer set up
    SDL_Init(SDL_INIT_VIDEO);

    auto window = SDL_CreateWindow("ShittyGUI Test", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, kWindowSize.width, kWindowSize.height, 0);
    if(!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if(!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // set up GUI library
    screen = std::make_shared<shittygui::Screen>(shittygui::Screen::PixelFormat::ARGB32,
            kWindowSize);
    printf("framebuffer: %p (stride %lu bytes)\n", screen->getBuffer(), screen->getBufferStride());

    screen->setBackgroundColor({0, 0.33, 0});

    // set up texture to render into
    const auto &physSize = screen->getFramebufferSize();
    auto inTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
            physSize.width, physSize.height);

    if(!inTex) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    /*
     * Main loop: wait for an event to arrive (in the form of user input or other events) and then
     * handle that. After each event is handled, we query the screen to see if it's been dirtied,
     * and if so, redraw.
     *
     * In a real application, you would want to synchronize drawing with the underlying display
     * and coalesce all event updates within a single frame into a single draw call. Not doing this
     * here simplifies the code considerably, but may lead to various graphical artifacts.
     */
    while(gRun) {
        SDL_Event e;
        if(!SDL_WaitEvent(&e)) {
            std::cerr << "SDL_WaitEvent failed: " << SDL_GetError() << std::endl;
            return 1;
        }

        // process the event
        switch(e.type) {
            // TODO: mouse movement
            // TODO: mouse buttons
            // TODO: key events

            // terminate the application
            case SDL_QUIT:
                gRun = false;
                break;
        }

        // redraw the screen, if it indicates that it's dirty. this is slow and shitty (lol)
        if(screen->isDirty()) {
            screen->redraw();

            SDL_UpdateTexture(inTex, nullptr, screen->getBuffer(), screen->getBufferStride());

            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, inTex, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
    }

    // clean up
    screen.reset();

    SDL_DestroyTexture(inTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
