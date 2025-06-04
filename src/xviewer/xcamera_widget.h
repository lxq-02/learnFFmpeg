#pragma once
#include <QWidget>

class XDecodeTask;
class XDemuxTask;
class XVideoView;

class XCameraWidget :public QWidget
{
	Q_OBJECT

public:
	XCameraWidget(QWidget* p = nullptr);
	~XCameraWidget();
	
	// 拖拽进入
	void dragEnterEvent(QDragEnterEvent* ev) override;

	// 拖拽松开
	void dropEvent(QDropEvent* ev) override;

	// 渲染
	void paintEvent(QPaintEvent* p) override;

	// 打开rtsp 开始解封装和解码
	bool Open(const char* url);

	// 渲染视频
	void Draw();

private:
	XDecodeTask* decode_ = nullptr;
	XDemuxTask* demux_ = nullptr;
	XVideoView* view_ = nullptr;
};

