#include "QueueLinked.h"
#include "olcPixelGameEngine.h"

using namespace std;

class bean : public olc::PixelGameEngine
{
  public:
    int posX, posY;
    int color;       //  1
    int orientation; //4 0 2
                     //  3
    bean(float x, float y, int c, int o)
    {
        posX = x;
        posY = y;
        color = c;
        orientation = o;
    };

    bean()
    {
        posX = 0;
        posY = 0;
        color = 1;
        orientation = 0;
    }
};

class Beans : public olc::PixelGameEngine
{
  public:
    Beans()
    {
        sAppName = "Beans";
    }

  private:
    int pix = 2;
    int width = 54 / pix;
    int speed = 54 / pix;
    int step = 12;
    bool pop = false;
    bool fall = false;
    bool gameOver = false;
    bool wait = false;
    bool spawn = false;
    const static int boardWid = 7;
    const static int boardHeight = 12;
    int playerWindowWidth = 320 / pix;
    int playerWindowHeight = 645 / pix;
    int playerWindowX = 42 / pix;
    int count = 0;
    // int tempX, tempY, tempC = 0;
    int floodCount = 0;
    int gameTime = 0;
    int playerBoardArray[(boardWid + 1) * boardHeight];
    int floodArray[(boardWid + 1) * boardHeight];
    vector<int> toBePopped;
    bean currBean;
    bean pairBean;
    bean nextBean;
    bean nextBeanPair;

