#ifndef IMAGE_ALGORITHM
#define IMAGE_ALGORITHM
#include"stdio.h"
#include <cmath>
#include <stdint.h>
#include <string.h>
#include <opencv2/opencv.hpp>
//using u8 = unsigned char;
typedef   uint32_t   u32;   ///32位
typedef   uint16_t   u16;   ///16位
typedef   uint8_t     u8;   ///8位

static long int crv_tab[256] = { 0 };
static long int cbu_tab[256] = { 0 };
static long int cgu_tab[256] = { 0 };
static long int cgv_tab[256] = { 0 };
static long int tab_76309[256] = { 0 };
static unsigned char clp[1024] = { 0 };  //for clip in CCIR601


void init_yuv420p_table()
{
	long int crv, cbu, cgu, cgv;
	int i, ind;
	static int init = 0;

	if (init == 1) return;

	crv = 104597;
	cbu = 132201;
	cgu = 25675;
	cgv = 53279;

	for (i = 0; i < 256; i++)
	{
		crv_tab[i] = ((i - 128) * crv);
		cbu_tab[i] = ((i - 128) * cbu);
		cgu_tab[i] = ((i - 128) * cgu);
		cgv_tab[i] = ((i - 128) * cgv);
		tab_76309[i] = ((76309 * (i - 16)));
	}

	for (i = 0; i < 384; i++)
	{
		clp[i] = 0;
	}
	ind = 384;
	for (i = 0; i < 256; i++)
	{
		clp[ind++] = i;
	}
	ind = 640;
	for (i = 0; i < 384; i++)
	{
		clp[ind++] = 255;
	}

	init = 1;
}

/* yuv420sp to rgb24 */
void yuv420sp_to_rgb24(unsigned char* yuvbuffer, int width, int height, unsigned char* rgbbuffer)
{
	// printf("############ harlem yuv420sp process start \n");
	int y1, y2, u, v;
	unsigned char* py1, * py2;
	int i, j, c1, c2, c3, c4;
	unsigned char* d1, * d2;
	unsigned char* src_u;
	static int init_yuv420p = 0;

	src_u = yuvbuffer + width * height;   // u

	py1 = yuvbuffer;   // y
	py2 = py1 + width;
	d1 = rgbbuffer;
	d2 = d1 + 3 * width;
	// printf("############ harlem yuv420sp init table start \n");
	init_yuv420p_table();
	// printf("############ harlem yuv420sp init table end \n");

	for (j = 0; j < height; j += 2)
	{
		for (i = 0; i < width; i += 2)
		{
			// NV12
			u = *src_u++;
			v = *src_u++;      // v紧跟u，在u的下一个位置

			// NV21
			// v = *src_u++;   
			// u = *src_u++;      // u紧跟v，在v的下一个位置

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left   
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-left   
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];

			//up-right   
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-right   
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
		}
		d1 += 3 * width;
		d2 += 3 * width;
		py1 += width;
		py2 += width;
	}
	// printf("############ harlem yuv420sp process end \n");
}

int crop(u8* srcData, int src_w, int src_h, int c, int dst_x, \
	int dst_y, int dst_w, int dst_h, u8* dstData)
{
	int dstIdx = 0;
	for (int i = dst_y; i < dst_y + dst_h; i++)
	{
		for (int j = dst_x; j < dst_x + dst_w; j++)
		{
			for (int k = 0; k < c; k++)
			{
				int srcIdx = (i * src_w + j) * c + k;
				dstData[dstIdx++] = srcData[srcIdx];
			}
		}
	}
	return 0;
}

