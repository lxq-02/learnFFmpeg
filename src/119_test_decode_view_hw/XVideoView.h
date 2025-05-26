#pragma once

#include <mutex>
#include <fstream>
struct AVFrame;

void MSleep(unsigned int ms);

// 获取当前时间戳 毫秒
long long NowMs();

/*
* 视频渲染接口类
* 隐藏SDL实现
* 渲染方案可替代
* 线程安全
*/
class XVideoView
{
public:
	enum Format // 枚举的值和ffmpeg中值一致
	{
		YUV420P = 0,
		RGB24 = 2,
		NV12 = 23,
		ARGB = 25,
		RGBA = 26,
		BGRA = 28
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
		Format fmt = RGBA) = 0;

	// 清理所有申请的资源，包括关闭窗口
	virtual void Close() = 0;

	// 处理窗口退出事件
	virtual bool IsExit() = 0;

	/*
	* 渲染图像，线程安全
	* @para data 渲染的二进制数据
	* @para linesize 一行数据的字节数，对于YUV420P就是Y一行字节数
	* linesize <= 0 根据宽度和像素格式自动算出大小
	* @return 渲染是否成功
	*/
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;
	virtual bool Draw(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch
	) = 0;

	// 显示缩放
	void Scale(int w, int h)
	{
		_scale_w = w;
		_scale_h = h;
	}

	bool DrawFrame(AVFrame* frame);

	int render_fps() { return _render_fps; }

	// 打开文件
	bool Open(std::string filepath);
	// 读取一帧数据，并维护AVFrame空间
	// 每次调用会覆盖上一次数据
	AVFrame* Read();
	void set_win_id(void* win) { _win_id = win; }
	virtual ~XVideoView();
protected:
	void* _win_id = nullptr;		// 窗口句柄
	int _render_fps = 0; // 显示帧率
	int _width = 0;
	int _height = 0;
	Format _fmt = RGBA; // 像素格式
	std::mutex _mtx;	// 确保线程安全

	int _scale_w = 0;	// 显示大小
	int _scale_h = 0;	
	
	std::chrono::steady_clock::time_point _beg_ms;	// 计时开始时间
	int _count = 0;	// 统计显示次数

private:
	std::ifstream _ifs; 
	AVFrame* _frame = nullptr;
	unsigned char* _cache = nullptr; // 复制NV12数据用
};

