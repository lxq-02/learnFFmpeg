#include "xcamera_widget.h"
#include <QStyleOption>
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDebug>
#include <QListWidget>
#include "xdemux_task.h"
#include "xdecode_task.h"
#include "xvideo_view.h"
#include "xcamera_config.h"

XCameraWidget::XCameraWidget(QWidget* p)
	:QWidget(p)
{
	// 接收拖拽
	this->setAcceptDrops(true);
}

XCameraWidget::~XCameraWidget()
{
	if (demux_)
	{
		demux_->Stop();
		delete demux_;
		demux_ = nullptr;
	}
	if (decode_)
	{
		decode_->Stop();
		delete decode_;
		decode_ = nullptr;
	}
	if (view_)
	{
		view_->Close();
		delete view_;
		view_ = nullptr;
	}
}

void XCameraWidget::dragEnterEvent(QDragEnterEvent* ev)
{
	// 接收拖拽进入
	ev->acceptProposedAction();
}

void XCameraWidget::dropEvent(QDropEvent* ev)
{
	// 拿到url
	qDebug() << ev->source()->objectName();
	auto wid = (QListWidget*)ev->source();
	qDebug() << wid->currentRow();
	auto cam = XCameraConfig::Instance()->GetCam(wid->currentRow());
	Open(cam.sub_url_);
}

void XCameraWidget::paintEvent(QPaintEvent* p)
{
	// 渲染样式表
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

bool XCameraWidget::Open(const char* url)
{
	if (demux_)
		demux_->Stop();
	if (decode_)
		decode_->Stop();

	// 打开解封装
	demux_ = new XDemuxTask();
	if (!demux_->Open(url))
	{
		return false;
	}

	// 打开视频解码器
	decode_ = new XDecodeTask();
	auto para = demux_->CopyVideoPara();
	if (!decode_->Open(para->para))
	{
		return false;
	}

	// 设定解码线程接收解封装数据
	demux_->set_next(decode_);

	// 初始化渲染参数
	view_ = XVideoView::Create();
	view_->set_win_id((void*)winId());
	view_->Init(para->para);

	// 启动解封装和解码线程
	demux_->Start();
	decode_->Start();
	return true;
}

void XCameraWidget::Draw()
{
	if (!demux_ || !decode_ || !view_)
	{
		return;
	}
	auto f = decode_->GetFrame();
	if (!f) return;
	view_->DrawFrame(f);
	XFreeFrame(&f);
}
