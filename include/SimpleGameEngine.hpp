#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <functional>

class LTexture {
private:
    SDL_Texture *mTexture = nullptr;
    int mWidth;
    int mHeight;
public:
    LTexture();

    ~LTexture();

    bool loadTextureFromText(const std::string& text, SDL_Color color);

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
    SDL_Window *gWindow = nullptr;
    LTexture texture;
public:
    GameEngine();
    
    virtual bool onFrameUpdate(float fElapsedTime) = 0;

    virtual bool onInit() = 0;

    virtual void onKeyboardEvent(int keycode, float secPerFrame);

    virtual void onMouseEvent(int posX, int posY, float secPerFrame);

    virtual bool drawPoint(int x, int y, Color color = {0xFF, 0xFF, 0xFF});

    bool drawLine(int x1, int y1, int x2, int y2, Color color = {0xFF, 0xFF, 0xFF});

    bool drawString(int x, int y, std::string text);

    bool constructConsole(int nCharsX, int nCharsY, const char * title);

    bool createResources();

    bool renderConsole();

    void startGameLoop();

    void close_sdl();

    ~GameEngine();
};