    bool OnUserCreate() override
    {
        for (int x = 0; x <= boardWid; x++)
            for (int y = 0; y <= boardHeight; y++)
            {
                //if x is 0 or x is width or y is height then put a 9 else put a 0
                playerBoardArray[y * boardWid + x] = (x == 0 || x == boardWid || y == boardHeight) ? 9 : 0;
                floodArray[y * boardWid + x] = (x == 0 || x == boardWid || y == boardHeight) ? 9 : 0;
            }

        // for (int x = 0; x < boardWid; x++)
        //     for (int y = 0; y < boardHeight; y++)
        //floodArray[y * boardWid + x] = 0;

        srand(time(NULL));

        currBean = bean(boardWid / 2, 0, rand() % 4 + 1, 0);
        pairBean = bean(boardWid / 2, -1, rand() % 4 + 1, 1);

        nextBean = bean(boardWid / 2, 0, rand() % 4 + 1, 0);
        nextBeanPair = bean(boardWid / 2, -1, rand() % 4 + 1, 1);

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        int row = 0;
        if (!gameOver)
        {
            this_thread::sleep_for(50ms); // Small Step = 1 Game Tick
            count++;
            if (wait)
                wait = !(count >= step);

            fall = (count >= step);

            //If there is no piece falling then we must be able to spawn
            //But first we need to check if there is a hanging piece or a pop.
            if (spawn)
            {
                //If we are not popping check if there is a fall. This is done because we this is a costly function.
                if (!pop)
                    row = checkArrayForEmptyFallSpace();

                //The function returns a row that needs to fall. If this is anything but 0 do a row fall.
                if (row != 0)
                {
                    //This waits for a fall after a pop
                    if (!wait)
                    {
                        needRowToFall(row);
                        row = 0;
                    }
                }
                //If nothing is falling then check for a pop
                else
                {
                    //Check if we are in the process of popping already. If not check for pop
                    if (!pop)
                        for (int y = 11; y >= 0; y--)
                            for (int x = 1; x < boardWid; x++)
                                if (playerBoardArray[y * boardWid + x] != 0)
                                    if (needToPop(x, y, playerBoardArray[y * boardWid + x]))
                                    {
                                        //This function return true if we need to pop else it just updates the array.
                                        wait = true;
                                        pop = true;
                                        count = 0;
                                    }

                    //If we need to pop go here
                    if (pop)
                    {
                        //Make sure we are not waiting
                        if (!wait)
                        {
                            //Pop the cells and reset.
                            popCells();
                            pop = false;
                            wait = true;
                            count = 0;
                        }
                    }
                    //Else we clear the flood array and spawn the next bean
                    else
                    {
                        //Clear array
                        for (int y = 11; y >= 0; y--)
                            for (int x = 1; x < boardWid; x++)
                            {
                                floodArray[y * boardWid + x] = 0;
                            }

                        //Spawn next piece
                        if (doesPieceFit(boardWid / 2, 0, 0, pairBean.orientation))
                        {
                            currBean = nextBean;
                            pairBean = nextBeanPair;
                            nextBean = bean(boardWid / 2, 0, rand() % 4 + 1, 0);
                            nextBeanPair = bean(boardWid / 2, -1, rand() % 4 + 1, 1);
                            spawn = false;
                        }
                        //If we cant. Game is over
                        else
                            gameOver = true;
                    }
                }
            }

            //User input
            if (!wait && !spawn)
            {
                if (GetKey(olc::Key::S).bHeld) //Down
                {
                    if (doesPieceFit(currBean.posX, currBean.posY, 1, pairBean.orientation))
                    {
                        currBean.posY++;
                        pairBean.posY++;
                    }
                }
                if (GetKey(olc::Key::A).bHeld) //Left
                {
                    if (doesPieceFit(currBean.posX, currBean.posY, 2, pairBean.orientation))
                    {
                        pairBean.posX--;
                        currBean.posX--;
                    }
                }
                if (GetKey(olc::Key::D).bHeld) //Right
                {
                    if (doesPieceFit(currBean.posX, currBean.posY, 3, pairBean.orientation))
                    {
                        currBean.posX++;
                        pairBean.posX++;
                    }
                }
                if (GetKey(olc::Key::Q).bPressed) //Rotate Counter
                {
                    if (doesPieceFit(currBean.posX, currBean.posY, 4, pairBean.orientation))
                    {
                        rotateBean(-1);
                    }
                }
                if (GetKey(olc::Key::E).bPressed) //Rotate Clock
                {
                    if (doesPieceFit(currBean.posX, currBean.posY, 5, pairBean.orientation))
                    {
                        rotateBean(1);
                    }
                }
                if (GetKey(olc::Key::P).bPressed)
                {
                    printPlayerArray();
                    cout << fall << endl;
                }
            }

            //Make piece fall down
            if (fall && !wait && !spawn)
            {
                count = 0;
                gameTime++;
                if (gameTime % 25 == 0)
                    if (step >= 10)
                        step--;

                //If the piece fits then fall
                if (doesPieceFit(currBean.posX, currBean.posY, 1, pairBean.orientation))
                {
                    currBean.posY++;
                    pairBean.posY++;
                }
                //Else the piece is at the bottom and we need to spawn another
                else
                {
                    playerBoardArray[currBean.posY * boardWid + currBean.posX] = currBean.color;
                    playerBoardArray[pairBean.posY * boardWid + pairBean.posX] = pairBean.color;

                    spawn = true;
                }
            }

            FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
            DrawRect(playerWindowX, playerWindowX, playerWindowWidth + 1, playerWindowHeight, olc::RED);

            //Draws the board
            for (int x = 0; x <= boardWid; x++)
                for (int y = 0; y <= boardHeight; y++)
                {
                    drawBean(playerBoardArray[y * boardWid + x], x, y);
                    //debug border
                    // if (playerBoardArray[y * boardWid + x] == 0)
                    //     FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width, olc::RED);
                    // else
                    //     FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width, olc::BLUE);
                }

            //Draw curr bean
            if (!spawn)
            {
                DrawRect((currBean.posX - 1) * width + playerWindowX - 1, currBean.posY * width + playerWindowX - 1, width, width, olc::WHITE);
                drawBean(currBean.color, currBean.posX, currBean.posY);
                drawBean(pairBean.color, pairBean.posX, pairBean.posY);
            }

            drawBean(nextBean.color, 8, 1);
            drawBean(nextBeanPair.color, 8, 0);
        }
        else //Game over
        {
            DrawRect(playerWindowX, playerWindowX, playerWindowWidth + 1, playerWindowHeight, olc::RED);
            //Draws the board
            for (int x = 0; x <= boardWid; x++)
                for (int y = 0; y <= boardHeight; y++)
                    drawBean(playerBoardArray[y * boardWid + x], x, y);
            //Draw curr bean
            drawBean(currBean.color, currBean.posX, currBean.posY);
            DrawString(2, playerWindowHeight / 2, "Game Over", olc::WHITE, 5);
        }
        return true;
    }

