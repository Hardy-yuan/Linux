#ifndef __PHOTO_H
#define __PHOTO_H

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
#include <pthread.h>
#include <math.h>

// 触摸屏
#include <linux/input.h>

// 液晶屏
#include <sys/mman.h>

struct bitmap_header
{
	int16_t type;
	int32_t size; // 整个文件大小
	int16_t reserved1;
	int16_t reserved2;
	int32_t offbits; // bmp图像数据偏移量
}__attribute__((packed));

struct bitmap_info
{
	int32_t size; // 本结构大小	
	int32_t width; // 单位：像素
	int32_t height;// 单位：像素
	int16_t planes; // 总为零

	int16_t bit_count; // 色深:24（1像素=24位=3字节）
	int32_t compression;
	int32_t size_img; // bmp数据大小，必须是4的整数倍
	int32_t X_pel;
	int32_t Y_pel;
	int32_t clrused;
	int32_t clrImportant;
}__attribute__((packed));

struct rgb_quad
{
	int8_t blue;
	int8_t green;
	int8_t red;
	int8_t reserved;
}__attribute__((packed));


struct image_info
{
	int width;
	int height;
	int pixel_size;
};

/*****************************/
struct photo_node
{
	//图片的名字
	char *photo_name;
	//图片转换成rgb格式的缓存
	char *rgb_buf;
	//rgb数据的宽度(像素点的个数)
	int rgb_width;
	//rgb数据的高度(像素点的个数)
	int rgb_height;
	//rgb数据的色深(字节数)
	int rgb_bpp;
};

//将图片全部转化成缩小之后的rgb数据
bool photo_buf_jpg(struct photo_node *rgb);

//将rgb数据显示在显存上
void display_rgb_jpg(struct photo_node *rgb);

//释放相应的资源
void lcd_free_jpg(void);

//将图片全部转化成缩小之后的rgb数据
bool photo_buf_bmp(struct photo_node *rgb);

//将rgb数据显示在显存上
void display_rgb_bmp(struct photo_node *rgb);

//释放相应的资源
void lcd_free_bmp(void);
#endif