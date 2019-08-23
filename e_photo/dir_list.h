#ifndef __DIR_LIST_H
#define __DIR_LIST_H

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>

#include "photo.h"
//将双向链表的数据类型指定为char *data
#define DL_NODE_TYPE struct photo_node *
#include "dlink_list.h"

//初始化目录，边路该目录下的文件名
linklist dir_list_init(char *target);

//将目录下面的文件名name，放到链表photo_dlist中去
void add_name(char *name, linklist photo_dlist);

//创建一个新的节点
linklist produce_new_node(char *n);

#endif