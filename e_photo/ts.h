#ifndef __TS_H
#define __TS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <linux/fb.h>

// 触摸屏
#include <linux/input.h>

// 液晶屏
#include <sys/mman.h>

#define CLICK  1//单击
#define DCLICK 2//双击
#define LEFT   3//向左
#define RIGHT  4//向右
#define UP     5//向上
#define DOWN   6//向下

int tp_init();
int get_motion(int tp);

#endif