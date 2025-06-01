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
    // ����¼� �����϶�����
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

    // ���ڴ�С�����仯
    void resizeEvent(QResizeEvent* ev) override;
public slots:
    void MaxWindow();
    void NormalWindow();

private:
    Ui::XViewerClass *ui;
};