/**
 * @brief                   图像处理算法
 *
 * @param buffer            输入的图像帧数据
 * @param width             图像帧宽度
 * @param height            图像帧高度
 * @param ch                图像帧通道数
 * @param pixfmt            图像格式图       [yuv420_nv12:0, yuv420_nv21:1, rgb:2, bgr:3]
 * @param x_len             tv 灯带像素宽度
 * @param y_len             tv 灯带像素高度
 * @param direction         灯带的起点，和算法输出帧的起点一致 0左上角，1右上角，2右下角，3左下角
 * @param level             默认3 为中等灵敏。数组越大越灵敏 共5级 [0,1,2,3,4]
 * @param pixel_frame_buff  一帧图像，算法后对应到灯带的一帧像素，像素总数=灯带像素总数
 * @return int 0     成功，其它错误码 -1 内存申请失败
 * @see
 * @warning
 * @note
*/

// 默认pixel_frame_buff的尺寸为(x_len+y_len)*2*3 存储图像帧周围的led灯带RGB值
int image_analyse_algorithm(u8* buffer, u16 width, u16 height, u8 ch, u8 pixfmt,\
    u8 x_len, u8 y_len, u8 direction, u8 level, u8* pixel_frame_buff) {
	printf("algo  process start !\n");
    //////////////////////////////////////////
    // 接口之外的超参数
    float m_margin = 0.2f;  // 边缘切分的尺寸，沿宽切分表示切分块高度占帧图像高度比例，
    u8 m_level = 6 - level; // 3-->3 4-->2 ...... 0-->6  参数后期再修改
	u16 m_base = 256;         //缩略图长边长度 用于图像缩放
	u8* sub_imags_rgb = (u8*)malloc(sizeof(u8) * 2*(x_len+y_len) * 3);
    // 数据格式转换
    u8* frame = NULL; // 默认RGB
    if (pixfmt == 0) {
        frame = (u8*)malloc(width * height * ch * sizeof(u8));
		if (frame == NULL) {	return -1;		}
        yuv420sp_to_rgb24(buffer, width, height, frame);  //这里有精度损失
		//cv::Mat cv_frame(height, width, CV_8UC3, frame);
		//cv::cvtColor(cv_frame, cv_frame, cv::COLOR_RGB2BGR);
		//cv::imwrite("yuv_1stimg.png", cv_frame);

    }
    else {
        frame = buffer;
    }
    
    // 1. 首先进行图像缩放，将长边缩放到base 得到缩略图
	u16 max_edge = height > width ? height : width;
	double factor = m_base / (max_edge * 1.0); 

	u16 new_height = height * factor;
	u16 new_width = width * factor;
	u8* new_frame = (u8*)malloc(new_width * new_height * ch * sizeof(u8));
	if (new_frame == NULL) {	return -1;	}
	for (int row = 0; row < new_height; row++) {
		for (int col = 0; col < new_width; col++) {
			int row_new = round(row / factor);
			if (row_new > height - 1) {
				row_new = height - 1;
			}
			int col_new = round(col / factor);
			if (col_new > width - 1) {
				col_new = width - 1;
			}
			new_frame[row * new_width * ch + col * ch] = frame[row_new * width * ch + col_new * ch];
			new_frame[row * new_width * ch + col * ch + 1] = frame[row_new * width * ch + col_new * ch + 1];
			new_frame[row * new_width * ch + col * ch + 2] = frame[row_new * width * ch + col_new * ch + 2];
		}
	}
	
	//cv::Mat cv_rescale_img=cv::Mat(new_height,new_width, CV_8UC3, new_frame).clone();
	//cv::cvtColor(cv_rescale_img, cv_rescale_img, cv::COLOR_RGB2BGR);
	//cv::imwrite("cv_rescale_img.png", cv_rescale_img);
	printf("image rescaled success!\n");

	// 2. 图像切分方法  原方法像素数整除LED灯数量，多余的像素两边直接丢弃  1280/16 = 80 则最多丢弃79个像素
	// 现改为  整除 除不尽向上取整 这样最多79个像素公用相邻led颜色相近影响不大
	u16 total_led = 2 * (x_len + y_len);
	u8** sub_images = (u8**)malloc(sizeof(u8*) * total_led); // 记录拆分的led灯
	u16* sub_image_hw = (u16*)malloc(sizeof(u16) * total_led * 2); // 记录灯的宽高 u8就可以
	u16 sub_image_idx = 0;

	u16 w_stride = std::ceil(new_width*1.0f / x_len);
	u16 h_stride = std::ceil(new_height * 1.0f / y_len);
	u16 w_margin = new_width * m_margin;
	u16 h_margin = new_height * m_margin;
	//u16 rm_margin_h = new_height % h_stride;
	//u16 rm_margin_w = new_width % w_stride;

	// up left->right   w_stride*h_margin	
	u16 y0 = 0;
	for (int x = 0; x < new_width; x += w_stride) {
		if (x + w_stride > new_width) {
			x = new_width - w_stride;
		}
		u8* sub_img = (u8*)malloc(w_stride * h_margin * ch * sizeof(u8));
		if (sub_img == NULL) {  return -1;  }
		int res = crop(new_frame, new_width, new_height, ch, x, y0, w_stride, h_margin, sub_img);
		sub_images[sub_image_idx] = sub_img;
		sub_image_hw[sub_image_idx * 2] = h_margin;
		sub_image_hw[sub_image_idx * 2+1] = w_stride;
		sub_image_idx++;
	}

	// right top->bottom  h_stride*w_margin	
	u16 x0 = new_width - w_margin;
	for (int y = 0; y < new_height; y += h_stride) {
		if (y + h_stride > new_height) {
			y = new_height - h_stride;
		}
		u8* sub_img = (u8*)malloc(h_stride * w_margin * ch * sizeof(u8));
		if (sub_img == NULL) { return -1; }
		int res = crop(new_frame, new_width, new_height, ch, x0, y, w_margin, h_stride, sub_img);
		sub_images[sub_image_idx] = sub_img;
		sub_image_hw[sub_image_idx * 2] = h_stride;
		sub_image_hw[sub_image_idx * 2 + 1] = w_margin;
		sub_image_idx++;
	}

	// bottom right->left  w_stride*h_margin	
	y0 = new_height - h_margin;
	for (int x = new_width; x > 0; x -= w_stride) {
		if (x - w_stride < 0) {
			x = 0 + w_stride;
		}
		u8* sub_img = (u8*)malloc(w_stride * h_margin * ch * sizeof(u8));
		if (sub_img == NULL) { return -1; }
		int res = crop(new_frame, new_width, new_height, ch, x - w_stride, y0, w_stride, h_margin, sub_img);
		sub_images[sub_image_idx] = sub_img;
		sub_image_hw[sub_image_idx * 2] = h_margin;
		sub_image_hw[sub_image_idx * 2 + 1] = w_stride;
		sub_image_idx++;
	}

	// left bottom->top  h_stride*w_margin	
	x0 = 0;
	for (int y = new_height; y > 0; y -= h_stride) {
		if (y - h_stride < 0) {
			y = 0 + h_stride;
		}
		u8* sub_img = (u8*)malloc(h_stride * w_margin * ch * sizeof(u8));
		if (sub_img == NULL) { return -1; }
		int res = crop(new_frame, new_width, new_height, ch, x0, y - h_stride, w_margin, h_stride, sub_img);
		sub_images[sub_image_idx] = sub_img;
		sub_image_hw[sub_image_idx * 2] = h_stride;
		sub_image_hw[sub_image_idx * 2 + 1] = w_margin;
		sub_image_idx++;
	}

	//for (int idx = 0; idx < total_led; idx++) {
	//	u8* sub_img = sub_images[idx];
	//	u16 sub_img_height = sub_image_hw[idx * 2];
	//	u16 sub_img_width = sub_image_hw[idx * 2+1];
	//	cv::Mat cv_sub_img = cv::Mat(sub_img_height, sub_img_width, CV_8UC3, sub_img);
	//	cv::String sub_img_name = std::to_string(idx) + ".png";
	//	cv::cvtColor(cv_sub_img, cv_sub_img, cv::COLOR_RGB2BGR);
	//	cv::imwrite(sub_img_name,cv_sub_img);
	//}
	printf("image split success!\n");
	// u8* sub_imags_rgb = (u8*)malloc(sizeof(u8) * total_led * 3);

	// 对每个子图进行处理
	for (int idx = 0; idx < total_led; idx++) {
		u16 matrix_size = m_level * m_level * m_level;
		u8** groups = (u8**)malloc(sizeof(u8*) * matrix_size);
		u32* groups_num = (u32*)malloc(sizeof(u32) * matrix_size);//记录每组的pixel个数
		memset(groups_num, 0, sizeof(u32)* matrix_size);

		u8* sub_img = sub_images[idx];
		u16 sub_height = sub_image_hw[idx * 2];
		u16 sub_width  = sub_image_hw[idx * 2 + 1];

		u8 r_min = 255, r_max = 0, g_min = 255, g_max = 0, b_min = 255, b_max = 0;
		for (int row = 0; row < sub_height; row++) {
			for (int col = 0; col < sub_width; col++) {
				u8 r = sub_img[row * sub_width * ch + col * ch];
				u8 g = sub_img[row * sub_width * ch + col * ch + 1];
				u8 b = sub_img[row * sub_width * ch + col * ch + 2];
				if (r < r_min) {	r_min = r;		}
				if (r > r_max) {	r_max = r;		}
				if (g < g_min) {	g_min = g;		}
				if (g > g_max) {	g_max = g;		}
				if (b < b_min) {	b_min = b;		}
				if (b > b_max) {	b_max = b;		}
			}
		}

		u8 r = 0, g = 0, b = 0;
		u8 r_dvalue = (r_max - r_min) / m_level;
		u8 g_dvalue = (g_max - g_min) / m_level;
		u8 b_dvalue = (b_max - b_min) / m_level;
		if (r_dvalue == 0) {	r_dvalue = m_level + 1; 	}// 当全图大小一致时，将其分到0组
		if (g_dvalue == 0) {	g_dvalue = m_level + 1;		}
		if (b_dvalue == 0) {	b_dvalue = m_level + 1;		}
	
		for (int i = 0; i < matrix_size; i++) {
			groups[i] = (u8*)malloc(sizeof(u8) * sub_height * sub_width);
			if (groups[i] == NULL) { return -1; }
			memset(groups[i], 0, sizeof(u8)* sub_height* sub_width);
		}	
																				  //accuracy_ = accuracy - 1;
		for (int row = 0; row < sub_height; row++) {
			for (int col = 0; col < sub_width; col++) {
				r = sub_img[row * sub_width * ch + col * ch];
				g = sub_img[row * sub_width * ch + col * ch + 1];
				b = sub_img[row * sub_width * ch + col * ch + 2];
				int r_index = (((r- r_min) / r_dvalue) < m_level) ? ((r - r_min) / r_dvalue) : m_level - 1;
				int g_index = (((g-g_min) / g_dvalue) < m_level) ? ((g - g_min) / g_dvalue) : m_level - 1;
				int b_index = (((b-b_min) / b_dvalue) < m_level) ? ((b - b_min) / b_dvalue) : m_level - 1;
				int grid_index = r_index * m_level * m_level + \
					g_index * m_level + b_index;// 在数组中的index
				int pixel_index = row * sub_width + col;
				groups[grid_index][groups_num[grid_index]++] = pixel_index;
			}
		}


		int max_idex = 0;
		for (int i = 0; i < matrix_size; i++) {
			int pixel_num = groups_num[i];
			if (pixel_num > groups_num[max_idex]) {
				max_idex = i;
			}
		}

		//计算平均色调
		int sum_r = 0, sum_g = 0, sum_b = 0;
		/*for (int m = 0; m < matrix_size; m++) {
			std::cout << groups_num[m] << std::endl;
		}*/
		int a = groups_num[max_idex];
		for (int j = 0; j < groups_num[max_idex]; j++) {
			
			u8 r = sub_img[groups[max_idex][j] * ch];
			u8 g = sub_img[groups[max_idex][j] * ch + 1];
			u8 b = sub_img[groups[max_idex][j] * ch + 2];
			sum_r += r;
			sum_g += g;
			sum_b += b;
		}

		u8 rgb0 = u8(sum_r / groups_num[max_idex]);
		u8 rgb1 = u8(sum_g / groups_num[max_idex]);
		u8 rgb2 = u8(sum_b / groups_num[max_idex]);
		sub_imags_rgb[idx * 3] = rgb0;
		sub_imags_rgb[idx * 3+1] = rgb1;
		sub_imags_rgb[idx * 3+2] = rgb2;
		
		free(groups_num);
		groups_num = NULL;
		for (int idx2 = 0; idx2 < matrix_size; idx2++) {
			free(groups[idx2]);
			groups[idx2] = NULL;
		}
		groups = NULL;

	}

	printf("image process success!\n");

	//0左上角，1右上角，2右下角，3左下角
	u8 start_led_serial = 0;
	switch (direction) {
	case 0:
		start_led_serial = 0;
		break;
	case 1:
		start_led_serial = x_len;
		break;
	case 2:
		start_led_serial = x_len + y_len;
		break;
	case 3:
		start_led_serial = x_len + y_len + x_len;
	}
	// memcpy是浅拷贝出错
	/*u8* sub_imags_rgb_head = sub_imags_rgb;
	memcpy(pixel_frame_buff, sub_imags_rgb + start_led_serial*3, (total_led - start_led_serial) * 3 * sizeof(u8));
	memcpy(pixel_frame_buff + (total_led - start_led_serial)*3, sub_imags_rgb, start_led_serial * 3 * sizeof(u8));
	for (int idx = 0; idx < total_led; idx++) {
		std::cout << std::to_string(idx) << " : ";
		std::cout << int(pixel_frame_buff[idx * 3]) << " " << int(pixel_frame_buff[idx * 3 + 1]) << \
			" " << int(pixel_frame_buff[idx * 3 + 2]) << std::endl;
	}*/
	//memset(pixel_frame_buff, 0, total_led * 3 * sizeof(u8));
	for (int idx = 0; idx < total_led-start_led_serial; idx++) {
		pixel_frame_buff[idx * 3] = sub_imags_rgb[(idx+ start_led_serial) * 3];
		pixel_frame_buff[idx * 3+1] = sub_imags_rgb[(idx+ start_led_serial) * 3+1];
		pixel_frame_buff[idx * 3+2] = sub_imags_rgb[(idx+ start_led_serial) * 3+2];
	}
	for (int idx = total_led - start_led_serial; idx < total_led; idx++) {
		int sub_image_rgb_idx = idx - (total_led - start_led_serial);
		pixel_frame_buff[idx * 3] = sub_imags_rgb[sub_image_rgb_idx * 3];
		pixel_frame_buff[idx * 3 + 1] = sub_imags_rgb[sub_image_rgb_idx * 3 + 1];
		pixel_frame_buff[idx * 3 + 2] = sub_imags_rgb[sub_image_rgb_idx * 3 + 2];
	}

    //////////////////////////////////////////
	//sub_imags_rgb = sub_imags_rgb_head;
	free(sub_imags_rgb);
	sub_imags_rgb = NULL;

	free(sub_image_hw);
	sub_image_hw = NULL;
	for (int idx = 0; idx < total_led; idx++) {
		free(sub_images[idx]);
		sub_images[idx] = NULL;
	}
	free(new_frame);
	new_frame = NULL;
	if (pixfmt == 0) {
		free(frame);
		frame = NULL;
	}
	printf("Memory released!\n");
    return 0;
}

#endif // IMAGE_ALGORITHM
