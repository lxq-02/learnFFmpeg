#pragma once
#include <mutex>
#include <vector>
struct AVCodecContext;
struct AVPacket;
struct AVFrame;
class XEncode
{
public:
	/**
	 * ��������������
	 * 
	 * @param codec_id ������ID�ţ���Ӧffmpeg��AVCodecIDö��ֵ
	 * @return ���������ģ�ʧ�ܷ���nullptr
	 */
	AVCodecContext* Create(int codec_id);

	/**
	 * ���ö���ı��������� �����Ĵ��ݵ������У��ռ���XEncode����
	 * ���� �̰߳�ȫ
	 * @param ctx ���������� ���_ctx��Ϊnullptr������������Դ
	 */
	void set_context(AVCodecContext* ctx);

	/**
	 * ���ñ���������̰߳�ȫ
	 * 
	 * @param key
	 * @param val
	 * @return 
	 */
	bool SetOpt(const char* key, const char* val);
	bool SetOpt(const char* key, int val);

	/**
	 * �򿪱����� �̰߳�ȫ
	 * 
	 */
	bool Open();

	/**
	 * �������ݣ��̰߳�ȫ ÿ���´���AVPacket
	 * 
	 * @param frame �ռ����û�ά��
	 * @return ʧ�ܷ���nullptr ���ص�AVPacket�û���Ҫͨ��av_packet_free����
	 */
	AVPacket* Encode(const AVFrame* frame);

	/**
	 * ����AVCodecContext����һ��AVFrame����Ҫ�������ͷ� av_frame_free
	 * 
	 * @return 
	 */
	AVFrame* CreateFrame();

	/**
	 * �������б��뻺����AVPacket
	 * 
	 * @return 
	 */
	std::vector<AVPacket *> End();

private:
	AVCodecContext* _ctx = nullptr; // ����������
	std::mutex _mtx;				// �����̻߳�����
};



