#ifndef CONIO_H
#define CONIO_H

// Linux 替代 DOS/Windows 的 <conio.h>。
// 原始 PL/0 源码 (2003 年 VC/TC 代码) 用到了 getch()，Linux 无 conio.h，
// 这里用 termios 提供等价实现，使源码无需改动即可在 GCC 下编译。

#include <termios.h>
#include <unistd.h>

// 不回显、不需回车的单字符读取（按任意键继续）。
static inline int getch(void)
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

#endif
