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
			// ��ȡʧ��
			cout << "-" << flush;
			if (!demux_.is_connected())
			{
				Open(url_, timeout_ms_);
			}
			this_thread::sleep_for(chrono::milliseconds(1));
			continue;
		}
		cout << "." << flush;
		Next(&pkt); // ���ݵ���һ������������
		av_packet_unref(&pkt); // ����֮�����ü�����һ
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