    bool doesPieceFit(int beanX, int beanY, int movement, int orientation) //1 down, 2 left, 3 right
    {
        if (movement == 1) //Down
        {
            if (orientation == 0)
            {
                if (pairBean.orientation == 1 || pairBean.orientation == 3)
                    return false;
                if (playerBoardArray[(beanY + 1) * boardWid + beanX] != 0)
                    return false;
                else
                    return true;
            }
            if (playerBoardArray[(beanY + 1) * boardWid + beanX] != 0)
                return false;

            if (orientation == 1 && playerBoardArray[(beanY + 1) * boardWid + beanX] != 0) //base
                return false;
            else if (orientation == 2 && playerBoardArray[(beanY + 1) * boardWid + beanX + 1] != 0)
                return false;
            else if (orientation == 3 && playerBoardArray[(beanY + 2) * boardWid + beanX] != 0)
                return false;
            else if (orientation == 4 && playerBoardArray[(beanY + 1) * boardWid + beanX - 1] != 0)
                return false;
        }
        if (movement == 2) //Left
        {
            if (playerBoardArray[beanY * boardWid + beanX - 1] != 0)
                return false;

            if (beanY > 0)
            {
                if (orientation == 1 && playerBoardArray[(beanY - 1) * boardWid + beanX - 1] != 0)
                    return false;
                else if (orientation == 2 && playerBoardArray[beanY * boardWid + beanX - 1] != 0) //base
                    return false;
                else if (orientation == 3 && playerBoardArray[(beanY + 1) * boardWid + beanX - 1] != 0)
                    return false;
                else if (orientation == 4 && playerBoardArray[beanY * boardWid + beanX - 2] != 0)
                    return false;
            }
            else if (orientation == 4 && playerBoardArray[beanY * boardWid + beanX - 2] != 0)
                return false;
        }
        if (movement == 3) //Right
        {
            if (playerBoardArray[beanY * boardWid + beanX + 1] != 0)
                return false;

            if (beanY > 0)
            {
                if (orientation == 1 && playerBoardArray[(beanY - 1) * boardWid + beanX + 1] != 0)
                    return false;
                else if (orientation == 2 && playerBoardArray[beanY * boardWid + beanX + 2] != 0)
                    return false;
                else if (orientation == 3 && playerBoardArray[(beanY + 1) * boardWid + beanX + 1] != 0)
                    return false;
                else if (orientation == 4 && playerBoardArray[beanY * boardWid + beanX + 1] != 0) //base
                    return false;
            }
            else if (orientation == 2 && playerBoardArray[beanY * boardWid + beanX + 2] != 0)
                return false;
        }
        if (movement == 4) //Rotate Counter
        {
            if (beanY > 0)
            {
                if (orientation == 1 && playerBoardArray[(beanY)*boardWid + beanX - 1] != 0)
                    return false;
                else if (orientation == 2 && playerBoardArray[(beanY - 1) * boardWid + beanX] != 0)
                    return false;
                else if (orientation == 3 && playerBoardArray[(beanY)*boardWid + beanX + 1] != 0)
                    return false;
                else if (orientation == 4 && playerBoardArray[(beanY + 1) * boardWid + beanX] != 0)
                    return false;
            }
        }
        if (movement == 5) //Rotate Clock
        {
            if (beanY > 0)
            {
                if (orientation == 1 && playerBoardArray[(beanY)*boardWid + beanX + 1] != 0)
                    return false;
                else if (orientation == 2 && playerBoardArray[(beanY + 1) * boardWid + beanX] != 0)
                    return false;
                else if (orientation == 3 && playerBoardArray[(beanY)*boardWid + beanX - 1] != 0)
                    return false;
                else if (orientation == 4 && playerBoardArray[(beanY - 1) * boardWid + beanX] != 0)
                    return false;
            }
        }
        else if (playerBoardArray[beanY * boardWid + beanX] != 0)
            return false;
        return true;
    }

