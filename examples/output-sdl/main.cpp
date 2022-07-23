/**
 * @file
 *
 * @brief SDL2 output example
 */
#include <shittygui/Image.h>
#include <shittygui/Screen.h>
#include <shittygui/ViewController.h>
#include <shittygui/Widgets/Button.h>
#include <shittygui/Widgets/Checkbox.h>
#include <shittygui/Widgets/Container.h>
#include <shittygui/Widgets/ImageView.h>
#include <shittygui/Widgets/Label.h>
#include <shittygui/Widgets/ProgressBar.h>

#include <SDL.h>

#include <atomic>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>

/// Window dimensions
constexpr static const shittygui::Size kWindowSize{800, 480};

/// Flag set for as long as we should keep running the UI task
std::atomic_bool gRun{true};

class SecondTestViewController: public shittygui::ViewController {
    public:
        SecondTestViewController() {
            // create outer container
            auto cont = shittygui::MakeWidget<shittygui::widgets::Container>({0, 0}, {800, 480});
            cont->setDrawsBorder(false);
            cont->setBorderRadius(0.);
            cont->setBackgroundColor({0.2, 0, 0});

            // label
            auto title = shittygui::MakeWidget<shittygui::widgets::Label>({10, 10}, {780, 50},
                    "Welcome to the chill zone");
            title->setFont("Avenir Next Italic", 24);
            title->setTextAlign(shittygui::TextAlign::Center);
            title->setTextColor({1, 1, 1});
            cont->addChild(title);

            // check boi
            auto check = shittygui::MakeWidget<shittygui::widgets::Checkbox>({10, 50}, {240, 32},
                    false, "weed smoking");
            check->setFont("Avenir Next", 18);
            check->setPushCallback([](auto whomst) {
                auto checkboi = std::dynamic_pointer_cast<shittygui::widgets::Checkbox>(whomst);
                printf("check state: %d\n", checkboi->isChecked());
            });
            cont->addChild(check);

            // icon
            auto tree = shittygui::Image::Read("./tree.png");

            cont->addChild(shittygui::MakeWidget<shittygui::widgets::ImageView>({10, 100}, {48, 48},
                    tree, shittygui::widgets::ImageView::Mode::None));

            // close btn
            auto butt = shittygui::MakeWidget<shittygui::widgets::Button>({560, 400}, {200, 38},
                    shittygui::widgets::Button::Type::Push);
            butt->setDebugLabel("'Push me' button");
            butt->setTitle("Go Away");
            butt->setPushCallback([this](auto whomst) {
                std::cout << "going away time" << std::endl;
                this->dismiss(true);
            });
            cont->addChild(butt);

            // store it as the root
            this->view = std::move(cont);
        }

        /// Get our root view
        std::shared_ptr<shittygui::Widget> &getWidget() override {
            return this->view;
        }

        /// Return a dummy title
        std::string_view getTitle() override {
            return "Test View Controller";
        }

        void viewWillAppear(const bool isAnimated) override {
            ViewController::viewWillAppear(isAnimated);
            printf("Test controller %p will appear (animated=%d)\n", this, isAnimated ? 1 : 0);
        }
        void viewDidAppear() override {
            ViewController::viewDidAppear();
            printf("Test controller %p did appear\n", this);
        }
        void viewWillDisappear(const bool isAnimated) override {
            ViewController::viewWillDisappear(isAnimated);
            printf("Test controller %p will disappear (animated=%d)\n", this, isAnimated ? 1 : 0);
        }
        void viewDidDisappear() override {
            ViewController::viewDidDisappear();
            printf("Test controller %p did disappear\n", this);
        }

    private:
        std::shared_ptr<shittygui::Widget> view;
};

