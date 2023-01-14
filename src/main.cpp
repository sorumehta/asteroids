#include <iostream>
#include "SimpleGameEngine.hpp"
#include <vector>
#include <cmath>
#include <utility>

int CURRENT_ID = 0;

class Asteroids : public GameEngine{
private:
    int score;
    const float mAcceleration;
    const float bulletSpeed;
    bool dead;
    struct SpaceObject{
        int id;
        float x; // x pos
        float y; // y pos
        float velX; // x velocity
        float velY; // y velocity
        int size;
        float angle;
        int health;
        Color colour;
        float mass = size*2;
    };
    std::vector<SpaceObject> vecAsteroids;
    std::vector<SpaceObject> vecBullets;
    SpaceObject player{};
    // model objects which contain initial coordinates to draw the corresponding objects on screen.
    std::vector<std::pair<float, float>> vecModelShip;
    std::vector<std::pair<float, float>> vecModelAsteroid;

public:
    Asteroids(): score(0), mAcceleration(100.0f), bulletSpeed(180.0f), dead(false){}

    bool onInit() override{
        int iSize = 32;

        vecAsteroids.push_back({CURRENT_ID++, 20.0f, 20.0f, 28.0, -30.0f, iSize, 0.0f, iSize * 10, {0xDA, 0xC2, 0x2B}}); //#DAC22B
        vecAsteroids.push_back({CURRENT_ID++,420.0f, 120.0f, -25.0, 16.0f, iSize, 0.0f, iSize * 10, {0x2B, 0xD2, 0xDA}}); //#2BD2DA
        vecAsteroids.push_back({CURRENT_ID++,120.0f, 0.0f, -25.0, 36.0f, iSize, 0.0f, iSize * 10, {0x9A, 0xDA, 0x2B}}); //#9ADA2B
        vecAsteroids.push_back({CURRENT_ID++,0.0f, 200.0f, 25.0, -16.0f, iSize, 0.0f, iSize * 10, {0xDA, 0x48, 0x2B}}); //#DA482B
        vecAsteroids.push_back({CURRENT_ID++,300.0f, 50.0f, -30.0, -20.0f, iSize, 0.0f, iSize * 10, {0xB4, 0x7A, 0xE1}}); //#B47AE1
        vecAsteroids.push_back({CURRENT_ID++,500.0f, 300.0f, 35.0, 35.0f, iSize, 0.0f, iSize * 10, {0x95, 0x73, 0x72}}); //#957372
        player.x = mWindowWidth / 2.0f;
        player.y = mWindowHeight / 2.0f;
        player.velX = 4.0f;
        player.velY = -3.0f;
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
            float seg_angle = 6.28318f * (float(i)) / (float(verts)); // portion of 2*PI
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
        return GameEngine::drawPoint(static_cast<int>(std::round(fx)), static_cast<int>(std::round(fy)), color);
    }

    void onKeyboardEvent(int keycode, float secPerFrame) override {
        if(dead){
            return;
        }
        switch (keycode) {
            case SDLK_RIGHT:
                player.angle += (10.0f * secPerFrame);
                break;
            case SDLK_LEFT:
                player.angle -= (10.0f * secPerFrame);
                break;
            case SDLK_UP: // a = v2 - v1 / t   =>   v2 = a*t + v1
                player.velX += (std::sin(player.angle) * mAcceleration * secPerFrame);
                player.velY += (-std::cos(player.angle) * mAcceleration * secPerFrame);
                break;
            case SDLK_SPACE:
                SpaceObject bullet = {CURRENT_ID++,player.x, player.y, bulletSpeed * std::sin(player.angle), -bulletSpeed * std::cos(player.angle), 0, 0};
                vecBullets.push_back(bullet);
        }
    }


    bool onFrameUpdate(float secPerFrame) override{
        // secPerFrame is the time it took for the previous frame in seconds. Why is it useful?
        // Suppose we update the object position by the same amount in each frame, we end up with
        // non-uniform speed due to inconsistent time taken to compute each frame.
        // If we want to move an object by 5 m/s, then in each frame, we move it by 5 / FPS.
        // where FPS = 1 / secPerFrame. So essentially, we move it by 5 * secPerFrame.

        // utility function
        auto doCirclesOverlap = [](float x1, float y1, float r1, float x2, float y2, float r2 ) -> bool {
            return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) <= (r1+r2)*(r1+r2);
        };

