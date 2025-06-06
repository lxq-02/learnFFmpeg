#include "xplayer.h"
#include "xaudio_play.h"

bool XPlayer::Open(const char* url, void* winid)
{
    // 解封装
    if (!demux_.Open(url))
        return false;

    // 视频解码
    auto vp = demux_.CopyVideoPara();
    if (vp)
    {
        if (!video_decode_.Open(vp->para))
        {
            return false;
        }
        // 用于过滤音频包
        video_decode_.set_stream_index(demux_.video_index());
        // 视频渲染
        if (!view_)
            view_ = XVideoView::Create();
        view_->set_win_id(winid);
        if (!view_->Init(vp->para))
            return false;
    }

    auto ap = demux_.CopyAudioPara();
    if (ap)
    {
		// 音频解码
		if (!audio_decode_.Open(ap->para))
		{
			return false;
		}
		// 用于过滤视频包
        audio_decode_.set_stream_index(demux_.audio_index());

        // 初始化音频播放
		XAudioPlay::Instance()->Open(ap->para);

    }
    else
    {
        demux_.set_syn_type(XSYN_VIDEO);    // 根据视频同步
    }

    // 解封装数据传到当前类
    demux_.set_next(this);
    return false;
}

void XPlayer::Main()
{

}

void XPlayer::Do(AVPacket* pkt)
{
    if (audio_decode_.is_open())
	    audio_decode_.Do(pkt);
    if (video_decode_.is_open())
	    video_decode_.Do(pkt);
}

void XPlayer::Start()
{
    demux_.Start();
    if (video_decode_.is_open())
        video_decode_.Start();
    if (audio_decode_.is_open())
	    audio_decode_.Start();
    XThread::Start();
}
