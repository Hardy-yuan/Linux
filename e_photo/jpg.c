#include "photo.h"
#include "jpeglib.h"

static char *lcd_buf;
static int lcd;
static struct fb_var_screeninfo vsinfo;
static int lcd_width;
static int lcd_height;
static int lcd_bpp;

static bool first = true;

//lcd显示屏的初始化
static bool lcd_init(void)
{
	if(first)
	{
		//打开lcd显示设备
		lcd = open("/dev/fb0", O_RDWR);
		if(lcd == -1)
		{
			perror("设备打开失败");
			return false;
		}
		
		// 获取了LCD的硬件参数
		bzero(&vsinfo, sizeof(vsinfo));
		ioctl(lcd, FBIOGET_VSCREENINFO, &vsinfo);
		lcd_width = vsinfo.xres;
		lcd_height = vsinfo.yres;
		lcd_bpp =  vsinfo.bits_per_pixel;

		//lcd屏幕映射内存
		lcd_buf = mmap(NULL, lcd_width*lcd_height*(lcd_bpp/8), PROT_READ | PROT_WRITE, MAP_SHARED, lcd, 0);
		if(lcd_buf == MAP_FAILED)
		{
			perror("映射内存失败");
			return false;
		}
			
		first = false;
	}
	
	return true;
}

//将图片全部转化成缩小之后的rgb数据
bool photo_buf_jpg(struct photo_node *rgb)
{
	if(!lcd_init())
		return false;
	
	// 打开JPG图片并读取图像数据
	int jpg_photo = open(rgb->photo_name, O_RDWR);
	
	printf("rgb->photo_name = %s\n", rgb->photo_name);
	
	//利用文件位置获得jpg文件的大小
	int jpg_size = lseek(jpg_photo, 0L, SEEK_END);
	//重新将文件位置放到文件的开头位置
	lseek(jpg_photo, 0L, SEEK_SET);
	
	//定义块内存存放jpg文件的内容
	char *jpg_buf = calloc(1, jpg_size);
	//完整的将所有的jpg图片中的内容放到内存中去
	char *tmp = jpg_buf;
	int size = jpg_size;
	while(size > 0)//将jpeg图片全部读取
	{
		int nread = read(jpg_photo, tmp, size);
		size -= nread;
		tmp += nread;
	}
	//关闭图片
	close(jpg_photo);
	
	// JPG  ==>  RGB
	// 声明解压缩结构体，以及错误管理结构体
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// 使用缺省的出错处理来初始化解压缩结构体
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// 配置该cinfo，使其从jpg_buf中读取jpg_size个字节
	// 这些数据必须是完整的JPEG数据
	jpeg_mem_src(&cinfo, jpg_buf, jpg_size);


	// 读取JPEG文件的头，并判断其格式是否合法
	int ret = jpeg_read_header(&cinfo, true);
	if(ret != 1)
	{
		fprintf(stderr, "[%d]: jpeg_read_header failed: "
			"%s\n", __LINE__, strerror(errno));
		return false;
	}


	// 开始解码
	jpeg_start_decompress(&cinfo);

	// cinfo中保存了图片文件的尺寸信息,以下的信息是将其解码成bmp图片的大小信息
	int photo_bmp_width = cinfo.output_width; // 宽
	int photo_bmp_height = cinfo.output_height; // 高
	int photo_bmp_bpp = cinfo.output_components; // 深：每个像素点包含的字节数，并不在位数
	printf("photo_bmp_width = %d, photo_bmp_height = %d, photo_bmp_bpp = %d\n", photo_bmp_width, photo_bmp_height, photo_bmp_bpp);
	//将图片的色深放到结构体中
	rgb->rgb_bpp = photo_bmp_bpp;

	// 图片的每一行所包含的字节数
	int row_stride = photo_bmp_width * photo_bmp_bpp;
	// 显存一行的总字节数
	int lcd_line_size = lcd_width * lcd_bpp/8 ; 

	// 根据图片的尺寸大小，分配一块相应的内存photo_bmp_buf
	// 用来存放从jpg_buf解压出来的图像数据
	unsigned char *photo_bmp_buf = calloc(1, photo_bmp_width*photo_bmp_height*photo_bmp_bpp);

	// 循环地将图片的每一行读出并解压到photo_bmp_buf中
	int line = 0;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = photo_bmp_buf +
				(cinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	// 解压完了，将jpeg相关的资源释放掉
 	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(jpg_buf);
	
/************************************/

	//定义两个变量分别是图片中宽的缩小倍数，和图片高的缩小倍数
	int multiple_width = 1, multiple_height = 1;
	multiple_width = (photo_bmp_width%lcd_width == 0) ? photo_bmp_width/lcd_width : (photo_bmp_width/lcd_width)+1;
	multiple_height = (photo_bmp_height%lcd_height == 0) ? photo_bmp_height/lcd_height : (photo_bmp_height/lcd_height)+1;
	//图片缩小的倍数
	int multiple = (multiple_width > multiple_height) ? multiple_width : multiple_height;
	printf("multiple = %d\n", multiple);
	
	//用来存放缩小的图片的宽度
	int rgb_mu_width = photo_bmp_width / multiple;
	//将缩小之后的图片的宽放到结构体中
	rgb->rgb_width = rgb_mu_width;
	//用来存放缩小的图片的高度
	int rgb_mu_height = photo_bmp_height / multiple;
	//将缩小之后的图片的高放到结构体中
	rgb->rgb_height = rgb_mu_height;
	printf("rgb_mu_width = %d, rgb_mu_height = %d\n", rgb_mu_width, rgb_mu_height);
	//用来存放缩小图片的rgb的值rgb24_mu_buf
	char *rgb24_mu_buf = calloc(1, rgb_mu_width*rgb_mu_height*photo_bmp_bpp);
	//将存放rgb的数据的内存放到结构体中
	rgb->rgb_buf = rgb24_mu_buf;
	if(rgb24_mu_buf == NULL)
		return false;

	//两个临时变量,一个指向的时未缩放数据的开始,一个指向的是存放缩放之后数据的开始
	char *p1 = photo_bmp_buf;
	char *p2 = rgb24_mu_buf;
	for(int i=0; i<rgb_mu_height; i++)
	{
		for(int j=0; j<rgb_mu_width; j++)
		{
			memcpy(p2+j*3, p1+j*3*multiple, 3);
		}
		p1 += row_stride*multiple;
		p2 += rgb_mu_width*photo_bmp_bpp;
	}
	
	free(photo_bmp_buf);
}

//将rgb数据显示在显存上
void display_rgb_jpg(struct photo_node *rgb)
{
	//将屏幕刷成黑色
	bzero(lcd_buf, lcd_width*lcd_height*(lcd_bpp/8));
	
	// 恭喜！现在photo_bmp_buf中就已经有图片对应的RGB数据了
	int red_offset  = vsinfo.red.offset;
	int green_offset= vsinfo.green.offset;
	int blue_offset = vsinfo.blue.offset;
	
	// 让 rgb24_mu_p 指向缩小之后图片的内存中
	char *rgb24_mu_p = rgb->rgb_buf;

	// 让显存指针指向指定的显示位置（x,y）
	int x = (lcd_width - rgb->rgb_width)/2;
	int y = (lcd_height - rgb->rgb_height)/2;
	char *lcd_p = lcd_buf + (y*lcd_width+x) * lcd_bpp/8;
	
	//选择图片和相片的最小值
	int width = rgb->rgb_width;
	int height = rgb->rgb_height;

	// 将缩小之后的rgb数据妥善地放入lcd_buf
	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			memcpy(lcd_p + 4*i + red_offset/8,   rgb24_mu_p + 3*i + 0, 1);
			memcpy(lcd_p + 4*i + green_offset/8, rgb24_mu_p + 3*i + 1, 1);
			memcpy(lcd_p + 4*i + blue_offset/8,  rgb24_mu_p + 3*i + 2, 1);
		}

		lcd_p += (lcd_width*lcd_bpp/8); // lcd显存指针向下偏移一行
		rgb24_mu_p += (rgb->rgb_width*rgb->rgb_bpp); // rgb指针向下偏移一行
	}
	
	printf("图片显示成功。\n");
}

