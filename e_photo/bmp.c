#include "photo.h"

static int lcd;
static char *lcd_buf;
//存储硬件设备的信息
static struct fb_var_screeninfo vsinfo;
static int lcd_width;//屏幕的宽
static int lcd_height;//屏幕的高
static int lcd_bpp;//屏幕的色深，一个像素点占用的位数
static bool first = true;

//lcd的初始化
static bool lcd_init(void)
{
	if(first)
	{
		//打开显示设备
		lcd = open("/dev/fb0", O_RDWR);
		if(lcd == -1)
		{
			perror("设备打开失败");
			return false;
		}
		
		//获取设备的硬件信息
		bzero(&vsinfo, sizeof(vsinfo));
		ioctl(lcd, FBIOGET_VSCREENINFO, &vsinfo);
		lcd_width = vsinfo.xres;//屏幕的宽
		lcd_height = vsinfo.yres;//屏幕的高
		lcd_bpp = vsinfo.bits_per_pixel;//色深
			
		//映射内存
		lcd_buf = mmap(NULL, lcd_width*lcd_height*(lcd_bpp/8), PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);
		if(lcd_buf == MAP_FAILED)
		{
			perror("映射内存失败");
			return false;
		}
		
		//标志位提示该初始化仅仅只是初始化一遍
		first = false;
	}
	return true;
}

bool photo_buf_bmp(struct photo_node *rgb)
{
	//初始化成功返回一个true
	if(!lcd_init())
		return false;
	
	//打开图片
	int bmp_photo = open(rgb->photo_name, O_RDWR);
	printf("rgb->photo_name = %s\n", rgb->photo_name);
	if(bmp_photo == -1)
	{
		perror("图片打开失败");
		return false;
	}
	
	//定义两个系统结构体，存储图片的各种信息
	struct bitmap_header header;
	struct bitmap_info info;
	
	//初始化两个结构体，均初始化成0
	bzero(&header, sizeof(header));
	bzero(&info, sizeof(info));
	read(bmp_photo, &header, sizeof(header));//该结构体中含有整个文件的大小
	read(bmp_photo, &info, sizeof(info));//该结构体中含有图片的长和宽
	//从结构体中分析出图片给的宽，高，色深
	int photo_width = info.width;
	int photo_height = info.height;
	int photo_bpp = info.bit_count;//色深是一个像素占用的字节数
	//将该图片的色深保存起来
	rgb->rgb_bpp = photo_bpp;
	printf("photo_width = %d, photo_height = %d, photo_bpp = %d\n", photo_width, photo_height, photo_bpp);
	
	//将剩余文件的RGB数据全部读取
	int rgb_size = header.size - sizeof(header) - sizeof(info);
	//定义一片内存，存储rgb数据
	char *rgb24_buf = calloc(1, rgb_size);
	if(rgb24_buf == NULL)
		return false;
	//循环读取图片数据，直到全部读取完成
	char *tmp = rgb24_buf;
	int size = rgb_size;
	while(size > 0)//将rgb数全部读取
	{
		int nread = read(bmp_photo, tmp, size);
		size -= nread;
		tmp += nread;
	}
	close(bmp_photo);
	
	//图片后端补的无效位的个数
	int space = (4 - ((photo_width * 3) % 4)) % 4;
	
	int photo_line_size = photo_width*3 + space; // 图片一行的总字节数

	//定义两个变量分别是图片中宽的缩小倍数，和图片高的缩小倍数
	int multiple_width = 1, multiple_height = 1;
	multiple_width = (photo_width%lcd_width == 0) ? photo_width/lcd_width : (photo_width/lcd_width)+1;
	multiple_height = (photo_height%lcd_height == 0) ? photo_height/lcd_height : (photo_height/lcd_height)+1;
	//图片缩小的倍数
	int multiple = (multiple_width > multiple_height) ? multiple_width : multiple_height;
	printf("multiple = %d\n", multiple);
	
	//用来存放缩小的图片的宽度
	int rgb_mu_width = photo_width / multiple;
	//将缩小之后的图片的宽保存到结构体中
	rgb->rgb_width = rgb_mu_width;
	//用来存放缩小的图片的高度
	int rgb_mu_height = photo_height / multiple;
	//将缩小之后的图片的高保存到结构体中
	rgb->rgb_height = rgb_mu_height;
	printf("rgb_mu_width = %d, rgb_mu_height = %d\n", rgb_mu_width, rgb_mu_height);
	char *rgb24_mu_buf = calloc(1, rgb_mu_width*rgb_mu_height*photo_bpp/8);
	//将缩小图片的rgb数据的内存指针保存到结构体中
	rgb->rgb_buf = rgb24_mu_buf;
	if(rgb24_mu_buf == NULL)
		return false;

	//两个临时变量,一个指向的时未缩放数据的开始,一个指向的是存放缩放之后数据的开始
	char *p1 = rgb24_buf;
	char *p2 = rgb24_mu_buf;
	
	for(int i=0; i<rgb_mu_height; i++)
	{
		for(int j=0; j<rgb_mu_width; j++)
		{
			memcpy(p2+j*3, p1+j*3*multiple, 3);
		}
		p1 += photo_line_size*multiple;
		p2 += rgb_mu_width*photo_bpp/8;
	}
	
	free(rgb24_buf);
}

