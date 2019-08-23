#ifndef __DLINK_LIST_H
#define __DLINK_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef DL_NODE_TYPE
#define DL_NODE_TYPE int
#endif

typedef DL_NODE_TYPE datatype;

typedef struct node
{
	union
	{
		datatype mydata;
		int len;
	}data;
	struct node *prev;
	struct node *next;
}listnode, *linklist;

//创建一个空的双向循环链表，空的链表返回一个头节点,不存在有效数据
linklist init_list();

/*
//产生一个有效数据时n的链表节点，成功时返回指向节点的指针
linklist produce_new_node(datatype n);
*/

//将结尾的数据节点从链表中取出来，返回给用户
linklist list_remove_tail(linklist head);

//将一个节点数据放到链表的结尾
void list_add_tail(linklist new_node, linklist head);

//将第一个节点数据取出返回给用户
linklist list_remove_initiate(linklist head);

//将一个节点数据放到链表的开头
void list_add_initiate(linklist new_node, linklist head);

//将一个特定的节点从链表中取出来，并将此节点返回给用户
linklist list_remove_node(linklist tmp, linklist head);

//遍历链表，具体的数据操作由函数指针传递
void travel_dlist(linklist head, void(*fun)(void *));

//销毁链表内存
void destroy_list(linklist head);

#endif