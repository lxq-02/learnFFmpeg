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
        if (!video_decode_.Open(vp->para))
        {
            return false;
        }
        // ���ڹ�����Ƶ��
        video_decode_.set_stream_index(demux_.video_index());
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
		// ���ڹ�����Ƶ��
        audio_decode_.set_stream_index(demux_.audio_index());

        // ��ʼ����Ƶ����
		XAudioPlay::Instance()->Open(ap->para);

    }
    else
    {
        demux_.set_syn_type(XSYN_VIDEO);    // ������Ƶͬ��
    }

    // ���װ���ݴ�����ǰ��
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
