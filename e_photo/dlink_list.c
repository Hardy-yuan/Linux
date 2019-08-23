#include "dlink_list.h"

//创建一个空的双向循环链表，空的链表返回一个头节点,不存在有效数据
linklist init_list()
{
	linklist head = calloc(1, sizeof(listnode));
	if(head != NULL)
	{
		//将头节点的前向指针和后向指针均指向自己
		head->prev = head;
		head->next = head;
		//将头节点中存放链表长度的值更新为0
		head->data.len = 0;
		return head;
	}
	return NULL;
}

/*
//产生一个有效数据时n的链表节点，成功时返回指向节点的指针
linklist produce_new_node(datatype n)
{
	linklist new_node = calloc(1, sizeof(listnode));
	if(new_node != NULL)
	{
		new_node->prev = NULL;
		new_node->next = NULL;
		new_node->data.mydata = n;
		return new_node;
	}
	return NULL;
}
*/

//将结尾的数据节点从链表中取出来，返回给用户
linklist list_remove_tail(linklist head)
{
	//判断链表是否为空
	if(head->data.len == 0)
		return NULL;
	//定义一个临时变量指向最后一个节点
	linklist tail = head->prev;
	//最后一个的上一个节点的next指向最后一个节点的下一个节点（即头节点）
	tail->prev->next = tail->next;
	//最后一个的下一个节点的prev指向最后一个节点的上一个节点
	tail->next->prev = tail->prev;
	//将结尾节点的前向指针和后向指针全部指向NULL
	tail->next = NULL;
	tail->prev = NULL;

	//更新链表的长度
	head->data.len--;

	return tail;
}

//将一个节点数据放到链表的结尾
void list_add_tail(linklist new_node, linklist head)
{
	//定义一个临时变量存放头节点的前一个节点（即最后一个节点）
	linklist tmp = head->prev;
	//新节点的前向指针指向最后一个节点
	new_node->prev = tmp;
	//将新节点的后向指针指向头节点
	new_node->next = head;
	//将最后一个节点的后向指针指向新节点
	tmp->next = new_node;
	//将头节点的前向指针指向新节点
	head->prev = new_node;

	//更新链表数据长度
	head->data.len++;
}

//将第一个节点数据取出返回给用户
linklist list_remove_initiate(linklist head)
{
	//判断链表是否为空
	if(head->data.len == 0)
		return NULL;
	//定义一个临时指针变量指向第一个数据
	linklist tmp = head->next;
	//将第一个数据的下一个数据和第一个数据的上一个数据之间连接
	tmp->prev->next = tmp->next;
	tmp->next->prev = tmp->prev;
	//将要取出的数据的前向指针和后向指针全部都指向空
	tmp->prev = NULL;
	tmp->next = NULL;

	//更新链表的长度
	head->data.len--;

	return tmp;
}

//将一个节点数据放到链表的开头
void list_add_initiate(linklist new_node, linklist head)
{
	//定义一个临时指针指向第一个节点
	linklist tmp = head->next;
	//新节点的前向指针指向头节点
	new_node->prev = head;
	//新节点的后向指针指向原链表的第一个节点
	new_node->next = tmp;
	//头节点指向新节点
	head->next = new_node;
	//原链表的第一个节点指向新节点
	tmp->prev = new_node;

	//更新链表的长度
	head->data.len++;
}

//将某个指定的节点从链表中取出来，并且返回给用户
linklist list_remove_node(linklist tmp, linklist head)
{
	//检测链表是否为空
	if(head->data.len == 0)
		return NULL;
	//指定节点的前面一个节点的next指向指定节点的后面一个节点
	tmp->prev->next = tmp->next;
	//指定节点的后面一个节点的prev指向指定节点的前面一个节点
	tmp->next->prev = tmp->prev;

	//将指定节点的前向节点指针和后向节点指针都NULL
	tmp->prev = NULL;
	tmp->next = NULL;

	//更新链表长度
	head->data.len--;

	return tmp;
}

//遍历链表，具体的数据操作由函数指针传递
void travel_dlist(linklist head, void (*fun)(void *))
{
	linklist tmp = head;
	int m = head->data.len;
	for(int i=0; i<m; i++)
	{
		tmp = tmp->next;
		fun((void *)tmp->data.mydata);
	}
}

//销毁链表内存
void destroy_list(linklist head)
{
	linklist tmp = head;
	int m = head->data.len;
	for(int i=0; i<m; i++)
	{
		linklist k = tmp->next;
		free(tmp);
		tmp = k;
	}
	free(tmp);
}
