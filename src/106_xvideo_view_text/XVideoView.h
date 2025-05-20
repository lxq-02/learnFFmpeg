#pragma once

#include <mutex>

/*
* 视频渲染接口类
* 隐藏SDL实现
* 渲染方案可替代
* 线程安全
*/
class XVideoView
{
public:
	enum Format
	{
		RGBA = 0,
		ARGB,
		YUV420P
	};

	enum RenderType
	{
		SDL = 0
	};

	static XVideoView* Create(RenderType type = SDL);

	/*
	* 初始化渲染窗口
	* @para w 窗口宽度
	* @para h 窗口高度
	* @para fmt 绘制的像素格式
	* @para win_id 窗口句柄，如果为空，创建新窗口
	* @return 是否创建成功
	*/
	virtual bool Init(int w, int h,
		Format fmt = RGBA,
		void* win_id = nullptr) = 0;

	/*
	* 渲染图像，线程安全
	* @para data 渲染的二进制数据
	* @para linesize 一行数据的字节数，对于YUV420P就是Y一行字节数
	* linesize <= 0 根据宽度和像素格式自动算出大小
	* @return 渲染是否成功
	*/
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;

protected:
	int _width = 0;
	int _height = 0;
	Format _fmt = RGBA; // 像素格式
	std::mutex _mtx;	// 确保线程安全
};