        // x2 = x1 + v*t
        player.x += player.velX * secPerFrame;
        player.y += player.velY * secPerFrame;

        WrapCoordinates(player.x, player.y, player.x, player.y);

        std::vector<std::pair<SpaceObject *, SpaceObject *>> vecCollidingAsteroids;

        // check ship collision with asteroids
        for(auto &a: vecAsteroids){
            if(isPointInsideCircle(a.x, a.y, a.size, player.x, player.y)){
                dead = true;
            }
        }

        for(auto &ast1: vecAsteroids){
            for(auto &ast2: vecAsteroids) {
                if(ast1.id != ast2.id) {
                    if (doCirclesOverlap(ast1.x, ast1.y, ast1.size, ast2.x, ast2.y, ast2.size)) {
                        vecCollidingAsteroids.emplace_back(&ast1, &ast2);
                        
                        // resolving static collision
                        float fDistance = std::sqrt(
                                (ast1.x - ast2.x) * (ast1.x - ast2.x) + (ast1.y - ast2.y) * (ast1.y - ast2.y));
                        float fOverlap = 0.5f * (fDistance - ast1.size - ast2.size);
                        //Displace first asteroid
                        //multiply the overlap by basis vector
                        ast1.x -= fOverlap * (ast1.x - ast2.x) / fDistance;
                        ast1.y -= fOverlap * (ast1.y - ast2.y) / fDistance;

                        //Displace second asteroid
                        ast2.x += fOverlap * (ast1.x - ast2.x) / fDistance;
                        ast2.y += fOverlap * (ast1.y - ast2.y) / fDistance;
                    }
                }
            }
        }

        // resolving dynamic collisions
        for(auto c : vecCollidingAsteroids){
            SpaceObject *b1 = c.first;
            SpaceObject *b2 = c.second;

            // calculate the unit vector in direction passing through centres of balls (the normal)
            float fDistance = std::sqrt((b1->x - b2->x)*(b1->x - b2->x) + (b1->y - b2->y)*(b1->y - b2->y));
            float nx = (b2->x - b1->x) / fDistance;
            float ny = (b2->y - b1->y) / fDistance;

            // calculate the tangent to the normal (transforming the vector using 90 degrees rotation)
            float tx = -ny;
            float ty = nx;
            // basically, tx and ty are where the basis vectors (i and j) land after transforming to the tangent line

            // now take dot product i.e. transform the velocity vector of ball on the tangent line (scalar)
            float fDotTang1 = b1->velX * tx + b1->velY * ty;
            float fDotTang2 = b2->velX * tx + b2->velY * ty;


            // now take dot product i.e. transform the velocity vector of ball on the normal line (scalar)
            float dpNorm1 = b1->velX * nx + b1->velY * ny;
            float dpNorm2 = b2->velX * nx + b2->velY * ny;


            // momentum must be conserved along the normal direction, so we use 1D momentum conservation eq to get final velocity
            // using dpNorm values as initial velocity quantity (scalar) in the normal direction
            float v1_scalar = (dpNorm1 * (b1->mass - b2->mass) + 2.0f * b2->mass * dpNorm2) / (b1->mass + b2->mass);
            float v2_scalar = (dpNorm2 * (b2->mass - b1->mass) + 2.0f * b1->mass * dpNorm1) / (b1->mass + b2->mass);

            // convert the scalar projections to vector by multiplying it with each basis vector,
            // the result would be the new velocity in the tangent direction
            b1->velX = fDotTang1 * tx + v1_scalar * nx;
            b1->velY = fDotTang1 * ty + v1_scalar * ny;
            b2->velX = fDotTang2 * tx + v2_scalar * nx;
            b2->velY = fDotTang2 * ty + v2_scalar * ny;

        }

