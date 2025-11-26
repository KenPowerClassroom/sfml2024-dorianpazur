#include <SFML/Graphics.hpp>
using namespace sf;

const uint32_t FPS = 60;
const uint32_t WINDOW_WIDTH = 1024;
const uint32_t WINDOW_HEIGHT = 768;
const int ROAD_WIDTH = 2000;
const int SEGMENT_LENGTH = 200; //segment length
const float CAMERA_DEPTH = 0.84f; //camera depth
const float Y_POS_MIN = 1500.0f;
const float X_POS_RANGE = 1.0f;
const float MAX_SPEED = 500.0f;
const float MIN_SPEED = 10.0f;
const float ACCEL = 4.0f;
const float DRAG = 1.0f / (FPS * 1000.0f);
const float BRAKE_DRAG = 50.0f / (FPS * 1000.0f);

void drawQuad(RenderWindow &w, Color c, float x1, float y1, float w1, float x2, float y2, float w2)
{
    ConvexShape shape(4);
    shape.setFillColor(c);
    shape.setPoint(0, Vector2f(x1-w1,y1));
    shape.setPoint(1, Vector2f(x2-w2,y2));
    shape.setPoint(2, Vector2f(x2+w2,y2));
    shape.setPoint(3, Vector2f(x1+w1,y1));
    w.draw(shape);
}

struct Line
{
    float x = 0,y = 0,z = 0; //3d center of line
    float screenX = 0,screenY = 0,screenWidth = 0; //screen coord
    float curve = 0,spriteX = 0,clip = 0,scale = 0;
    Sprite sprite;
    
    Line()
    {
        spriteX = curve = x = y = z = 0;
    }
    
    void project(float camX,float camY,float camZ)
    {
        scale = CAMERA_DEPTH/std::fmaxf(0, z-camZ);
        screenX = (1 + scale*(x - camX)) * WINDOW_WIDTH/2;
        screenY = (1 - scale*(y - camY)) * WINDOW_HEIGHT/2;
        screenWidth = scale * ROAD_WIDTH  * WINDOW_WIDTH/2;
    }
    
    void drawSprite(RenderWindow &app)
    {
        Sprite s = sprite;
        int w = s.getTextureRect().width;
        int h = s.getTextureRect().height;
        
        float destX = screenX + scale * spriteX * WINDOW_WIDTH/2;
        float destY = screenY + 4;
        float destW  = w * screenWidth / 266;
        float destH  = h * screenWidth / 266;
        
        destX += destW * spriteX; //offsetX
        destY += destH * (-1);    //offsetY
        
        float clipH = destY+destH-clip;
        if (clipH<0) clipH=0;
        
        if (clipH>=destH) return;
        s.setTextureRect(IntRect(0,0,w,h-h*clipH/destH));
        s.setScale(destW/w,destH/h);
        s.setPosition(destX, destY);
        app.draw(s);
    }
};

// moved these out of the function so it doesn't complain about stack usage
float xPos = 0;
float yPos = Y_POS_MIN;
float zPos = 0;
float speed = 0;
std::vector<Line> lines;

Texture t[50];
Sprite object[50];

float lerp(float a, float b, float t)
{
    return (a * (1.0f - t)) + (b * t);
}

