#pragma once

#include "XVideoView.h"
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class XSDL : public XVideoView
{
public:
	// ���������������Դ�������رմ���
	void Close() override;

	// �������˳��¼�
	bool IsExit() override;

	/*
	* ��ʼ����Ⱦ����
	* @para w ���ڿ��
	* @para h ���ڸ߶�
	* @para fmt ���Ƶ����ظ�ʽ
	* @para win_id ���ھ�������Ϊ�գ������´���
	* @return �Ƿ񴴽��ɹ�
	*/
	bool Init(int w, int h,
		Format fmt = RGBA,
		void* win_id = nullptr) override;

	/*
	* ��Ⱦͼ���̰߳�ȫ
	* @para data ��Ⱦ�Ķ���������
	* @para linesize һ�����ݵ��ֽ���������YUV420P����Yһ���ֽ���
	* linesize <= 0 ���ݿ�Ⱥ����ظ�ʽ�Զ������С
	* @return ��Ⱦ�Ƿ�ɹ�
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

