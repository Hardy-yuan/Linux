#include "ts.h"
#include "dlink_list.h"

static int x=0, y=0;
//xm1，ym1表示按下的坐标，xm2，ym2表示松开之后的坐标
static int xm1=0, ym1=0, xm2=0, ym2=0;

static inline void clear_flag()
{
	xm1 = 0;
	ym1 = 0;
	xm2 = 0;
	ym2 = 0;
}

int get_motion(int tp)
{
	struct input_event buf;

	while(1)
	{
		//小于0，说明没有按下
		//1.第一次按下
		//2.已经按下了一次
		while(read(tp, &buf, sizeof(buf)) < 0)//表示无操作
		{

			if((ym2 - ym1 > 50) && abs(xm2 - xm1) < 50)
			{
				clear_flag();
				return DOWN;//下移
			}
			if((ym1 - ym2 > 50) && abs(xm2 - xm1) < 50)
			{
				clear_flag();
				return UP;//上移
			}
			if((xm1 - xm2 > 50) && abs(ym2 - ym1) < 50)
			{
				clear_flag();
				return LEFT;//左移
			}
			if((xm2 - xm1 > 50) && abs(ym2 - ym1) < 50)
			{
				clear_flag();
				return RIGHT;//右移
			}
			
		}

		//绝对坐标事件
		if(buf.type == EV_ABS)
		{
			//触摸屏的x轴事件
			if(buf.code == ABS_X)
				x = buf.value;
			//触摸屏的x轴事件
			if(buf.code == ABS_Y)
				y = buf.value;
		}

		//表示触摸屏按下的一瞬间
		if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 1)
		{
			xm1 = x;
			ym1 = y;
			printf("xm1 = %d, ym1 = %d\n", xm1, ym1);
			
			xm2 = 0;
			ym2 = 0;
		}

		//按键触发事件，触摸屏按压事件，压力值为0，表示松开的一瞬间 
		if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
		{
			xm2 = x;
			ym2 = y;
			printf("xm2 = %d, ym2 = %d\n", xm2, ym2);
		}
	}
}


int tp_init()
{
	int tp = open("/dev/input/event0", O_RDWR);

	// 获取文件当前的状态标签
	long flag = fcntl(tp, F_GETFL);

	// 将非阻塞标签加入到当前状态标签中
	flag |= O_NONBLOCK;

	// 重新设置文件的状态标签（此时含有非阻塞标签）
	fcntl(tp, F_SETFL, flag);
	
	return tp;
}

