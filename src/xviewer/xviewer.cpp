#include "xviewer.h"
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

XViewer::XViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::XViewerClass())
{
    ui->setupUi(this);
    // 去除原窗口边框
    setWindowFlags(Qt::FramelessWindowHint);

    // 布局head和body 垂直布局器
    QVBoxLayout* vlay = new QVBoxLayout();
    // 边框间距
    vlay->setContentsMargins(0, 0, 0, 0);
    // 元素间距
    vlay->setSpacing(0);
    vlay->addWidget(ui->head);
    vlay->addWidget(ui->body);
    this->setLayout(vlay);

    // 相机列表 和 相机预览
    // 水平布局器
    auto hlay = new QHBoxLayout();
    ui->body->setLayout(hlay);
    // 边框间距
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->addWidget(ui->left);
    hlay->addWidget(ui->cams);
    
}

XViewer::~XViewer()
{
    delete ui;
}

static bool mouse_press = false;
static QPoint mouse_point;

void XViewer::mouseMoveEvent(QMouseEvent* ev)
{
    if (!mouse_press)
    {
        QWidget::mouseMoveEvent(ev);
        return;
    }
    this->move(ev->globalPos() - mouse_point);
}

void XViewer::mousePressEvent(QMouseEvent* ev)
{
    if (ev->buttons() & Qt::LeftButton)
    {
        mouse_press = true;
        mouse_point = ev->pos();
    }
}

void XViewer::mouseReleaseEvent(QMouseEvent* ev)
{
    mouse_press = false;
}

void XViewer::resizeEvent(QResizeEvent* ev)
{
    int x = width() - ui->head_button->width();
    int y = ui->head_button->y();
    ui->head_button->move(x, y);
}

void XViewer::MaxWindow()
{
    ui->max->setVisible(false);
    ui->normal->setVisible(true);
    showMaximized();
}

void XViewer::NormalWindow()
{
    ui->max->setVisible(true);
    ui->normal->setVisible(false);
    showNormal();
}
