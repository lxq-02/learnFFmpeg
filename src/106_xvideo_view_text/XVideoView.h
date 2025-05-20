#pragma once

#include <mutex>

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

	/*
	* ��Ⱦͼ���̰߳�ȫ
	* @para data ��Ⱦ�Ķ���������
	* @para linesize һ�����ݵ��ֽ���������YUV420P����Yһ���ֽ���
	* linesize <= 0 ���ݿ�Ⱥ����ظ�ʽ�Զ������С
	* @return ��Ⱦ�Ƿ�ɹ�
	*/
	virtual bool Draw(const unsigned char* data, int linesize = 0) = 0;

protected:
	int _width = 0;
	int _height = 0;
	Format _fmt = RGBA; // ���ظ�ʽ
	std::mutex _mtx;	// ȷ���̰߳�ȫ
};

