#include "SimpleGameEngine.hpp"


LTexture::LTexture() {
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

int LTexture::getHeight() const { return mHeight; }

int LTexture::getWidth() const { return mWidth; }

void LTexture::render(SDL_Renderer *renderer, int x, int y) {
    SDL_Rect rect = {x, y, mWidth, mHeight};
    SDL_RenderCopy(renderer, mTexture, NULL, &rect);
}

void LTexture::free() {
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
        mWidth = 0;
        mHeight = 0;
    }
}

LTexture::~LTexture() {
    //Deallocate
    free();
}


GameEngine::GameEngine(): mWindowWidth(80), mWindowHeight(40), gWindow(nullptr), gRenderer(nullptr) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL initialization failed: " << SDL_GetError();
    }
}

GameEngine::~GameEngine() {
    close_sdl();
}

bool GameEngine::constructConsole(int windowWidth = 80, int windowHeight = 40, const char * title = "Window") {
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    int maxWidth = DM.w;
    int maxHeight = DM.h;
    if(windowWidth > maxWidth || windowHeight > maxHeight){
        std::cout << "Window size too large! ";
        std::cout << "maxWidth = " << maxWidth << ", maxHeight = " << maxHeight << std::endl;
        return false;
    }
    gWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               windowWidth, windowHeight, SDL_WINDOW_SHOWN); // 5 margin
    if (gWindow == nullptr) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError();
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == nullptr) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError();
        return false;
    }
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;
    return true;

}

bool GameEngine::createResources() {

    return true;
}

bool GameEngine::renderConsole() {


    //update screen
    SDL_RenderPresent(gRenderer);
    return true;
}

bool GameEngine::drawLine(int x1, int y1, int x2, int y2, Color color ) {
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
    if(SDL_RenderDrawLine( gRenderer, x1, y1, x2, y2 ) != 0){
        return false;
    }
    return true;
}

bool GameEngine::drawPoint(int x, int y, Color color) {
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    if(SDL_RenderDrawPoint( gRenderer, x, y ) != 0){
        return false;
    }
    return true;
}

void GameEngine::close_sdl() {

    //Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = nullptr;

    //Quit SDL subsystems
    SDL_Quit();

}

void GameEngine::initScreen() {
    //clear screen
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gRenderer);
}

void GameEngine::startGameLoop() {
    bool quit = false;
    if (!createResources()) {
        std::cout << "error while loading resources" << std::endl;
        close_sdl();
        quit = true;
    }
    initScreen();
    if (!onInit()){
        std::cout << "onInit function returned error" << std::endl;
        quit = true;
    }
    auto prevFrameTime = std::chrono::system_clock::now();
    auto currFrameTime = std::chrono::system_clock::now();

    while(!quit){
        // handle timing
        currFrameTime = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = currFrameTime - prevFrameTime;
        prevFrameTime = currFrameTime;
        float frameElapsedTime = elapsedTime.count();
        initScreen();
        //handle input
        SDL_Event e;
        SDL_Event *userInput = nullptr;
        if (SDL_PollEvent(&e) != 0) {
            //User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            } else {
                userInput = &e;
            }

        }
        if(!onFrameUpdate(frameElapsedTime, userInput)){
            quit = true;
        }

        // 4. RENDER OUTPUT

        if (!renderConsole()) {
            std::cout << "error while loading texture from text" << std::endl;
            quit = true;
        }

    }
}
