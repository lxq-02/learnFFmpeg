#pragma once

#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QMenu>
#include "ui_xviewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class XViewerClass; };
QT_END_NAMESPACE

class XViewer : public QWidget
{
    Q_OBJECT

public:
    XViewer(QWidget *parent = nullptr);
    ~XViewer();

public:
    // 鼠标事件 用于拖动窗口
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

    // 窗口大小发生变化
    void resizeEvent(QResizeEvent* ev) override;
    // 右键菜单
    void contextMenuEvent(QContextMenuEvent* event) override;

    // 显示预览视频窗口
    void View(int count);

    // 刷新左侧相机列表
    void RefreshCams();

    // 编辑摄像机
    void SetCam(int index);

    // 定时器渲染视频 回调函数
    void timerEvent(QTimerEvent* ev) override;

public slots:
    void MaxWindow();
    void NormalWindow();
    void View1();
    void View4();
    void View9();
    void View16();
    void AddCam(); // 新增摄像机配置
    void SetCam();
    void DelCam();
private:
    Ui::XViewerClass *ui;
    QMenu left_menu_;
};
