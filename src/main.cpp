#include <iostream>
#include "SimpleGameEngine.hpp"
#include <vector>
#include <cmath>
#include <utility>

class Asteroids : public GameEngine{
private:
    const float mAcceleration;
    struct SpaceObject{
        float x; // x pos
        float y; // y pos
        float velX; // x velocity
        float velY; // y velocity
        int size;
        float angle;
    };
    std::vector<SpaceObject> vecAsteroids;
    std::vector<SpaceObject> vecBullets;
    SpaceObject player{};
    // model objects which contain initial coordinates to draw the corresponding objects on screen.
    std::vector<std::pair<float, float>> vecModelShip;
    std::vector<std::pair<float, float>> vecModelAsteroid;

public:
    Asteroids(): mAcceleration(30.0f){}

    bool onInit() override{
        vecAsteroids.push_back({20.0f, 20.0f, 8.0, -6.0f, (int)8, 0.0f});
        player.x = mWindowWidth / 2.0f;
        player.y = mWindowHeight / 2.0f;
        player.velX = 0.0f;
        player.velY = 0.0f;
        player.angle = 0.0f;
        // ship initial coordinates in game space (where 0,0 is center of the screen)
        // we don't change them, they act as model coordinates
        vecModelShip = {
                {0.0f, -11.0f},
                {-5.0f, 5.0f},
                {5.0f, 5.0f}
        };
        int verts = 20; // asteroid is a 20 vertices polygon
        for(int i = 0; i < verts; i++) {
            float radius = 1.0f;
            float seg_angle = 6.28318f * (float(i)) / (float(verts));
            vecModelAsteroid.emplace_back(radius * std::sin(seg_angle), radius*std::cos(seg_angle));
        }

        return true;
    }
    void WrapCoordinates(float ix, float iy, float &ox, float &oy)
    {
        ox = ix;
        oy = iy;
        if (ix < 0.0f)	ox = ix + (float)mWindowWidth;
        if (ix >= (float)mWindowWidth)	ox = ix - (float)mWindowWidth;
        if (iy < 0.0f)	oy = iy + (float)mWindowHeight;
        if (iy >= (float)mWindowHeight) oy = iy - (float)mWindowHeight;
    }
    bool drawPoint(int x, int y, Color color = {0xFF, 0xFF, 0xFF}) override{
        float fx, fy;
        WrapCoordinates(x, y, fx, fy);
        return GameEngine::drawPoint(static_cast<int>(std::round(fx)), static_cast<int>(std::round(fy)));
    }

    void onKeyboardEvent(int keycode, float secPerFrame) override {
        switch (keycode) {
            case SDLK_RIGHT:
                player.angle -= (5.0f * secPerFrame);
                break;
            case SDLK_LEFT:
                player.angle += (5.0f * secPerFrame);
                break;
            case SDLK_UP: // a = v2 - v1 / t   =>   v2 = a*t + v1
                player.velX += (std::sin(player.angle) * mAcceleration * secPerFrame);
                player.velY += (-std::cos(player.angle) * 20.f * secPerFrame);
                break;
            case SDLK_SPACE:
                SpaceObject bullet = {player.x, player.y, 50.0f * std::sin(player.angle), -50.0f * std::cos(player.angle), 0, 0};
                vecBullets.push_back(bullet);
        }
    }

    bool onFrameUpdate(float secPerFrame) override{
        // secPerFrame is the time it took for the previous frame in seconds. Why is it useful?
        // Suppose we update the object position by the same amount in each frame, we end up with
        // non-uniform speed due to inconsistent time taken to compute each frame.
        // If we want to move an object by 5 m/s, then in each frame, we move it by 5 / FPS.
        // where FPS = 1 / secPerFrame. So essentially, we move it by 5 * secPerFrame.

        // x2 = x1 + v*t
        player.x += player.velX * secPerFrame;
        player.y += player.velY * secPerFrame;

        WrapCoordinates(player.x, player.y, player.x, player.y);

//         update and draw asteroids
        for(auto &a : vecAsteroids){
            a.x += (a.velX * secPerFrame);
            a.y += (a.velY * secPerFrame);
            WrapCoordinates(a.x, a.y, a.x, a.y);
            DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, a.size);
        }

        // draw bullets
        for(auto &b : vecBullets){
            b.x += (b.velX * secPerFrame);
            b.y += (b.velY * secPerFrame);
            WrapCoordinates(b.x, b.y, b.x, b.y);
            drawPoint(b.x, b.y);
        }

        // remove bullets which are off the screen
        if(vecBullets.size() > 0){
            auto it = std::remove_if(vecBullets.begin(), vecBullets.end(),
                                     [&](SpaceObject o){
                return (o.x <1 || o.y < 1 || o.x >= mWindowWidth || o.y >= mWindowHeight);});
            if(it != vecBullets.end()){
                vecBullets.erase(it);
            }
        }

        // draw ship
        DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

        return true;
    }

    // Draws a model on screen with the given rotation, translation and scaling
    void DrawWireFrameModel(const std::vector<std::pair<float, float>> &vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, Color color = {0xFF, 0xFF, 0xFF})
    {
        // std::pair.first = x coordinate
        // std::pair.second = y coordinate

        // Create translated model vector of coordinate pairs, we don't want to change the original one
        std::vector<std::pair<float, float>> vecTransformedCoordinates;
        int verts = vecModelCoordinates.size();
        vecTransformedCoordinates.resize(verts);

        // Rotate
        // To rotate the ship by angle A to left, the equations are:
        //    P2_x = |P2|*cos(A1 + A2) where |P1| and |P2| are equal, A1 is original angle, A2 is rotated angle
        // => P2_x = P1_x * cos(A2) - P1_y * sin(A2)
        //    Similarly,
        //    P2_y = P1_x * sin(A2) + P1_y * cos(A2)
        // Since these equations are just manipulating x and y to get new x and y,
        // we can also represent these equations using a matrix multiplication
        // [P2_x] = [cos(A)  -sin(A)] [P1_x]
        // [P2_y] = [sin(A)   cos(A)] [P1_y]
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * std::cos(r) - vecModelCoordinates[i].second * std::sin(r);
            vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * std::sin(r) + vecModelCoordinates[i].second * std::cos(r);
        }

        // Scale
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
        }

        // Translate
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
        }

        // Draw Closed Polygon
        for (int i = 0; i < verts + 1; i++)
        {
            int j = (i + 1);
            drawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
                     vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, color);
        }
    }

};

int main(int argc, char *args[]) {
    Asteroids asteroids;
    asteroids.constructConsole(800, 450, "Asteroids");
    asteroids.startGameLoop();

    return 0;
}
