#pragma once

#include "XVideoView.h"
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class XSDL : public XVideoView
{
public:
	// 清理所有申请的资源，包括关闭窗口
	void Close() override;

	// 处理窗口退出事件
	bool IsExit() override;

	/*
	* 初始化渲染窗口
	* @para w 窗口宽度
	* @para h 窗口高度
	* @para fmt 绘制的像素格式
	* @para win_id 窗口句柄，如果为空，创建新窗口
	* @return 是否创建成功
	*/
	bool Init(int w, int h,
		Format fmt = RGBA,
		void* win_id = nullptr) override;

	/*
	* 渲染图像，线程安全
	* @para data 渲染的二进制数据
	* @para linesize 一行数据的字节数，对于YUV420P就是Y一行字节数
	* linesize <= 0 根据宽度和像素格式自动算出大小
	* @return 渲染是否成功
	*/
	bool Draw(const unsigned char* data, int linesize = 0) override;
	bool Draw(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch
	) override;

private:
	SDL_Window* _window = nullptr;
	SDL_Renderer* _renderer = nullptr;
	SDL_Texture* _texture = nullptr;
};