int outrun()
{
    RenderWindow app(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Outrun Racing!");
    app.setFramerateLimit(FPS);

    for (int i=1;i<=7;i++)
    {
        t[i].loadFromFile("images/outrun/" + std::to_string(i) + ".png");
        t[i].setSmooth(true);
        object[i].setTexture(t[i]);
    }

    Texture bg;
    bg.loadFromFile("images/outrun/bg.png");
    bg.setRepeated(true);
    Sprite sBackground(bg);
    sBackground.setTextureRect(IntRect(0,0,5000,411));
    sBackground.setPosition(-2000,0);

    for(int i = 0; i < 1600; i++)
    {
        Line line;
        line.z = i*SEGMENT_LENGTH;

        if (i>300 && i<700) line.curve=0.5;
        if (i>1100) line.curve=-0.7;

        if (i<300 && i%20==0) {line.spriteX=-2.5; line.sprite=object[5];}
        if (i%17==0)          {line.spriteX=2.0; line.sprite=object[6];}
        if (i>300 && i%20==0) {line.spriteX=-0.7; line.sprite=object[4];}
        if (i>800 && i%20==0) {line.spriteX=-1.2; line.sprite=object[1];}
        if (i==400)           {line.spriteX=-1.2; line.sprite=object[7];}

        if (i>750) line.y = sin(i/30.0) * Y_POS_MIN;

        lines.push_back(line);
    }

    int lineCount = lines.size();

    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }
        
        float accel = ACCEL;
        float drag = DRAG;

        if (Keyboard::isKeyPressed(Keyboard::Right)) xPos+=0.1;
        if (Keyboard::isKeyPressed(Keyboard::Left)) xPos-=0.1;
        if (Keyboard::isKeyPressed(Keyboard::Tab)) accel *= 3.0f;
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            if (speed >= 0.0f)
            {
                speed += ACCEL * (1.0f - std::fmin(1.0f, std::fabsf(speed / MAX_SPEED))); // drive forward
            }
            else // apply brakes if reversing
            {
                drag += BRAKE_DRAG / std::fabsf(speed) + 0.001f; // simulate wheel slip
            }

        }
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            if (speed <= 0.0f)
            {
                speed -= ACCEL * (1.0f - std::fmin(1.0f, std::fabsf(speed / (MAX_SPEED * 0.25f)))); // reverse
            }
            else // apply brakes if driving forward
            {
                drag += BRAKE_DRAG / std::fabsf(speed) + 0.001f; // simulate wheel slip
            }
        }
        if (Keyboard::isKeyPressed(Keyboard::W)) yPos+=100;
        if (Keyboard::isKeyPressed(Keyboard::S)) yPos-=100;
        
        speed /= 1 + (std::fabsf(speed) * drag); // apply drag
        if (std::fabsf(speed) < MIN_SPEED && drag != DRAG) // is braking and going too slow
            speed = 0.0f; // stop

        // hard limit speed
        speed = std::fmaxf(-MAX_SPEED, std::fminf(speed, MAX_SPEED));

        zPos+=speed;
        //while (zPos >= lineCount*SEGMENT_LENGTH) zPos-=lineCount*SEGMENT_LENGTH;
        //while (zPos < 0) zPos += lineCount*SEGMENT_LENGTH;
        zPos = zPos - (lineCount * SEGMENT_LENGTH) * floor(zPos / (lineCount * SEGMENT_LENGTH));

        // clamp values to prevent crashes
        yPos = std::fmax(Y_POS_MIN, yPos);
        xPos = std::fmax(-X_POS_RANGE, std::fmin(X_POS_RANGE, xPos));
        
        float zPosFrac = std::fmodf(zPos, SEGMENT_LENGTH) / SEGMENT_LENGTH;
        app.clear(Color(105,205,4));
        app.draw(sBackground);
        int startPos = zPos/SEGMENT_LENGTH;
        float camH = lerp(lines[(size_t)startPos].y, lines[((size_t)startPos + 1) % lineCount].y, zPosFrac) + yPos;
        sBackground.move(lines[(size_t)startPos].curve*-speed*0.01f,0);
        
        float screenY = 0;
        float maxy = WINDOW_HEIGHT;
        float x = 0, dx = 0;
        
        ///////draw road////////
        for (int lineIdx = startPos; lineIdx<startPos+300; lineIdx++)
        {
            Line &curLine = lines[lineIdx%lineCount];

            float zPosProj = std::fmodf(zPos, lineCount * SEGMENT_LENGTH); // projection-relative Z position

            if (lineIdx >= lineCount)
                zPosProj -= lineCount * SEGMENT_LENGTH; // wrap around
            
            curLine.project(xPos*ROAD_WIDTH-x, camH, zPosProj);
            x+=dx;
            dx+=curLine.curve;
            
            Line nextLine = lines[(lineIdx + 1) % lineCount];
            Line prevLine = lines[std::max(1, lineIdx - 1) % lineCount];

            // blend between current and next line's height for smoothness
            screenY = lerp(curLine.screenY, nextLine.screenY, zPosFrac);
            curLine.clip=maxy;
            if (screenY >= maxy) continue;
            maxy = screenY;
            
            Color grass  = (lineIdx/3)%2?Color(16,200,16):Color(0,154,0);
            Color rumble = (lineIdx/3)%2?Color(255,255,255):Color(0,0,0);
            Color road   = (lineIdx/3)%2?Color(107,107,107):Color(105,105,105);
            
            drawQuad(app, grass, 0, prevLine.screenY, WINDOW_WIDTH, 0, curLine.screenY, WINDOW_WIDTH);
            drawQuad(app, rumble,prevLine.screenX, prevLine.screenY, prevLine.screenWidth*1.2f, curLine.screenX, curLine.screenY, curLine.screenWidth*1.2f);
            drawQuad(app, road,  prevLine.screenX, prevLine.screenY, prevLine.screenWidth, curLine.screenX, curLine.screenY, curLine.screenWidth);
        }

        ////////draw objects////////
        for(int lineIdx = startPos+300; lineIdx > startPos; lineIdx--)
            lines[lineIdx%lineCount].drawSprite(app);

        app.display();
    }

    return 0;
}
