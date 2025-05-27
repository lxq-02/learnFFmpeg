#pragma once
#include "xcodec.h"
class XDecode :public XCodec
{
public:
	bool Send(const AVPacket* pkt); // 发送解码
	bool Recv(AVFrame* frame, bool is_hw_copy = true);		// 获取解码
	std::vector<AVFrame*> End();	// 获取缓存数据

	/**
	 * 初始化硬件加速 
	 * 
	 * @param type 4 AV_HWDEVICE_TYPE_DXVA2
	 * @return 
	 */
	bool InitHW(int type = 4);
};