void display_rgb_bmp(struct photo_node *rgb)
{
	//将屏幕刷成黑色
	bzero(lcd_buf, lcd_width*lcd_height*(lcd_bpp/8));
	
	// 让 rgb24_p 指向最后一行，为了将图片上下颠倒一下
	char *rgb24_mu_p = rgb->rgb_buf + rgb->rgb_width*(rgb->rgb_height-1)*(rgb->rgb_bpp/8);

	// 让显存指针指向指定的显示位置（x,y）
	int x = (lcd_width - rgb->rgb_width)/2;
	int y = (lcd_height - rgb->rgb_height)/2;
	char *lcd_p = lcd_buf + (y*lcd_width+x) * lcd_bpp/8;
	
	//选择图片和相片的最小值
	int width = rgb->rgb_width;
	int height = rgb->rgb_height;

	// 5，妥善地将BMP中的RGB数据搬到映射内存上
	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			memcpy(lcd_p+4*i, rgb24_mu_p+3*i, 3);
		}
		lcd_p += (lcd_width * lcd_bpp/8);
		rgb24_mu_p -= (rgb->rgb_width*rgb->rgb_bpp/8);
	}
	printf("图片显示成功。\n");
}

/*
//到图片的分辨率和屏幕不对时将图片全部放大或缩小
bool read_photo_shrink(char *photo)
{
	//初始化成功返回一个true
	if(!lcd_init())
		return false;
	
	//将屏幕刷成黑色
	bzero(lcd_buf, lcd_width*lcd_height*(lcd_bpp/8));
	
	//打开图片
	int bmp_photo = open(photo, O_RDWR);
	if(bmp_photo == -1)
	{
		perror("图片打开失败");
		return false;
	}
	
	//定义两个系统结构体，存储图片的各种信息
	struct bitmap_header header;
	struct bitmap_info info;
	
	//初始化两个结构体，均初始化成0
	bzero(&header, sizeof(header));
	bzero(&info, sizeof(info));
	read(bmp_photo, &header, sizeof(header));//该结构体中含有整个文件的大小
	read(bmp_photo, &info, sizeof(info));//该结构体中含有图片的长和宽
	//从结构体中分析出图片给的宽，高，色深
	int photo_width = info.width;
	int photo_height = info.height;
	int photo_bpp = info.bit_count;//色深是一个像素占用的字节数
	printf("photo_width = %d, photo_height = %d, photo_bpp = %d\n", photo_width, photo_height, photo_bpp);
	
	//将剩余文件的RGB数据全部读取
	int rgb_size = header.size - sizeof(header) - sizeof(info);
	//定义一片内存，存储rgb数据
	char *rgb24_buf = calloc(1, rgb_size);
	if(rgb24_buf == NULL)
		return false;
	//循环读取图片数据，直到全部读取完成
	char *tmp = rgb24_buf;
	int size = rgb_size;
	while(size > 0)//将rgb数全部读取
	{
		int nread = read(bmp_photo, tmp, size);
		size -= nread;
		tmp += nread;
	}
	
	//图片后端补的无效位的个数
	int space = (4 - ((photo_width * 3) % 4)) % 4;

	int photo_line_size = photo_width*3 + space; // 图片一行的总字节数
	int lcd_line_size = lcd_width * lcd_bpp/8 ; // 显存一行的总字节数

	//定义两个变量分别是图片中宽的缩小倍数，和图片高的缩小倍数
	int multiple_width = 1, multiple_height = 1;
	multiple_width = (photo_width%lcd_width == 0) ? photo_width/lcd_width : (photo_width/lcd_width)+1;
	multiple_height = (photo_height%lcd_height == 0) ? photo_height/lcd_height : (photo_height/lcd_height)+1;
	//图片缩小的倍数
	int multiple = (multiple_width > multiple_height) ? multiple_width : multiple_height;
	printf("multiple = %d\n", multiple);
	
	//用来存放缩小的图片的宽度
	int rgb_mu_width = photo_width / multiple;
	//用来存放缩小的图片的高度
	int rgb_mu_height = photo_height / multiple;
	printf("rgb_mu_width = %d, rgb_mu_height = %d\n", rgb_mu_width, rgb_mu_height);
	char *rgb24_mu_buf = calloc(1, rgb_mu_width*rgb_mu_height*photo_bpp/8);
	if(rgb24_mu_buf == NULL)
		return false;

	//两个临时变量,一个指向的时未缩放数据的开始,一个指向的是存放缩放之后数据的开始
	char *p1 = rgb24_buf;
	char *p2 = rgb24_mu_buf;
	for(int i=0; i<rgb_mu_height; i++)
	{
		for(int j=0; j<rgb_mu_width; j++)
		{
			memcpy(p2+j*3, p1+j*3*multiple, 3);
		}
		p1 += photo_line_size*multiple;
		p2 += rgb_mu_width*photo_bpp/8;
	}
	
	free(rgb24_buf);
	// 让 rgb24_p 指向最后一行，为了将图片上下颠倒一下
	char *rgb24_mu_p = rgb24_mu_buf + rgb_mu_width*(rgb_mu_height-1)*(photo_bpp/8);

	// 让显存指针指向指定的显示位置（x,y）
	int x = (lcd_width - rgb_mu_width)/2;
	int y = (lcd_height - rgb_mu_height)/2;
	char *lcd_p = lcd_buf + (y*lcd_width+x) * lcd_bpp/8;
	
	//选择图片和相片的最小值
	int width = rgb_mu_width;
	int height = rgb_mu_height;

	// 5，妥善地将BMP中的RGB数据搬到映射内存上
	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			memcpy(lcd_p+4*i, rgb24_mu_p+3*i, 3);
		}
		lcd_p += lcd_line_size;
		rgb24_mu_p -= rgb_mu_width*photo_bpp/8;
	}
	//释放相应资源
	free(rgb24_mu_buf);
	close(bmp_photo);
	
	return true;
}
*/

void lcd_free_bmp(void)
{
	if(!first)
	{
		munmap(lcd_buf, lcd_width * lcd_height * lcd_bpp/8);
		close(lcd);
	}
	
	first = true;
}