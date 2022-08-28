#include "ltant.h"

Ground initGround(int hflen, int hfwid)
{
    int len = hflen * 2 + 1, wid = hfwid * 2 + 1; //试验场全长
    Ground ret = {len, wid, NULL};

    //分配内存
    ret.grd = (int **)calloc(len, sizeof(int *));
    for (int i = 0; i < wid; ++i)
    {
        ret.grd[i] = (int *)calloc(wid, sizeof(int));
    }
    return ret;
}

void printGrd(Ground *ground, int startx, int starty)
{
    startx /= 2;
    for (int i = 0; i < LINES; i++)
        for (int j = 0; j < COLS / 2; j++) // 一个方块占两格!!!
        {
            int tmp = *gotoGrd(ground, startx + j, starty + i);
            mvaddstr(i, j * 2, colorList[tmp]);
        }
    refresh(); // 刷新终端屏幕
}

int *gotoGrd(Ground *ground, int antx, int anty)
{
    int grdx = antx + ground->wid / 2;
    int grdy = anty + ground->len / 2;
    return ground->grd[grdy] + grdx;
}

void turnAnt(Ant *antp, Ground *ground)
{
    //变向
    int *ptr = gotoGrd(ground, antp->x, antp->y);
    int turns = *ptr * (-2) + 1;
    int antfwd = antp->fwd;
    antfwd = antfwd + turns;
    if (antfwd < 0)
        antfwd += 4;
    else if (antfwd >= 4)
        antfwd -= 4;

    //变格
    *ptr = 1 - *ptr;
    //蚂蚁前进
    antp->fwd = antfwd;
    antp->y += FWD_LIST[antp->fwd][0];
    antp->x += FWD_LIST[antp->fwd][1];
}

int main()
{
    Ground ground = initGround(2048, 2048);
    // *gotoGrd(&ground, 0, 0) = _BLACK; // 测试: 在试验场中央放置一个黑块
    Ant ant = {0, 0, 0};              // 初始化蚂蚁

    //设置文字编码(防乱码)并初始化终端。
    setlocale(LC_ALL, "");
    initscr();
    raw();
    noecho();

    int i=1;
    do
    {
        turnAnt(&ant, &ground);
        printGrd(&ground, -COLS / 2, -LINES / 2);
        i++;
    } while (i<15000);

    //退出curses模式并结束程序
    endwin();
    return 0;
}
