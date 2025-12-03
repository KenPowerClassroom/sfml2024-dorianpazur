#pragma once

#include <SFML/Graphics.hpp>
using namespace sf;
#include "outrun.h"

#ifndef TESTS
extern int lineCount;
#endif
const float Y_POS_MIN = 1500.0f;
const float X_POS_RANGE = 1.0f;
const float MAX_SPEED = 500.0f;
const float MIN_SPEED = 10.0f;
const float ACCEL = 4.0f;
const float DRAG = 1.0f / 60000.0f;
const float BRAKE_DRAG = 50.0f / 60000.0f;

class Camera
{
    float x = 0;
    float y = Y_POS_MIN;
    float z = 0;
    float spd = 0;

    float accel = ACCEL;
    float drag = DRAG;

public:

    float xPos() const
    {
        return x;
    }

    float yPos() const
    {
        return y;
    }

    float zPos() const
    {
        return z;
    }

    float speed() const
    {
        return spd;
    }

    void update() {
#ifndef TESTS // we handle inputs manually in tests
        resetModifiers();

        if (Keyboard::isKeyPressed(Keyboard::Right)) strafeRight();
        if (Keyboard::isKeyPressed(Keyboard::Left)) strafeLeft();
        if (Keyboard::isKeyPressed(Keyboard::Tab)) boost();
        if (Keyboard::isKeyPressed(Keyboard::Up)) holdAccelerator();
        if (Keyboard::isKeyPressed(Keyboard::Down)) holdBrakes();
        if (Keyboard::isKeyPressed(Keyboard::W)) rise();
        if (Keyboard::isKeyPressed(Keyboard::S)) fall();
#endif

        spd /= 1 + (std::fabsf(spd) * drag); // apply drag
        if (std::fabsf(spd) < MIN_SPEED && drag != DRAG) // is braking and going too slow
            spd = 0.0f; // stop

        // hard limit speed
        spd = std::fmaxf(-MAX_SPEED, std::fminf(spd, MAX_SPEED));

        z += spd;
        //while (zPos >= lineCount*SEGMENT_LENGTH) zPos-=lineCount*SEGMENT_LENGTH;
        //while (zPos < 0) zPos += lineCount*SEGMENT_LENGTH;
        wrapPosition();

        clampPositions();
    }

    void clampPositions()
    {
        // clamp values to prevent crashes
        y = std::fmax(Y_POS_MIN, y);
        x = std::fmax(-X_POS_RANGE, std::fmin(X_POS_RANGE, x));
    }

    void holdAccelerator()
    {
        if (spd >= 0.0f)
        {
            spd += ACCEL * (1.0f - std::fmin(1.0f, std::fabsf(spd / MAX_SPEED))); // drive forward
        }
        else // apply brakes if reversing
        {
            drag += BRAKE_DRAG / std::fabsf(spd) + 0.001f; // simulate wheel slip
        }
    }

    void holdBrakes()
    {
        if (spd <= 0.0f)
        {
            spd -= ACCEL * (1.0f - std::fmin(1.0f, std::fabsf(spd / (MAX_SPEED * 0.25f)))); // reverse
        }
        else // apply brakes if driving forward
        {
            drag += BRAKE_DRAG / std::fabsf(spd) + 0.001f; // simulate wheel slip
        }
    }

    void rise()
    {
        y += 100.0f;
    }

    void fall()
    {
        y -= 100.0f;
    }

    void boost()
    {
        accel *= 3.0f;
    }

    void strafeLeft()
    {
        x -= 0.1f;
    }

    void strafeRight()
    {
        x += 0.1f;
    }

    void wrapPosition()
    {
#ifndef TESTS
        z = z - (lineCount * SEGMENT_LENGTH) * floor(z / (lineCount * SEGMENT_LENGTH));
#endif
    }

    void resetModifiers()
    {
        accel = ACCEL;
        drag = DRAG;
    }
};
