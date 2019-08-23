#include "dir_list.h"

//链表节点的遍历的回调函数
void show(void *mydata)
{
	printf("%s\n", ((struct photo_node *)mydata)->photo_name);
}

//链表和目录文件的初始化，将目录中的文件的名字保存在链表中
//成功是返回的是保存了名字的链表
linklist dir_list_init(char *target)
{
	//定义一个结构体，存储文件属性
	struct stat info;
	//将target文件或目录的信息放到结构体中
	stat(target, &info);

	//创建一个空的双向循环链表，空的链表返回一个头节点,不存在有效数据
	linklist photo_dlist = init_list();

	if(S_ISDIR(info.st_mode))//如果是一个目录
	{
		//打开目录，获得一个指向目录的指针
		DIR *dirp = opendir(target);
		//进入该目录
		chdir(target);
		//定义一个目录项指针
		struct dirent *ep;

		//循环不断的读取目录项
		while(1)
		{
			ep = readdir(dirp);
			if(ep == NULL)
			{
				break;
			}
			if(ep->d_name[0] == '.')
				continue;

			//将ep->d_name文件或目录的信息放到结构体中
			//判断该目录下
			stat(ep->d_name, &info);
			if(S_ISDIR(info.st_mode))//如果是一个目录
				continue;
				
			if(strstr(ep->d_name, ".bmp") || strstr(ep->d_name, ".jpg"))
				add_name(ep->d_name, photo_dlist);//将节点加入到链表中去

		}

		
		//遍历链表，具体的数据操作由函数指针传递
		travel_dlist(photo_dlist, show);
		

		//将保存了文件名字的链表返回回去
		return photo_dlist;
	}
	else
	{
		printf("[%s]不是目录。\n", target);
		return NULL;
	}
}

//将目录下面的文件名name，放到链表photo_dlist中去
void add_name(char *name, linklist photo_dlist)
{
	//产生一个新的节点
	linklist new_node = produce_new_node(name);
	//将一个节点数据放到链表的结尾
	list_add_tail(new_node, photo_dlist);
}

//产生一个有效数据时n的链表节点，成功时返回指向节点的指针
linklist produce_new_node(char *n)
{
	//产生一个新的节点
	linklist new_node = calloc(1, sizeof(listnode));
	new_node->data.mydata = calloc(1, sizeof(struct photo_node));
	new_node->data.mydata->photo_name = calloc(1, 20);
	if(new_node!=NULL && new_node->data.mydata!=NULL && new_node->data.mydata->photo_name!=NULL)
	{
		new_node->prev = NULL;
		new_node->next = NULL;
		memcpy(new_node->data.mydata->photo_name, n, 20);
		new_node->data.mydata->rgb_buf = NULL;
		new_node->data.mydata->rgb_width = 0;
		new_node->data.mydata->rgb_height = 0;
		new_node->data.mydata->rgb_bpp = 0;

		return new_node;
	}
	return NULL;
}