        //  update and draw asteroids
        for(auto &a : vecAsteroids){
            a.x += (a.velX * secPerFrame);
            a.y += (a.velY * secPerFrame);
            WrapCoordinates(a.x, a.y, a.x, a.y);
            DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, a.size, true, a.colour);
        }

        std::vector<SpaceObject> newAsteroids;

        // draw bullets
        for(auto &b : vecBullets){
            b.x += (b.velX * secPerFrame);
            b.y += (b.velY * secPerFrame);
            b.angle -= 1.0f * secPerFrame;
//            WrapCoordinates(b.x, b.y, b.x, b.y);
            drawPoint(b.x, b.y);

            // check collision with asteroids

            for (auto &a: vecAsteroids){
                if(isPointInsideCircle(a.x, a.y, a.size, b.x, b.y)){
                    // collision with asteroid
                    b.x = -100;
                    a.health -= 100;
                    if(a.health <= 0){
                        score += 20;
                        if(a.size > 16){
                            float rand_angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/6.28318f));
                            newAsteroids.push_back({CURRENT_ID++,a.x+10, a.y+10, a.velX * std::sin(rand_angle), a.velY * std::cos(rand_angle), a.size/2, 0, (a.size/2)*10, a.colour});
                            rand_angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/6.28318f));
                            newAsteroids.push_back({CURRENT_ID++,a.x-10, a.y-10, a.velX * std::sin(rand_angle), a.velY * std::cos(rand_angle), a.size/2, 0, (a.size/2)*10, a.colour});
                        }
                        a.x = -100;
                    }
                }
            }
        }

        for(auto a: newAsteroids){
            vecAsteroids.push_back(a);
        }

        // remove bullets which are off the screen
        if(!vecBullets.empty()){
            auto it = std::remove_if(vecBullets.begin(), vecBullets.end(),
                                     [&](SpaceObject o){
                return (o.x <1 || o.y < 1 || o.x >= mWindowWidth || o.y >= mWindowHeight);});
            if(it != vecBullets.end()){
                vecBullets.erase(it);
            }
        }

        // remove asteroids which are off the screen
        if(!vecAsteroids.empty()){
            auto it = std::remove_if(vecAsteroids.begin(), vecAsteroids.end(),
                                     [&](SpaceObject o){ return (o.x < 0 ); });
            if(it != vecAsteroids.end()){
                vecAsteroids.erase(it);
            }
        }
        // draw ship
        DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);
        drawString(2, 2, "Score: " + std::to_string(score));
        if(dead){
            drawString(mWindowWidth/2, mWindowHeight/2, "Game Over Kiddo!");
        }
        return true;
    }

    bool isPointInsideCircle(float cx, float cy, float radius, float x, float y)
    {
        return (x-cx)*(x-cx) + (y-cy)*(y-cy) < radius * radius;
    }

    void DrawWireFrameModel(const std::vector<std::pair<float, float>> &vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, bool fillCircle = false, Color color = {0xFF, 0xFF, 0xFF})
    {
        // Create translated model vector of coordinate pairs, we don't want to change the original one
        std::vector<std::pair<float, float>> vecTransformedCoordinates;
        unsigned int verts = vecModelCoordinates.size();
        vecTransformedCoordinates.resize(verts);

        // Rotate

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
            drawLine(static_cast<int>(std::round(vecTransformedCoordinates[i % verts].first)), static_cast<int>(std::round(vecTransformedCoordinates[i % verts].second)),
                     static_cast<int>(std::round(vecTransformedCoordinates[j % verts].first)), static_cast<int>(std::round(vecTransformedCoordinates[j % verts].second)), color);
        }

        if(fillCircle){
            fillCircleWithColor({static_cast<int>(x), static_cast<int>(y)}, s,{color.r, color.g, color.b});
        }
    }
    void fillCircleWithColor(SDL_Point center, int radius, SDL_Color color)
    {
        for (int w = 0; w < radius * 2; w++)
        {
            for (int h = 0; h < radius * 2; h++)
            {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                if ((dx*dx + dy*dy) <= (radius * radius))
                {
                    drawPoint( center.x + dx, center.y + dy, {color.r, color.b, color.g});
                }
            }
        }
    }


};

int main(int argc, char *args[]) {
    Asteroids asteroids;
    asteroids.constructConsole(800, 450, "Asteroids");
    asteroids.startGameLoop();

    return 0;
}