class TestViewController: public shittygui::ViewController {
    public:
        /**
         * @brief Initialize the view controller
         */
        TestViewController() {
            // create outer container
            auto cont = shittygui::MakeWidget<shittygui::widgets::Container>({0, 0}, {800, 480});
            cont->setDrawsBorder(false);
            cont->setBorderRadius(0.);
            cont->setBackgroundColor({0, 0.125, 0});
            cont->setDebugLabel("Root container");

            // left container
            auto left = shittygui::MakeWidget<shittygui::widgets::Container>({20, 20}, {360, 430});
            left->setBackgroundColor({0.33, 0, 0});
            left->setDebugLabel("Left container");

            auto leftLabel = shittygui::MakeWidget<shittygui::widgets::Label>({2, 0}, {356, 45},
                    "Hello <i>World</i>!", true);
            leftLabel->setFont("Avenir Next Bold", 24);
            leftLabel->setTextAlign(shittygui::TextAlign::Center);
            leftLabel->setTextColor({1, 1, 1});
            leftLabel->setDebugLabel("'Hello world' label");
            left->addChild(leftLabel);

            auto longLabel = shittygui::MakeWidget<shittygui::widgets::Label>({3, 45}, {354, 240});
            longLabel->setContent(R"(I'm baby retro single-origin coffee stumptown small batch echo park, chicharrones tote bag vexillologist literally. Mlkshk intelligentsia shabby chic sustainable. Shabby chic copper mug helvetica DIY art party you probably haven't heard of them, humblebrag cloud bread adaptogen blog. Dreamcatcher wayfarers raw denim XOXO lyft disrupt jianbing tattooed 90's chia. Gluten-free post-ironic bushwick single-origin coffee brooklyn yes plz. Umami humblebrag shabby chic, selvage pok pok franzen church-key.

        Lomo photo booth single-origin coffee health goth raclette YOLO franzen unicorn vexillologist migas woke wolf irony. Retro ugh palo santo cray aesthetic fashion axe, pabst hashtag poutine. Meggings tbh schlitz, mixtape celiac viral la croix hammock offal squid brooklyn yr fam. Vice chambray kogi fashion axe selfies schlitz trust fund yes plz. Keytar lo-fi affogato pop-up slow-carb schlitz drinking vinegar cray pinterest. Fashion axe vice messenger bag scenester cold-pressed XOXO schlitz YOLO kombucha you probably haven't heard of them. Direct trade small batch pickled, enamel pin yes plz lumbersexual chartreuse forage iceland messenger bag prism.)");
            longLabel->setFont("Liberation Sans", 11);
            longLabel->setTextAlign(shittygui::TextAlign::Left);
            longLabel->setWordWrap(true);
            longLabel->setEllipsizeMode(shittygui::EllipsizeMode::Middle);
            longLabel->setTextColor({0.9, 1, 1});
            longLabel->setDebugLabel("Long text label");

            left->addChild(longLabel);

            cont->addChild(left);

            // right container
            auto right = shittygui::MakeWidget<shittygui::widgets::Container>({420, 20}, {360, 430});
            right->setBackgroundColor({0, 0, 0.33});
            right->setDebugLabel("Right container");

            auto indetBar = shittygui::MakeWidget<shittygui::widgets::ProgressBar>({5, 400}, {350, 22},
                    shittygui::widgets::ProgressBar::Style::Indeterminate);
            indetBar->setDebugLabel("Indeterminate progress bar");
            right->addChild(indetBar);

            auto normalBar = shittygui::MakeWidget<shittygui::widgets::ProgressBar>({5, 368}, {350, 22},
                    shittygui::widgets::ProgressBar::Style::Determinate);
            normalBar->setProgress(.5);
            normalBar->setDebugLabel("Determinate progress bar");
            right->addChild(normalBar);

            // buttons
            auto butt = shittygui::MakeWidget<shittygui::widgets::Button>({5, 300}, {150, 38},
                    shittygui::widgets::Button::Type::Push);
            butt->setDebugLabel("'Push me' button");
            butt->setTitle("Push me");
            butt->setIconGravity(shittygui::widgets::Button::IconGravity::Left);
            butt->setPushCallback([this](auto whomst) {
                auto btn = std::dynamic_pointer_cast<shittygui::widgets::Button>(whomst);
                btn->setTitle("fuk off");

                std::cout << "presentment time" << std::endl;
                auto vc = std::make_shared<SecondTestViewController>();
                this->presentViewController(vc, true);
            });

            right->addChild(butt);

            auto blah = shittygui::MakeWidget<shittygui::widgets::Button>({300, 300}, {150, 38},
                    shittygui::widgets::Button::Type::Push);
            blah->setDebugLabel("'Push me' button");
            blah->setTitle("Long label button");
            right->addChild(blah);

            auto plantImg = shittygui::Image::Read("./plant.png");
            butt->setIcon(plantImg);

            auto butt2 = shittygui::MakeWidget<shittygui::widgets::Button>({5, 270}, {24, 24},
                    shittygui::widgets::Button::Type::Push);
            butt2->setIcon(plantImg);
            butt2->setDebugLabel("Plant button");

            right->addChild(butt2);

            // image views
            auto pyramid = shittygui::Image::Read("./egyptian_pyramid.png");
            auto spectrum = shittygui::Image::Read("./spectrum.png");
            auto tree = shittygui::Image::Read("./tree.png");

            right->addChild(shittygui::MakeWidget<shittygui::widgets::ImageView>({5, 200}, {48, 48},
                    pyramid, shittygui::widgets::ImageView::Mode::None));
            right->addChild(shittygui::MakeWidget<shittygui::widgets::ImageView>({58, 200}, {16, 16},
                    pyramid, shittygui::widgets::ImageView::Mode::None));
            right->addChild(shittygui::MakeWidget<shittygui::widgets::ImageView>({58, 224}, {16, 16},
                    pyramid, shittygui::widgets::ImageView::Mode::ScaleProportionalDown));
            right->addChild(shittygui::MakeWidget<shittygui::widgets::ImageView>({79, 200}, {48, 64},
                    pyramid, shittygui::widgets::ImageView::Mode::ScaleIndependently));
            right->addChild(shittygui::MakeWidget<shittygui::widgets::ImageView>({132, 200}, {48, 64},
                    pyramid, shittygui::widgets::ImageView::Mode::ScaleProportionalUpDown));

            auto borderlessImg = shittygui::MakeWidget<shittygui::widgets::ImageView>({185, 200}, {32, 32},
                    spectrum, shittygui::widgets::ImageView::Mode::None);
            borderlessImg->setBorderWidth(0);
            borderlessImg->setBackgroundColor({0, 0, 0, 0});
            right->addChild(borderlessImg);

            auto borderlessImg2 = shittygui::MakeWidget<shittygui::widgets::ImageView>({185, 232}, {32, 32},
                    tree, shittygui::widgets::ImageView::Mode::None);
            borderlessImg2->setBorderWidth(0);
            borderlessImg2->setBackgroundColor({0, 0, 0, 0});
            right->addChild(borderlessImg2);

            cont->addChild(right);

            // store it as the root
            this->view = std::move(cont);
        }

        /// Get our root view
        std::shared_ptr<shittygui::Widget> &getWidget() override {
            return this->view;
        }

        /// Return a dummy title
        std::string_view getTitle() override {
            return "Test View Controller";
        }

        void viewDidAppear() override {
            ViewController::viewDidAppear();
            printf("View controller %p did appear\n", this);
        }
        void viewDidDisappear() override {
            ViewController::viewDidDisappear();
            printf("View controller %p did disappear\n", this);
        }

    private:
        std::shared_ptr<shittygui::Widget> view;
};

/**
 * @brief Set up the demo screen
 */
static void InitScreen(const std::shared_ptr<shittygui::Screen> &screen) {
    auto vc = std::make_shared<TestViewController>();
    screen->setRootViewController(vc);
}

/**
 * @brief Insert a touch event into the screen's event queue
 *
 * This takes a mouse button up/down event to insert a touch up/down event if it's the left button,
 * and for the center mouse button generate a button event.
 */
static void InsertTouchEvent(const std::shared_ptr<shittygui::Screen> &screen,
        const SDL_MouseButtonEvent &event) {
    // select (middle) button
    if(event.button == SDL_BUTTON_MIDDLE) {
        screen->queueEvent(shittygui::event::Button(shittygui::event::Button::Select,
                    (event.state == SDL_PRESSED)));
    }
    // touch event
    else if(event.button == SDL_BUTTON_LEFT) {
        screen->queueEvent(shittygui::event::Touch(shittygui::Point(event.x, event.y),
                    (event.state == SDL_PRESSED)));
    }
}

/**
 * @brief Insert a movement event into the screen's event queue
 *
 * Convert a mouse motion event into a touch event. The evnet is discarded if the mouse isn't down.
 */
static void InsertTouchEvent(const std::shared_ptr<shittygui::Screen> &screen,
        const SDL_MouseMotionEvent &event) {
    // ignore event if no button down or if it's not the left button
    if(!event.state || event.state != SDL_BUTTON_LMASK) {
        return;
    }

    screen->queueEvent(shittygui::event::Touch(shittygui::Point(event.x, event.y),
                (event.state & SDL_BUTTON_LMASK)));
}

/**
 * @brief Insert a scroll event (based on a mouse wheel event)
 *
 * Convert mouse wheel events into scroll events.
 */
static void InsertScrollEvent(const std::shared_ptr<shittygui::Screen> &screen,
        const SDL_MouseWheelEvent &event) {
    screen->queueEvent(shittygui::event::Scroll(std::ceil(event.preciseY)));
}

/**
 * Application entry point
 */
int main(const int argc, const char **argv) {
    int err;
    std::shared_ptr<shittygui::Screen> screen;

    // get a SDL window and renderer set up
    SDL_Init(SDL_INIT_VIDEO);

    auto window = SDL_CreateWindow("ShittyGUI Test", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, kWindowSize.width, kWindowSize.height,
            SDL_WINDOW_ALLOW_HIGHDPI);
    if(!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // figure out if our UI is scaled
    int renderW, renderH, windowW, windowH;
    err = SDL_GetRendererOutputSize(renderer, &renderW, &renderH);
    if(err) {
        std::cerr << "SDL_GetRendererOutputSize failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GetWindowSize(window, &windowW, &windowH);

    printf("Window size: %dx%d, drawable size %dx%d\n", windowW, windowH, renderW, renderH);

    // set up GUI library
    screen = std::make_shared<shittygui::Screen>(shittygui::Screen::PixelFormat::RGB24,
            shittygui::Size(renderW, renderH));
    printf("framebuffer: %p (stride %lu bytes)\n", screen->getBuffer(), screen->getBufferStride());

    screen->setBackgroundColor({0, 0.33, 0});

    if(renderW != windowW) {
        const double uiScale = static_cast<double>(renderW) / static_cast<double>(windowW);
        printf("UI scale: %g\n", uiScale);

        screen->setScaleFactor(uiScale);
    }

    InitScreen(screen);

    // set up texture to render into
    const auto &physSize = screen->getFramebufferSize();
    auto inTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING,
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
     *
     * We simulate a rotary encoder with the mouse wheel; rotate it vertically to scroll, and click
     * it to simulate the encoder "select" button.
     */
    while(gRun) {
        // process events
        SDL_Event e;

        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                // mouse motion
                case SDL_MOUSEMOTION:
                    InsertTouchEvent(screen, e.motion);
                    break;
                // mouse button events
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    InsertTouchEvent(screen, e.button);
                    break;
                // mouse wheel = encoder events
                case SDL_MOUSEWHEEL:
                    InsertScrollEvent(screen, e.wheel);
                    break;

                // TODO: key events

                // terminate the application
                case SDL_QUIT:
                    gRun = false;
                    break;
            }
        }

        // update the state of the screen
        screen->processEvents();
        screen->handleAnimations();

        // redraw the screen, if it indicates that it's dirty. this is slow and shitty (lol)
        if(screen->isDirty()) {
            screen->redraw();

            SDL_UpdateTexture(inTex, nullptr, screen->getBuffer(), screen->getBufferStride());
        }

        // update display
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, inTex, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // clean up
    screen.reset();

    SDL_DestroyTexture(inTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
