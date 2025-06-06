#include "xplayvideo.h"
#include <QDebug>

XPlayVideo::XPlayVideo(QDialog*parent)
	: QDialog(parent)
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
	if (!player.Open(url, (void*)ui.video->winId()))
	{
		return false;
	}
	player.Start();
	startTimer(10);

	//if (!demux_.Open(url)) // 解封装
	//{
	//	return false;
	//}
	//auto vp = demux_.CopyVideoPara();
	//if (!vp)
	//{
	//	return false;
	//}
	//if (!decode_.Open(vp->para)) // 解码
	//{
	//	return false;
	//}
	//demux_.set_next(&decode_);
	//if (!view_)
	//{
	//	//view_->Close();
	//	//delete view_;
	//	//view_ = nullptr;
	//	view_ = XVideoView::Create();
	//}
	//view_->set_win_id((void*)winId()); 
	//if (view_->Init(vp->para))			// SDL渲染
	//	return false;
	//demux_.set_syn_type(XSYN_VIDEO);
	//demux_.Start();
	//decode_.Start();
	return true;
}

void XPlayVideo::timerEvent(QTimerEvent* ev)
{
	//if (!view_) return; // 如果没有视图则不渲染
	//auto f = decode_.GetFrame();
	//if (!f) return;
	//view_->DrawFrame(f);
	//XFreeFrame(&f); // 释放帧内存
	player.Update();
	auto pos = player.pos_ms();
	auto total = player.total_ms();
	ui.pos->setMaximum(total);

	ui.pos->setValue(pos);
}

void XPlayVideo::Close()
{
	player.Stop();
	//// 关闭上次数据
	//demux_.Stop();
	//decode_.Stop();
	//if (view_)
	//{
	//	view_->Close(); // 关闭上次视图
	//	delete view_;
	//	view_ = nullptr; // 释放视图
	//}
}

void XPlayVideo::closeEvent(QCloseEvent* ev)
{
	Close();
}

void XPlayVideo::SetSpeed()
{
	float speed = 1;
	int s = ui.speed->value();

	if (s <= 10)
	{
		speed = (float)s / 10;
	}
	else
	{
		speed = s - 9;
	}
	ui.speedtxt->setText(QString::number(speed));

	player.SetSpeed(speed);
}