/*
bool display(char *photo)
{
	if(!lcd_init())
		return false;
	
	//将屏幕刷成黑色
	bzero(lcd_buf, lcd_width*lcd_height*(lcd_bpp/8));
	
	// 打开JPG图片并读取图像数据
	int jpg_photo = open(photo, O_RDWR);
	
	//利用文件位置获得jpg文件的大小
	int jpg_size = lseek(jpg_photo, 0L, SEEK_END);
	//重新将文件位置放到文件的开头位置
	lseek(jpg_photo, 0L, SEEK_SET);
	
	//定义块内存存放jpg文件的内容
	char *jpg_buf = calloc(1, jpg_size);
	//完整的将所有的jpg图片中的内容放到内存中去
	char *tmp = jpg_buf;
	int size = jpg_size;
	while(size > 0)//将jpeg图片全部读取
	{
		int nread = read(jpg_photo, tmp, size);
		size -= nread;
		tmp += nread;
	}
	//关闭图片
	close(jpg_photo);
	
	// JPG  ==>  RGB
	// 声明解压缩结构体，以及错误管理结构体
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// 使用缺省的出错处理来初始化解压缩结构体
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// 配置该cinfo，使其从jpg_buf中读取jpg_size个字节
	// 这些数据必须是完整的JPEG数据
	jpeg_mem_src(&cinfo, jpg_buf, jpg_size);


	// 读取JPEG文件的头，并判断其格式是否合法
	int ret = jpeg_read_header(&cinfo, true);
	if(ret != 1)
	{
		fprintf(stderr, "[%d]: jpeg_read_header failed: "
			"%s\n", __LINE__, strerror(errno));
		return false;
	}


	// 开始解码
	jpeg_start_decompress(&cinfo);

	// cinfo中保存了图片文件的尺寸信息,以下的信息是将其解码成bmp图片的大小信息
	int photo_bmp_width = cinfo.output_width; // 宽
	int photo_bmp_height = cinfo.output_height; // 高
	int photo_bmp_bpp = cinfo.output_components; // 深：每个像素点包含的字节数，并不在位数

	// 图片的每一行所包含的字节数
	int row_stride = photo_bmp_width * photo_bmp_bpp;
	// 显存一行的总字节数
	int lcd_line_size = lcd_width * lcd_bpp/8 ; 

	// 根据图片的尺寸大小，分配一块相应的内存photo_bmp_buf
	// 用来存放从jpg_buf解压出来的图像数据
	unsigned char *photo_bmp_buf = calloc(1, photo_bmp_width*photo_bmp_height*photo_bmp_bpp);

	// 循环地将图片的每一行读出并解压到photo_bmp_buf中
	int line = 0;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = photo_bmp_buf +
				(cinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	// 解压完了，将jpeg相关的资源释放掉
 	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(jpg_buf);
	
	
	// 恭喜！现在photo_bmp_buf中就已经有图片对应的RGB数据了
	int red_offset  = vsinfo.red.offset;
	int green_offset= vsinfo.green.offset;
	int blue_offset = vsinfo.blue.offset;

	//定义两个变量分别是图片中宽的缩小倍数，和图片高的缩小倍数
	int multiple_width = 1, multiple_height = 1;
	multiple_width = (photo_bmp_width%lcd_width == 0) ? photo_bmp_width/lcd_width : (photo_bmp_width/lcd_width)+1;
	multiple_height = (photo_bmp_height%lcd_height == 0) ? photo_bmp_height/lcd_height : (photo_bmp_height/lcd_height)+1;
	//图片缩小的倍数
	int multiple = (multiple_width > multiple_height) ? multiple_width : multiple_height;
	printf("multiple = %d\n", multiple);
	
	//用来存放缩小的图片的宽度
	int rgb_mu_width = photo_bmp_width / multiple;
	//用来存放缩小的图片的高度
	int rgb_mu_height = photo_bmp_height / multiple;
	printf("rgb_mu_width = %d, rgb_mu_height = %d\n", rgb_mu_width, rgb_mu_height);
	//用来存放缩小图片的rgb的值rgb24_mu_buf
	char *rgb24_mu_buf = calloc(1, rgb_mu_width*rgb_mu_height*photo_bmp_bpp);
	if(rgb24_mu_buf == NULL)
		return false;

	//两个临时变量,一个指向的时未缩放数据的开始,一个指向的是存放缩放之后数据的开始
	char *p1 = photo_bmp_buf;
	char *p2 = rgb24_mu_buf;
	for(int i=0; i<rgb_mu_height; i++)
	{
		for(int j=0; j<rgb_mu_width; j++)
		{
			memcpy(p2+j*3, p1+j*3*multiple, 3);
		}
		p1 += row_stride*multiple;
		p2 += rgb_mu_width*photo_bmp_bpp;
	}
	
	free(photo_bmp_buf);
	
	// 让 rgb24_mu_p 指向缩小之后图片的内存中
	char *rgb24_mu_p = rgb24_mu_buf;

	// 让显存指针指向指定的显示位置（x,y）
	int x = (lcd_width - rgb_mu_width)/2;
	int y = (lcd_height - rgb_mu_height)/2;
	char *lcd_p = lcd_buf + (y*lcd_width+x) * lcd_bpp/8;
	
	//选择图片和相片的最小值
	int width = rgb_mu_width;
	int height = rgb_mu_height;

	// 将缩小之后的rgb数据妥善地放入lcd_buf
	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			memcpy(lcd_p + 4*i + red_offset/8,   rgb24_mu_p + 3*i + 0, 1);
			memcpy(lcd_p + 4*i + green_offset/8, rgb24_mu_p + 3*i + 1, 1);
			memcpy(lcd_p + 4*i + blue_offset/8,  rgb24_mu_p + 3*i + 2, 1);
		}

		lcd_p += (lcd_width*lcd_bpp/8); // lcd显存指针向下偏移一行
		rgb24_mu_p += (rgb_mu_width*photo_bmp_bpp); // rgb指针向下偏移一行
	}

	// 释放相应的资源
	free(rgb24_mu_buf);
	
	return true;
}
*/

//释放相应的资源
void lcd_free_jpg(void)
{
	if(!first)
	{
		munmap(lcd_buf, lcd_width*lcd_height*(lcd_bpp/8));
		close(lcd);
	}
	
	first = true;
}