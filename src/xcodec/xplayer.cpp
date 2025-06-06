#include "xplayer.h"
#include "xaudio_play.h"

bool XPlayer::Open(const char* url, void* winid)
{
    // ���װ
    if (!demux_.Open(url))
        return false;

    // ��Ƶ����
    auto vp = demux_.CopyVideoPara();
    if (vp)
    {
        // ��Ƶ��ʱ��
        this->total_ms_ = vp->total_ms;

        if (!video_decode_.Open(vp->para))
        {
            return false;
        }
        // ���ڹ�����Ƶ��
        video_decode_.set_stream_index(demux_.video_index());
        // ����
        video_decode_.set_block_size(100);
        // ��Ƶ��Ⱦ
        if (!view_)
            view_ = XVideoView::Create();
        view_->set_win_id(winid);
        if (!view_->Init(vp->para))
            return false;
    }

    auto ap = demux_.CopyAudioPara();
    if (ap)
    {
		// ��Ƶ����
		if (!audio_decode_.Open(ap->para))
		{
			return false;
		}
        // ����
		audio_decode_.set_block_size(100); // �����С

		// ���ڹ�����Ƶ��
        audio_decode_.set_stream_index(demux_.audio_index());

        // frame ����
        audio_decode_.set_frame_cache(true);

        // ��ʼ����Ƶ����
		XAudioPlay::Instance()->Open(*ap);
    }
    else
    {
        demux_.set_syn_type(XSYN_VIDEO);    // ������Ƶͬ��
    }

    // ���װ���ݴ�����ǰ��
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
    // ��Ⱦ��Ƶ
	if (view_)
	{
		auto f = video_decode_.GetFrame();
		if (f)
		{
			view_->DrawFrame(f);
			XFreeFrame(&f); // �ͷ� AVFrame �ڴ�
		}
	}

    // ��Ƶ����
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