    void drawBean(int color, int x, int y)
    {
        switch (color)
        {
        case 1: //Red
        {
            FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width - 1, olc::RED);
            break;
        }
        case 2: //Green
        {
            FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width - 1, olc::GREEN);
            break;
        }
        case 3: //Blue
        {
            FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width - 1, olc::BLUE);
            break;
        }
        case 4: //Yellow
        {
            FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width - 1, olc::YELLOW);
            break;
        }
        case 5: //Purple
        {
            FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width - 1, olc::MAGENTA);
            break;
        }
        case 6: //Grey
        {
            FillRect((x - 1) * width + playerWindowX, y * width + playerWindowX, width - 1, width - 1, olc::GREY);
            break;
        }
        }
    }

    void rotateBean(int dir)
    {
        if (dir == -1)
        {
            if (pairBean.posX == 1 && pairBean.orientation == 1)
                return;
            if (pairBean.orientation == 1)
                pairBean.orientation = 4;
            else
                pairBean.orientation--;

            switch (pairBean.orientation)
            {
            case 1:
                pairBean.posX--;
                pairBean.posY--;
                break;
            case 2:
                if (pairBean.posX == 6)
                {
                    pairBean.orientation++;
                    break;
                }
                pairBean.posX++;
                pairBean.posY--;
                break;
            case 3:
                if (pairBean.posY == 11)
                {
                    pairBean.orientation++;
                    break;
                }
                pairBean.posX++;
                pairBean.posY++;
                break;
            case 4:
                pairBean.posX--;
                pairBean.posY++;
                break;
            }
        }
        else if (dir == 1)
        {
            if (pairBean.orientation == 4)
                pairBean.orientation = 1;
            else
                pairBean.orientation++;

            switch (pairBean.orientation)
            {
            case 1:
                pairBean.posX++;
                pairBean.posY--;
                break;
            case 2:
                if (pairBean.posX == 6)
                {
                    pairBean.orientation--;
                    break;
                }
                pairBean.posX++;
                pairBean.posY++;
                break;
            case 3:
                if (pairBean.posY == 11)
                {
                    pairBean.orientation--;
                    break;
                }
                pairBean.posX--;
                pairBean.posY++;
                break;
            case 4:
                if (pairBean.posX == 1)
                {
                    pairBean.orientation--;
                    break;
                }
                pairBean.posX--;
                pairBean.posY--;
                break;
            }
        }
    }

    void floodFill(int sourceArray[], int destArray[], int x, int y, int color, int colorNew)
    {
        if (sourceArray[y * boardWid + x] == color && destArray[y * boardWid + x] != colorNew)
        {
            floodCount++;
            destArray[y * boardWid + x] = colorNew;
            toBePopped.push_back(x);
            toBePopped.push_back(y);
        }
        else
            return;

        floodFill(sourceArray, destArray, x + 1, y, color, colorNew);
        floodFill(sourceArray, destArray, x - 1, y, color, colorNew);
        floodFill(sourceArray, destArray, x, y + 1, color, colorNew);
        floodFill(sourceArray, destArray, x, y - 1, color, colorNew);
    }

    void printPlayerArray()
    {
        for (int y = 0; y <= boardHeight; y++)
        {
            for (int x = 0; x <= boardWid; x++)
            {
                cout << playerBoardArray[y * boardWid + x] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    void printFloodArray()
    {
        for (int y = 0; y <= boardHeight; y++)
        {
            for (int x = 0; x <= boardWid; x++)
            {
                cout << floodArray[y * boardWid + x] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    int checkArrayForEmptyFallSpace()
    {
        for (int y = 10; y >= 0; y--)
            for (int x = 1; x < boardWid; x++)
                if (playerBoardArray[y * boardWid + x] != 0)
                    if (playerBoardArray[(y + 1) * boardWid + x] == 0)
                    {
                        return y;
                    }
        return 0;
    }

    void needRowToFall(int row)
    {
        int temp = 0;
        for (int i = 1; i < boardWid; i++)
            if (playerBoardArray[row * boardWid + i] != 0)
                if (playerBoardArray[(row + 1) * boardWid + i] == 0)
                {
                    int index = row * boardWid + i;
                    temp = playerBoardArray[index];
                    playerBoardArray[index] = 0;
                    playerBoardArray[index + boardWid] = temp;

                    temp = floodArray[index];
                    floodArray[index] = 0;
                    floodArray[index + boardWid] = temp;
                }
    }

    bool needToPop(int x, int y, int color)
    {
        floodCount = 0;
        bool popp = false;

        floodFill(playerBoardArray, floodArray, x, y, color, color);

        for (int i = 0; i < floodCount; i++)
        {
            int tempY = toBePopped.back();
            toBePopped.pop_back();

            int tempX = toBePopped.back();
            toBePopped.pop_back();

            floodArray[tempY * boardWid + tempX] = -floodCount;
        }

        if (floodCount >= 4)
            popp = true;

        return popp;
    }

    void popCells()
    {
        for (int y = 11; y >= 0; y--)
            for (int x = 1; x < boardWid; x++)
                if (floodArray[y * boardWid + x] <= -4)
                {
                    floodArray[y * boardWid + x] = 0;
                    playerBoardArray[y * boardWid + x] = 0;
                }
    }
};

int main()
{
    Beans b;
    int pix = 2;
    if (b.Construct(1080 / pix, 720 / pix, pix, pix))
        b.Start();
    return 0;
};