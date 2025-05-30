#pragma once
#include <mutex>
#include <vector>
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

void PrintErr(int err);
//////////////////////////////////////////
/// ����ͽ���Ļ���
class XCodec
{
public:
	/**
	 * �����������������
	 *
	 * @param codec_id ������ID�ţ���Ӧffmpeg��AVCodecIDö��ֵ
	 * @param is_encode �Ƿ��Ǳ�������true��ʾ��������false��ʾ������
	 * @return ���������ģ�ʧ�ܷ���nullptr
	 */
	static AVCodecContext* Create(int codec_id, bool is_encode);

	/**
	 * ���ö���ı��������� �����Ĵ��ݵ������У��ռ���XEncode����
	 * ���� �̰߳�ȫ
	 * @param ctx ���������� ���ctx_��Ϊnullptr������������Դ
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
	 * ����AVCodecContext����һ��AVFrame����Ҫ�������ͷ� av_frame_free
	 *
	 * @return
	 */
	AVFrame* CreateFrame();

protected:
	AVCodecContext* ctx_ = nullptr; // ����������
	std::mutex mtx_;				// �����̻߳�����
};

