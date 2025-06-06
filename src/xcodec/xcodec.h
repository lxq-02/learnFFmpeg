#pragma once
#include "xtools.h"
#include <mutex>
#include <vector>

//////////////////////////////////////////
/// 编码和解码的基类
class XCODEC_API XCodec
{
public:
	/**
	 * 创建编解码码上下文
	 *
	 * @param codec_id 编码器ID号，对应ffmpeg的AVCodecID枚举值
	 * @param is_encode 是否是编码器，true表示编码器，false表示解码器
	 * @return 编码上下文，失败返回nullptr
	 */
	static AVCodecContext* Create(int codec_id, bool is_encode);

	/**
	 * 设置对象的编码上下文 上下文传递到对象中，空间由XEncode管理
	 * 加锁 线程安全
	 * @param ctx 编码上下文 如果ctx_不为nullptr，则先清理资源
	 */
	void set_context(AVCodecContext* ctx);

	/**
	 * 设置编码参数，线程安全
	 *
	 * @param key
	 * @param val
	 * @return
	 */
	bool SetOpt(const char* key, const char* val);
	bool SetOpt(const char* key, int val);

	/**
	 * 打开编码器 线程安全
	 *
	 */
	bool Open();

	/**
	 * 根据AVCodecContext创建一个AVFrame，需要调用者释放 av_frame_free
	 *
	 * @return
	 */
	AVFrame* CreateFrame();

	virtual void Clear();

protected:
	AVCodecContext* ctx_ = nullptr; // 编码上下文
	std::mutex mtx_;				// 编码线程互斥锁
};

