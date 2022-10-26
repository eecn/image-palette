#include "image_algorithm.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
using namespace cv;
using namespace std;

int test_image() {
	cv::Mat img = cv::imread("demo.png");
	cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
	u8* image = img.data;
	int res = 0;
	std::ofstream log("./log.txt",  std::ios::out);
	u8 x_len = 16, y_len = 9, direction = 0, pixfmt = 1;
	u8 level = 3;
	u16 width = img.cols;
	u16 height = img.rows;
	u8 ch = img.channels();
	int total_led = 2 * (x_len + y_len);
	u8* pixel_frame_buff = (u8*)malloc(total_led * ch * sizeof(u8));
	memset(pixel_frame_buff, 0, total_led * ch * sizeof(u8));
	auto start = std::chrono::system_clock::now();
	res = image_analyse_algorithm(image, width, height, ch, pixfmt, \
		x_len, y_len, direction, level, pixel_frame_buff);
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	std::cout<< double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den
		<< "秒" << std::endl;
	for (int idx = 0; idx < total_led; idx++) {
		//std::cout << std::to_string(idx) << " : ";
		//std::cout << int(pixel_frame_buff[idx * 3]) << " " << int(pixel_frame_buff[idx * 3 + 1]) << \
		//	" " << int(pixel_frame_buff[idx * 3 + 2]) << std::endl;
		log << int(pixel_frame_buff[idx * 3]) << " " << int(pixel_frame_buff[idx * 3 + 1]) << \
			" " << int(pixel_frame_buff[idx * 3 + 2]) << std::endl;
	}
	free(pixel_frame_buff);
	log.close();
	return 0;
}

int test_video_mp4() {
	Mat img;
	VideoCapture video;
	video.open("./imgs/video.mp4");

	if (!video.isOpened())  // 判断是否调用成功
	{
		cout << "打开摄像头失败，请确实摄像头是否安装成功";
		return -1;
	}

	//video >> img;  //获取图像
	//检测是否成功获取图像
	//if (img.empty())   //判断有没有读取图像成功
	//{
	//	cout << "没有获取到图像" << endl;
	//	return -1;
	//}
	//bool isColor = (img.type() == CV_8UC3);  //判断相机（视频）类型是否为彩色
	
	std::ofstream log("./log_video.txt", std::ios::out);
	int res = 0;
	u8 x_len = 16, y_len = 9, direction = 0, pixfmt = 1;
	u8 level = 3;
	u8 ch = 3;
	int total_led = 2 * (x_len + y_len);
	u8* pixel_frame_buff = (u8*)malloc(total_led * ch * sizeof(u8));
	vector<chrono::microseconds> process_time;
	while (1)
	{
		//检测是否执行完毕
		if (!video.read(img))   //判断能都继续从摄像头或者视频文件中读出一帧图像
		{
			cout << "摄像头断开连接或者视频读取完成" << endl;
			break;
		}
		//cv::imwrite("mp4_1stimg.png", img);
		//////////////////////////////////////////////////////
		u8* image = img.data;
		u16 width = img.cols;
		u16 height = img.rows;
		memset(pixel_frame_buff, 0, total_led * ch * sizeof(u8));
		auto start = std::chrono::system_clock::now();
		res = image_analyse_algorithm(image, width, height, ch, pixfmt, \
			x_len, y_len, direction, level, pixel_frame_buff);
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		process_time.push_back(duration);
		for (int idx = 0; idx < total_led; idx++) {
			log << int(pixel_frame_buff[idx * 3]) << " " << int(pixel_frame_buff[idx * 3 + 1]) << \
				" " << int(pixel_frame_buff[idx * 3 + 2]) << std::endl;
		}
		//////////////////////////////////////////////////////
		//writer.write(img);  //把图像写入视频流
		//writer << img;
		imshow("Live", img);  //显示图像
		char c = waitKey(50);
		if (c == 27)  //按ESC案件退出视频保存
		{
			break;
		}	
	}
	double average_time = 0;
	for (int idx = 0; idx < process_time.size(); idx++) {
		average_time += double(process_time[idx].count());
	}
	std::cout << average_time / process_time.size() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "sceonds" << std::endl;
	log.close();
	return 0;
}

int test_video_yuv() {
	const char* video_path = "./video/16053.yuv";
	int frame_height = 360;
	int frame_width = 640;
	int res = 0;
	u8 x_len = 16, y_len = 9, direction = 0, pixfmt = 0;
	u8 level = 3;
	u8 ch = 3;
	int total_led = 2 * (x_len + y_len);
	u8* pixel_frame_buff = (u8*)malloc(total_led * ch * sizeof(u8));
	vector<chrono::microseconds> process_time;

	int frameSize = frame_height * frame_width * 3 / 2;
	unsigned char* buffer = (unsigned char*)malloc(frameSize * sizeof(unsigned char));
	FILE* fd = fopen(video_path, "rb+");
	if (NULL == fd) { return -1; }

	fseek(fd, 0, SEEK_END);
	int size = ftell(fd);
	rewind(fd);
	int frameCount = size / frameSize;
	std::ofstream log("./log_video_yuv.txt", std::ios::out);
	for (int i = 0; i < frameCount; i++)
	{
		fread(buffer, 1, frameSize * sizeof(unsigned char), fd);
		memset(pixel_frame_buff, 0, total_led * ch * sizeof(u8));
		auto start = std::chrono::system_clock::now();
		res = image_analyse_algorithm(buffer, frame_width, frame_height, ch, pixfmt, \
			x_len, y_len, direction, level, pixel_frame_buff);
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		process_time.push_back(duration);
		for (int idx = 0; idx < total_led; idx++) {
			log << int(pixel_frame_buff[idx * 3]) << " " << int(pixel_frame_buff[idx * 3 + 1]) << \
				" " << int(pixel_frame_buff[idx * 3 + 2]) << std::endl;
		}

	}
	double average_time = 0;
	for (int idx = 0; idx < process_time.size(); idx++) {
		average_time += double(process_time[idx].count());
	}
	std::cout << average_time / process_time.size() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "sceonds" << std::endl;
	log.close();
	return 0;
}

int  main() {
	int res = -1;
	res = test_video_mp4();
	// res = test_video_yuv();
	return 0;
}