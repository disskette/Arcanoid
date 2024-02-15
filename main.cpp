#include <X11/Xlib.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <list>

int WW = 1100; // window width
int WH = 900; // window height
Window myWindow;
Display *ourDisplay;
int s;

class Brick
{
    Display *d;
    Window   w;
    int      s;
    int x;
    int y;
    int width, height;

public:
    Brick(Display *d, Window w, int s, int x , int y, int width, int height)
    : d(d), w(w), s(s), x(x), y(y), width(width), height(height) {};

    void Draw() const
    {
        XFillRectangle(d, w, DefaultGC(d,s), x, y, width, height);
    };

    bool CheckCollision(int& ball_x, int& ball_y, int& moveX, int& moveY, int ball_diameter)
    {

        if (ball_x >= x - ball_diameter && ball_x <= x+width)
        {
            if (ball_y >= y - ball_diameter && ball_y <= y+height)
            {
                // Changing the Ball`s direction
                if (ball_y + ball_diameter - moveY <= y && y <= ball_y + 16) moveY *= -1;
                else if (ball_y - moveY >= y + height && y + height >= ball_y) moveY *= -1;
                else if (ball_x + 16 - moveX <= x && x <= ball_x + 16) moveX *= -1;
                else if (ball_x - moveX >= x + width  && x + width  >= ball_x) moveX *=-1;
                return 1;
            }
        }
        return 0;

    }
};

class Ball
{
    friend class Field;
    friend class Timer;
    Display *d;
    Window   w;
    int      s;
    int x;
    int y;
    int diameter; 

public:
    Ball(Display *d, Window w, int s, int diameter)
    : d(d), w(w), s(s), diameter(diameter)
    {
        x = WW / 2 +20;
        y = WH-120;
    };

    ~Ball(){};

    void Draw() const
    {
        XFillArc(d, w, DefaultGC(d,s), x, y, diameter, diameter, 0, 360*64);
    }
};

class Platform
{
    friend class Field;

    Display *d;
    Window   w;
    int      s;
    int x;
    int y;
    int width, height;
    int coeff = 0;

public:
    Platform(Display *d, Window w, int s)
    :d(d), w(w), s(s), width(90), height(10)
    {
        y = WH - 90;
        x = WW / 2 - 10;
    };

    void Draw() const
    {
        XFillRectangle(d, w, DefaultGC(d,s), x, y, width, height);
    };

    bool goRight(const int& d = 1)
    {
        if (coeff + d <=2)
        {
            coeff += d;
            return 0;
        } 

        return 1;
        
    }

    bool goLeft(const int& d = -1)
    {
        if (coeff + d >= -2)
        {
            coeff += d;
            return 0;
        } 
        return 1;
        
    }

    ~Platform(){};
};

class Field
{

    Display *d;
    Window   w;
    int      s;
    Platform& p;
    Ball& b; 
    std::list<Brick> Bricks;
    

    // Ball speed parameters
    int moveX = 3, moveY = -5;

    // Platform speed parameterss
    int delta = 4;

    
public:
    Field(Display *d, Window w, int s, Platform& p, Ball& b, std::list<Brick>& Bricks)
    :d(d), w(w), s(s), p(p), b(b), Bricks(Bricks)
    {};

    void moveBall()
    {
        b.x += moveX;
        b.y += moveY;
    }

    void movePlatform()
    {
        if (p.x + (p.coeff*delta) >= 1 && (p.x+p.width) + (p.coeff*delta) <= WW -1) 
            p.x += p.coeff*delta;
        else p.coeff = 0;

    }

    void CheckPlatformCollision()
    {
        if ((b.y +b.diameter) >= p.y && b.y <= p.y + p.height)
        {
            if (b.x  >= p.x - b.diameter && b.x <= p.x + p.width) 
            {

                switch (p.coeff)
                {
                case 0:
                    moveY *= -1;
                    break;
                case 1:
                        if (moveX > 0) moveX = 3;
                        else if (moveX < 0) moveX = -2;
                        moveY = -8 + abs(moveX);
                    break;
                case 2:

                        if (moveX > 0) moveX = 4;
                        else if (moveX < 0) moveX = -1;
                        moveY = -8 + abs(moveX);
                    break;

                case -1:

                    if (moveX > 0) moveX = 2;
                    else if (moveX < 0) moveX = -3;
                    moveY = -8 + abs(moveX);
                
                case -2:
                    if (moveX > 0) moveX =1;
                    else if (moveX < 0) moveX = -4;
                    moveY = -8 + abs(moveX);

                default:
                    break;
            } 
            b.y = p.y - b.diameter - 2; // Moving the Ball slightly away from the Platform to avoid incorrect collisions
            
            } 

        }
    }

