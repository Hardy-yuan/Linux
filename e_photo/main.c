#include <stdio.h>
#include <string.h>

#include "dir_list.h"
#include "ts.h"
#include "photo.h"

void display(linklist temp);
void cache_phtot(linklist temp);
void dis_photo(linklist temp);
void cache(linklist temp);
void dis(linklist temp);

linklist photo_dlist, tmp;

int main()
{
	//触摸屏初始化
	int tp = tp_init();

	//链表初始化,头节点
	photo_dlist = dir_list_init("./");
	tmp = photo_dlist->next;
	cache(tmp);
	dis(tmp);

	while(1)
	{
		switch(get_motion(tp))
		{
			case LEFT:  //左移
				printf("左移\n");
				tmp = tmp->prev;
				if(tmp == photo_dlist)
					tmp = tmp->prev;
				break;
			case RIGHT:  //右移
				printf("右移\n");
				tmp = tmp->next;
				if(tmp == photo_dlist)
					tmp = tmp->next;
				break;
			case UP:     printf("上移\n"); break;
			case DOWN:   printf("下移\n"); break;
			default : break;
		}
		dis(tmp);
		cache(tmp);
	}
	return 0;
}

void cache(linklist temp)
{
	printf("temp1 = %s\n", temp->data.mydata->photo_name);
	linklist k;
	//缓存
	if(temp->data.mydata->rgb_buf == NULL)
	{
		cache_phtot(temp);
		printf("1.缓存%s\n", temp->data.mydata->photo_name);
	}
	//判断temp的上一个节点是否是头节点
	if(temp->prev == photo_dlist)
		k = temp->prev;
	else 
		k = temp;
	//将当前节点的下一个节点缓存
	if(k->prev->data.mydata->rgb_buf == NULL)
	{
			cache_phtot(k->prev);
			printf("2.缓存%s\n", k->prev->data.mydata->photo_name);
	}
	//判断temp的下一个节点是否为头节点	
	if(temp->next == photo_dlist)
		k = temp->next;
	else 
		k = temp;
	//将当前节点的上一个节点缓存
	if(k->next->data.mydata->rgb_buf == NULL)
	{
		cache_phtot(k->next);
		printf("3.缓存%s\n", k->next->data.mydata->photo_name);
	}
}

void dis(linklist temp)
{
	linklist k;
	printf("temp2 = %s\n", temp->data.mydata->photo_name);
	
	dis_photo(temp);
	printf("4.显示%s\n", temp->data.mydata->photo_name);
	
	//判断当前节点的前面的第二个节点是否是头节点
	if(temp->prev->prev == photo_dlist || temp->prev == photo_dlist)
		k = temp->prev;
	else
		k = temp;
	//将当前节点的前面第二个节点释放
	if(k->prev->prev->data.mydata->rgb_buf != NULL)
	{
		free(k->prev->prev->data.mydata->rgb_buf);
		k->prev->prev->data.mydata->rgb_buf = NULL;
		printf("5.释放%s\n", k->prev->prev->data.mydata->photo_name);
	}
	
	if(temp->next->next == photo_dlist || temp->next == photo_dlist)
		k = temp->next;
	else
		k = temp;
	if(k->next->next->data.mydata->rgb_buf != NULL)
	{
		free(k->next->next->data.mydata->rgb_buf);
		k->next->next->data.mydata->rgb_buf = NULL;
		printf("6.释放%s\n", k->next->next->data.mydata->photo_name);
	}
}

//自动判断一张图片格式并缓存它
void cache_phtot(linklist temp)
{
	if(strstr(temp->data.mydata->photo_name, ".bmp"))
		photo_buf_bmp(temp->data.mydata);
	
	if(strstr(temp->data.mydata->photo_name, ".jpg"))
		photo_buf_jpg(temp->data.mydata);
}

//判断当前图片格式，并显示它
void dis_photo(linklist temp)
{
	if(strstr(temp->data.mydata->photo_name, ".bmp"))
		display_rgb_bmp(temp->data.mydata);
	if(strstr(temp->data.mydata->photo_name, ".jpg"))
		display_rgb_jpg(temp->data.mydata);
}
