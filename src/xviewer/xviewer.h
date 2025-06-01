#pragma once

#include <QtWidgets/QWidget>
#include <QMouseEvent>
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
public slots:
    void MaxWindow();
    void NormalWindow();

private:
    Ui::XViewerClass *ui;
};