    bool CheckWallCollision()
    {
        if (b.x <= 3)
        {
            b.x = 4; // Moving the Ball slightly away from the wall to avoid incorrect collisions
            moveX = moveX * (-1); // Left wall
        } 
        else if (b.x + b.diameter >= WW-3)
        {
            b.x = WW -3 - 16;
            moveX = moveX * (-1); // Right wall
        } 
        else if (b.y <= 3)
        {
            moveY = moveY * (-1); // Top wall
            b.y = 4;

        } 

        else if (b.y + b.diameter >= WH-3) // Bottom wall -> failure
        {
            moveX = 0;
            moveY = 0;   
            return 1;
        }
        return 0;
    }

    void DrawBricks()
    {
        for (std::list<Brick>::iterator it=Bricks.begin(); it!= Bricks.end(); ++it)
        {
            Brick br = *it;
            br.Draw();
        }
    }

    bool CheckBricksCollision()
    {
        for (std::list<Brick>::iterator it=Bricks.begin(); it!= Bricks.end(); ++it)
        {
            Brick br = *it;
            if (br.CheckCollision(b.x, b.y, moveX, moveY, b.diameter)) it =Bricks.erase(it);
            if (Bricks.size()== 0) break;
        }
        if (Bricks.size() == 0) return 1;
        return 0;

    }

    ~Field(){};
};

class Timer {
    size_t   term;
    void   (Field::*func)();
    bool     stop;
    Field* f;
    
  public:
    Timer()
      : term(0), func(0), stop(false)
    {};

    Timer(void (Field::*func)(), Field* f)
    : term(0), func(func), stop(false), f(f)
    {}
    
    void StartTimer(size_t usec)
    {
      term = usec;
      
      struct timeval cur;
      struct timeval old;
      size_t delta;
      
      gettimeofday(&old, 0);
      while(!stop)
      {
        gettimeofday(&cur, 0);
	
	    delta  = cur.tv_sec  - old.tv_sec;
	    delta *= 10000;
	    delta += cur.tv_usec - old.tv_usec;
	
	    if(delta >= term)
	    {
	        if(func) (f->*func)();
	        stop = true;
	    }
      }
    }

    bool get_state() const { return stop; }

    void set_state() { stop = false; }
};


int main()
{
    
    ourDisplay = XOpenDisplay(NULL);
    if ( ourDisplay == NULL) return 1;


    int s=DefaultScreen(ourDisplay);        
    Window rootWindow=RootWindow(ourDisplay, s); 
    unsigned long bgcolor=WhitePixel(ourDisplay, s); 


    myWindow=XCreateSimpleWindow(ourDisplay,rootWindow,100, 100, WW, WH,
    0, 0, bgcolor);
    

    XSelectInput(ourDisplay,myWindow,
    KeyPressMask | KeyReleaseMask |
    ExposureMask );

    XMapWindow(ourDisplay, myWindow);    
    XFlush(ourDisplay);

    int ball_diameter = 16;
    int brick_x = 20, brick_y = 20;
    int brick_width = 90, brick_height = 25;


    Platform p(ourDisplay,myWindow,s);
    Ball b(ourDisplay, myWindow,s, ball_diameter);
    std::list<Brick> Bricks;

 
    for (int row = 0; row < 6; row++)
    {

        while (brick_x + brick_width < WW-10)
        {
            Bricks.push_back(Brick(ourDisplay, myWindow, s, brick_x, brick_y, brick_width, brick_height));
            brick_x += 130;
        }
        
        brick_x = 20;
        brick_y += 45;

    }

    Field f(ourDisplay,myWindow,s, p, b, Bricks);
    Timer t_ball(&Field::moveBall, &f);
    Timer t_platform(&Field::movePlatform, &f);
    t_ball.StartTimer(1);
    t_platform.StartTimer(1);


    while (1)
    {

        XExposeEvent xe = {Expose, 0, 1, ourDisplay, myWindow, 0, 0, 
            WW, WH,
            0};
        XEvent event;
        XNextEvent(ourDisplay,&event);


        if (event.type == KeyPress)
        {
            if (event.xkey.keycode == 114) 
                p.goRight();
            if (event.xkey.keycode == 113) 
                p.goLeft();
        }

        if (event.type == Expose)
        {
            if (t_ball.get_state())
            {
                t_ball.set_state();
                t_ball.StartTimer(2000);
            }

            if (t_platform.get_state())
            {
                t_platform.set_state();
                t_platform.StartTimer(2000);
            }

            f.CheckPlatformCollision();
            f.CheckBricksCollision();
        }

        XClearWindow(ourDisplay, myWindow);
        p.Draw();
        b.Draw();
        f.DrawBricks();

        if(!XPending(ourDisplay))
        {
        XSendEvent(ourDisplay, myWindow, False, ExposureMask, (XEvent *) &xe);
        usleep(10000);
        }

        if (f.CheckWallCollision()) 
        {
            std::cout << "Game Over";
            break;
        }

        if (f.CheckBricksCollision()) 
        {
            std::cout << "You won!";
            break;
        }

        XFlush(ourDisplay);
    }

    XDestroyWindow(ourDisplay, myWindow);
    XCloseDisplay(ourDisplay);
  
    return 0;
}