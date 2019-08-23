#include <stdio.h>
#include <string.h>

#include "dir_list.h"
#include "ts.h"
#include "photo.h"

int main()
{
	//触摸屏初始化
	int tp = tp_init();
	//触摸屏初始化

	//链表初始化
	linklist photo_dlist = dir_list_init("./");
	linklist tmp = photo_dlist->next;
	
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
			case UP:     printf("上移\n");break;
			case DOWN:   printf("下移\n");break;
			default : break;
		}
		//printf("*");
	}
	return 0;
}