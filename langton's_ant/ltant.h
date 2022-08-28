#pragma once

#include <curses.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

/*试验场*/
typedef struct
{
    int len;        //上下长
    int wid;        //左右宽
    int **grd;      //试验场方格组
} Ground;
/*这个试验场的原点在中心，
* 大概长这样( y, x)
(-1,-1) (-1, 0) (-1,+1)
( 0,-1) ( 0, 0) ( 0,+1)
(+1,-1) (+1, 0) (+1,+1)
*/

//颜色
#define _BLACK  1
#define _WHITE  0

/*黑白字符列表
* 例如, colorList[_WHITE]就是白色方块"██".
* 使用数组而非if判断以提高速度。
*/
const char colorList[][8] = {
    "██",
    "  ",
    ""};

/*蚂蚁*/
typedef struct
{
    int x;      // 蚂蚁的横坐标
    int y;      // 蚂蚁的纵坐标
    int fwd;   // 蚂蚁的朝向，见下面的朝向列表
} Ant;

//朝向
#define _LEFT  -1
#define _RIGHT  1
#define _STATIC 0
#define _DOWN   1
#define _UP    -1

#define _TURN_LEFT   1
#define _TURN_RIGHT -1

//朝向列表: {fwdy,fwdx}
int FWD_LIST[][2] = {
    {_UP, _STATIC},
    {_STATIC, _LEFT},
    {_DOWN, _STATIC},
    {_STATIC, _RIGHT}
};

/*函数: 初始化试验场
 * 参数: 上下半长, 左右半宽
 * 返回: 初始化为0的试验场 */
Ground initGround(int hflen, int hfwid);

/*函数: 打印某一位置的试验场
 * 参数: 试验场, 起始位置x,y坐标
 * 返回: 无 */
void printGrd(Ground *ground, int startx, int starty);

/*函数: 找到特定位置的试验场方格
 * 参数: 指向试验场的指针, xy坐标(一般用于给蚂蚁找位子)
 * 返回: 指向方格的指针 */
int *gotoGrd(Ground *ground, int antx, int anty);

/*函数: 让蚂蚁转弯并前进
 * 参数: Ant*蚂蚁指针, Ground*
 * 返回: 无 */
void turnAnt(Ant *antp, Ground *ground);
