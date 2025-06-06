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
	Close(); // �ر��ϴ�����
}

bool XPlayVideo::Open(const char* url)
{
	if (!player.Open(url, (void*)ui.video->winId()))
	{
		return false;
	}
	player.Start();
	startTimer(10);

	//if (!demux_.Open(url)) // ���װ
	//{
	//	return false;
	//}
	//auto vp = demux_.CopyVideoPara();
	//if (!vp)
	//{
	//	return false;
	//}
	//if (!decode_.Open(vp->para)) // ����
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
	//if (view_->Init(vp->para))			// SDL��Ⱦ
	//	return false;
	//demux_.set_syn_type(XSYN_VIDEO);
	//demux_.Start();
	//decode_.Start();
	return true;
}

void XPlayVideo::timerEvent(QTimerEvent* ev)
{
	//if (!view_) return; // ���û����ͼ����Ⱦ
	//auto f = decode_.GetFrame();
	//if (!f) return;
	//view_->DrawFrame(f);
	//XFreeFrame(&f); // �ͷ�֡�ڴ�
	player.Update();
	auto pos = player.pos_ms();
	auto total = player.total_ms();
	ui.pos->setMaximum(total);

	ui.pos->setValue(pos);
}

void XPlayVideo::Close()
{
	player.Stop();
	//// �ر��ϴ�����
	//demux_.Stop();
	//decode_.Stop();
	//if (view_)
	//{
	//	view_->Close(); // �ر��ϴ���ͼ
	//	delete view_;
	//	view_ = nullptr; // �ͷ���ͼ
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
