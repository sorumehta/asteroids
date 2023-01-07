#pragma once

#include <SDL.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <functional>

class LTexture {
private:
    SDL_Texture *mTexture;
    int mWidth;
    int mHeight;

public:
    LTexture();

    ~LTexture();

    void render(SDL_Renderer *renderer, int x, int y);

    int getWidth() const;

    int getHeight() const;

    void free();
};

typedef struct ConsoleInfo{
    int windowWidth;
    int windowHeight;
    int nCharsX;
    int nCharsY;
    char *screenBuffer;
} ConsoleInfo;

struct Color {
    int r;
    int g;
    int b;
};

class GameEngine {
protected:
    int mWindowWidth;
    int mWindowHeight;
    SDL_Event e;
private:
    void initScreen();
    SDL_Window *gWindow;
    SDL_Renderer *gRenderer;
public:
    GameEngine();
    
    virtual bool onFrameUpdate(float fElapsedTime) = 0;
    virtual bool onInit() = 0;
    virtual void onKeyboardEvent(int keycode, float secPerFrame);
    virtual void onMouseEvent(int posX, int posY, float secPerFrame);

    bool constructConsole(int nCharsX, int nCharsY, const char * title);

    bool createResources();

    bool renderConsole();

    bool drawLine(int x1, int y1, int x2, int y2, Color color = {0xFF, 0xFF, 0xFF});
    virtual bool drawPoint(int x, int y, Color color = {0xFF, 0xFF, 0xFF});

    void startGameLoop();

    void close_sdl();

    ~GameEngine();
};
