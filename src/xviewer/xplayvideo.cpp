#include "xplayvideo.h"

XPlayVideo::XPlayVideo(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	startTimer(10);
}

XPlayVideo::~XPlayVideo()
{
	Close(); // 关闭上次数据
}

bool XPlayVideo::Open(const char* url)
{
	if (!demux_.Open(url)) // 解封装
	{
		return false;
	}
	auto vp = demux_.CopyVideoPara();
	if (!vp)
	{
		return false;
	}
	if (!decode_.Open(vp->para)) // 解码
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
	if (view_->Init(vp->para))			// SDL渲染
		return false;
	demux_.set_syn_type(XSYN_VIDEO);
	demux_.Start();
	decode_.Start();
	return true;
}

void XPlayVideo::timerEvent(QTimerEvent* ev)
{
	if (!view_) return; // 如果没有视图则不渲染
	auto f = decode_.GetFrame();
	if (!f) return;
	view_->DrawFrame(f);
	XFreeFrame(&f); // 释放帧内存
}

void XPlayVideo::Close()
{
	// 关闭上次数据
	demux_.Stop();
	decode_.Stop();
	if (view_)
	{
		view_->Close(); // 关闭上次视图
		delete view_;
		view_ = nullptr; // 释放视图
	}
}

void XPlayVideo::closeEvent(QCloseEvent* ev)
{
	Close();
}
