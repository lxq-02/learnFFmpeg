#pragma once

#include <mutex>
struct AVFrame;

/*
* ��Ƶ��Ⱦ�ӿ���
* ����SDLʵ��
* ��Ⱦ���������
* �̰߳�ȫ
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
	* ��ʼ����Ⱦ����
	* @para w ���ڿ��
	* @para h ���ڸ߶�
	* @para fmt ���Ƶ����ظ�ʽ
	* @para win_id ���ھ�������Ϊ�գ������´���
	* @return �Ƿ񴴽��ɹ�
	*/
	virtual bool Init(int w, int h,
		Format fmt = RGBA,
		void* win_id = nullptr) = 0;

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
protected:
	int _width = 0;
	int _height = 0;
	Format _fmt = RGBA; // ���ظ�ʽ
	std::mutex _mtx;	// ȷ���̰߳�ȫ

	int _scale_w = 0;	// ��ʾ��С
	int _scale_h = 0;	// 
};

