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
	 * 创建编码上下文
	 * 
	 * @param codec_id 编码器ID号，对应ffmpeg的AVCodecID枚举值
	 * @return 编码上下文，失败返回nullptr
	 */
	AVCodecContext* Create(int codec_id);

	/**
	 * 设置对象的编码上下文 上下文传递到对象中，空间由XEncode管理
	 * 加锁 线程安全
	 * @param ctx 编码上下文 如果_ctx不为nullptr，则先清理资源
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
	 * 编码数据，线程安全 每次新创建AVPacket
	 * 
	 * @param frame 空间由用户维护
	 * @return 失败返回nullptr 返回的AVPacket用户需要通过av_packet_free清理
	 */
	AVPacket* Encode(const AVFrame* frame);

	/**
	 * 根据AVCodecContext创建一个AVFrame，需要调用者释放 av_frame_free
	 * 
	 * @return 
	 */
	AVFrame* CreateFrame();

	/**
	 * 返回所有编码缓存中AVPacket
	 * 
	 * @return 
	 */
	std::vector<AVPacket *> End();

private:
	AVCodecContext* _ctx = nullptr; // 编码上下文
	std::mutex _mtx;				// 编码线程互斥锁
};



