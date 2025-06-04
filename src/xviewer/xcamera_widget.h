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
	
	// ��ק����
	void dragEnterEvent(QDragEnterEvent* ev) override;

	// ��ק�ɿ�
	void dropEvent(QDropEvent* ev) override;

	// ��Ⱦ
	void paintEvent(QPaintEvent* p) override;

	// ��rtsp ��ʼ���װ�ͽ���
	bool Open(const char* url);

	// ��Ⱦ��Ƶ
	void Draw();

private:
	XDecodeTask* decode_ = nullptr;
	XDemuxTask* demux_ = nullptr;
	XVideoView* view_ = nullptr;
};

