#include "xdemux_task.h"
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}

void XDemuxTask::Main()
{
	AVPacket pkt;
	while (!is_exit_)
	{
		if (!demux_.Read(&pkt))
		{
			// 读取失败
			cout << "-" << flush;
			if (!demux_.is_connected())
			{
				Open(url_, timeout_ms_);
			}
			this_thread::sleep_for(chrono::milliseconds(1));
			continue;
		}
		cout << "." << flush;
		Next(&pkt); // 传递到下一个责任链函数
		av_packet_unref(&pkt); // 传递之后，引用计数加一
		this_thread::sleep_for(chrono::milliseconds(1));
	}
}

bool XDemuxTask::Open(std::string url, int timeout_ms)
{
	LOGDEBUG("XDemuxTask::Open begin!");
	demux_.set_ctx(nullptr);
	this->url_ = url;
	this->timeout_ms_ = timeout_ms;
	auto c = demux_.Open(url.c_str());
	if (!c) return false;
	demux_.set_ctx(c);
	demux_.set_time_out_ms(timeout_ms);
	LOGDEBUG("XDemuxTask::Open end!");
	return true;
}
