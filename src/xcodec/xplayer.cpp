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
        // 视频总时长
        this->total_ms_ = vp->total_ms;

        if (!video_decode_.Open(vp->para))
        {
            return false;
        }
        // 用于过滤音频包
        video_decode_.set_stream_index(demux_.video_index());
        // 缓冲
        video_decode_.set_block_size(100);
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
        // 缓冲
		audio_decode_.set_block_size(100); // 缓冲大小

		// 用于过滤视频包
        audio_decode_.set_stream_index(demux_.audio_index());

        // frame 缓冲
        audio_decode_.set_frame_cache(true);

        // 初始化音频播放
		XAudioPlay::Instance()->Open(*ap);
    }
    else
    {
        demux_.set_syn_type(XSYN_VIDEO);    // 根据视频同步
    }

    // 解封装数据传到当前类
    demux_.set_next(this);
    return true;
}

void XPlayer::Stop()
{
    XThread::Stop();
    demux_.Stop();
    audio_decode_.Stop();
    video_decode_.Stop();
    Wait();
    demux_.Wait();
    audio_decode_.Wait();
    video_decode_.Wait();
    if (view_)
    {
        view_->Close();
        delete view_;
        view_ = nullptr;
    }
    XAudioPlay::Instance()->Close();
}

void XPlayer::Main()
{
    long long syn = 0;
	XAudioPlay* au = XAudioPlay::Instance();
	auto ap = demux_.CopyAudioPara();
    auto vp = demux_.CopyVideoPara();
    video_decode_.set_time_base(vp->time_base);
    while (!is_exit_)
    {
        this->pos_ms_ = video_decode_.cur_ms();
        if (ap)
        {
            syn = XRescale(au->cur_pts(), ap->time_base, vp->time_base);
            audio_decode_.set_syn_pts(au->cur_pts() + 10000);
            video_decode_.set_syn_pts(syn);
        }
        MSleep(1);
    }
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

void XPlayer::Update()
{
    // 渲染视频
	if (view_)
	{
		auto f = video_decode_.GetFrame();
		if (f)
		{
			view_->DrawFrame(f);
			XFreeFrame(&f); // 释放 AVFrame 内存
		}
	}

    // 音频播放
    auto au = XAudioPlay::Instance();
    auto f = audio_decode_.GetFrame();
    if (!f) return;
    au->Push(f);
    XFreeFrame(&f);
}

void XPlayer::SetSpeed(float s)
{
    XAudioPlay::Instance()->SetSpeed(s);
}
