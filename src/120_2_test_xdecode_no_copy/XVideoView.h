#pragma once

#include <mutex>
#include <fstream>
struct AVFrame;

void MSleep(unsigned int ms);

// ��ȡ��ǰʱ��� ����
long long NowMs();

/*
* ��Ƶ��Ⱦ�ӿ���
* ����SDLʵ��
* ��Ⱦ���������
* �̰߳�ȫ
*/
class XVideoView
{
public:
	enum Format // ö�ٵ�ֵ��ffmpeg��ֵһ��
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
	* ��ʼ����Ⱦ����
	* @para w ���ڿ��
	* @para h ���ڸ߶�
	* @para fmt ���Ƶ����ظ�ʽ
	* @para win_id ���ھ�������Ϊ�գ������´���
	* @return �Ƿ񴴽��ɹ�
	*/
	virtual bool Init(int w, int h,
		Format fmt = RGBA) = 0;

	// ���������������Դ�������رմ���
	virtual void Close() = 0;

	// �������˳��¼�
	virtual bool IsExit() = 0;

	/*
	* ��Ⱦͼ���̰߳�ȫ
	* @para data ��Ⱦ�Ķ���������
	* @para linesize һ�����ݵ��ֽ���������YUV420P����Yһ���ֽ���
	* linesize <= 0 ���ݿ�Ⱥ����ظ�ʽ�Զ������С
	* @return ��Ⱦ�Ƿ�ɹ�
	*/
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;
	virtual bool Draw(
		const unsigned char* y, int y_pitch,
		const unsigned char* u, int u_pitch,
		const unsigned char* v, int v_pitch
	) = 0;

	// ��ʾ����
	void Scale(int w, int h)
	{
		_scale_w = w;
		_scale_h = h;
	}

	bool DrawFrame(AVFrame* frame);

	int render_fps() { return _render_fps; }

	// ���ļ�
	bool Open(std::string filepath);
	// ��ȡһ֡���ݣ���ά��AVFrame�ռ�
	// ÿ�ε��ûḲ����һ������
	AVFrame* Read();
	void set_win_id(void* win) { _win_id = win; }
	virtual ~XVideoView();
protected:
	void* _win_id = nullptr;		// ���ھ��
	int _render_fps = 0; // ��ʾ֡��
	int _width = 0;
	int _height = 0;
	Format _fmt = RGBA; // ���ظ�ʽ
	std::mutex _mtx;	// ȷ���̰߳�ȫ

	int _scale_w = 0;	// ��ʾ��С
	int _scale_h = 0;	
	
	std::chrono::steady_clock::time_point _beg_ms;	// ��ʱ��ʼʱ��
	int _count = 0;	// ͳ����ʾ����

private:
	std::ifstream _ifs; 
	AVFrame* _frame = nullptr;
	unsigned char* _cache = nullptr; // ����NV12������
};

