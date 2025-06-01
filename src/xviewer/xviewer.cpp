#include "xviewer.h"
#include <QGridLayout>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QDebug>
#include <QContextMenuEvent>

// 解决中文乱码
#define C(s) QString::fromLocal8Bit(s)

static QWidget* cam_wids[16] = { 0 };


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

    ///////////////////////////////////////////
    /// 初始化右键菜单
    // 视图=》 1 窗口
    //         4 窗口
    auto m = left_menu_.addMenu(C("视图"));
    auto a = m->addAction(C("1窗口"));
    connect(a, SIGNAL(triggered()), this, SLOT(View1()));
    a = m->addAction(C("4窗口"));
    connect(a, SIGNAL(triggered()), this, SLOT(View4()));
    a = m->addAction(C("9窗口"));
    connect(a, SIGNAL(triggered()), this, SLOT(View9()));
    a = m->addAction(C("16窗口"));
    connect(a, SIGNAL(triggered()), this, SLOT(View16()));

    // 默认9窗口
    View9();
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

void XViewer::contextMenuEvent(QContextMenuEvent* event)
{
    // 鼠标位置显示右键菜单
    left_menu_.exec(QCursor::pos());
    event->accept(); 
}

void XViewer::View(int count)
{
    qDebug() << "View" << count;
    // 2x2 3x3 4x4
    // 确定列数
    int cols = sqrt(count);
    // 总窗口数量
    int wid_size = sizeof(cam_wids) / sizeof(QWidget*);
    
    // 初始化布局器
    auto lay = (QGridLayout*)ui->cams->layout();
    if (!lay)
    {
        lay = new QGridLayout();
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(2);     // 元素间距
        ui->cams->setLayout(lay);
    }
    // 初始化窗口
    for (int i = 0; i < count; ++i)
    {
        if (!cam_wids[i])
        {
            cam_wids[i] = new QWidget();
            cam_wids[i]->setStyleSheet("background-color:rgb(51,51,51);");
        }
        lay->addWidget(cam_wids[i], i/cols, i%cols);
    }

    // 清理多余的窗体
    for (int i = count; i < wid_size; ++i)
    {
        if (cam_wids[i])
        {
            delete cam_wids[i];
            cam_wids[i] = nullptr;
        }
    }
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

void XViewer::View1()
{
    View(1);
}

void XViewer::View4()
{
    View(4);
}

void XViewer::View9()
{
    View(9);
}

void XViewer::View16()
{
    View(16);
}
