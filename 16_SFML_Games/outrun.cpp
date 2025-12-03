#include <SFML/Graphics.hpp>
#include "outrun.h"
#include "outrun_camera.h"
using namespace sf;

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
        s.setTextureRect(IntRect(0,0,w,(int)(h-h*clipH/destH)));
        s.setScale(destW/w,destH/h);
        s.setPosition(destX, destY);
        app.draw(s);
    }
};

int lineCount;

// moved these out of the function so it doesn't complain about stack usage
Camera camera;
std::vector<Line> lines;


float lerp(float a, float b, float t)
{
    return (a * (1.0f - t)) + (b * t);
}

int outrun()
{
    RenderWindow app(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Outrun Racing!");
    app.setFramerateLimit(FPS);
    
    Texture t[50];
    Sprite object[50];

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
        line.z = (float)(i*SEGMENT_LENGTH);

        if (i>300 && i<700) line.curve=0.5f;
        if (i>1100) line.curve=-0.7f;

        if (i<300 && i%20==0) {line.spriteX=-2.5f; line.sprite=object[5];}
        if (i%17==0)          {line.spriteX=2.0f; line.sprite=object[6];}
        if (i>300 && i%20==0) {line.spriteX=-0.7f; line.sprite=object[4];}
        if (i>800 && i%20==0) {line.spriteX=-1.2f; line.sprite=object[1];}
        if (i==400)           {line.spriteX=-1.2f; line.sprite=object[7];}

        if (i>750) line.y = sin(i/30.0f) * Y_POS_MIN;

        lines.push_back(line);
    }

    lineCount = lines.size();

    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }
        
        camera.update();
        
        float zPosFrac = std::fmodf(camera.zPos(), SEGMENT_LENGTH) / SEGMENT_LENGTH;
        app.clear(Color(105,205,4));
        app.draw(sBackground);
        int startPos = (int)(camera.zPos()/SEGMENT_LENGTH);
        float camH = lerp(lines[(size_t)startPos].y, lines[((size_t)startPos + 1) % lineCount].y, zPosFrac) + camera.yPos();
        sBackground.move(lines[(size_t)startPos].curve*(-camera.speed()) * 0.01f, 0);
        
        float screenY = 0;
        float maxy = WINDOW_HEIGHT;
        float x = 0, dx = 0;
        
        ///////draw road////////
        for (int lineIdx = startPos; lineIdx<startPos+300; lineIdx++)
        {
            Line &curLine = lines[lineIdx%lineCount];

            float zPosProj = std::fmodf(camera.zPos(), (float)(lineCount * SEGMENT_LENGTH)); // projection-relative Z position

            if (lineIdx >= lineCount)
                zPosProj -= lineCount * SEGMENT_LENGTH; // wrap around
            
            curLine.project(camera.xPos()*ROAD_WIDTH-x, camH, zPosProj);
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
