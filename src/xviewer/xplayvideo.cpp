#include "xplayvideo.h"

XPlayVideo::XPlayVideo(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	startTimer(10);
}

XPlayVideo::~XPlayVideo()
{
	Close(); // �ر��ϴ�����
}

bool XPlayVideo::Open(const char* url)
{
	if (!demux_.Open(url)) // ���װ
	{
		return false;
	}
	auto vp = demux_.CopyVideoPara();
	if (!vp)
	{
		return false;
	}
	if (!decode_.Open(vp->para)) // ����
	{
		return false;
	}
	demux_.set_next(&decode_);
	if (!view_)
	{
		//view_->Close();
		//delete view_;
		//view_ = nullptr;
		view_ = XVideoView::Create();
	}
	view_->set_win_id((void*)winId()); 
	if (view_->Init(vp->para))			// SDL��Ⱦ
		return false;
	demux_.set_syn_type(XSYN_VIDEO);
	demux_.Start();
	decode_.Start();
	return true;
}

void XPlayVideo::timerEvent(QTimerEvent* ev)
{
	if (!view_) return; // ���û����ͼ����Ⱦ
	auto f = decode_.GetFrame();
	if (!f) return;
	view_->DrawFrame(f);
	XFreeFrame(&f); // �ͷ�֡�ڴ�
}

void XPlayVideo::Close()
{
	// �ر��ϴ�����
	demux_.Stop();
	decode_.Stop();
	if (view_)
	{
		view_->Close(); // �ر��ϴ���ͼ
		delete view_;
		view_ = nullptr; // �ͷ���ͼ
	}
}

void XPlayVideo::closeEvent(QCloseEvent* ev)
{
	Close();
}
