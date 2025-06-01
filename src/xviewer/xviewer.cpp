#include "xviewer.h"
#include <QGridLayout>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QDebug>
#include <QContextMenuEvent>

// �����������
#define C(s) QString::fromLocal8Bit(s)

static QWidget* cam_wids[16] = { 0 };


XViewer::XViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::XViewerClass())
{
    ui->setupUi(this);
    // ȥ��ԭ���ڱ߿�
    setWindowFlags(Qt::FramelessWindowHint);

    // ����head��body ��ֱ������
    QVBoxLayout* vlay = new QVBoxLayout();
    // �߿���
    vlay->setContentsMargins(0, 0, 0, 0);
    // Ԫ�ؼ��
    vlay->setSpacing(0);
    vlay->addWidget(ui->head);
    vlay->addWidget(ui->body);
    this->setLayout(vlay);

    // ����б� �� ���Ԥ��
    // ˮƽ������
    auto hlay = new QHBoxLayout();
    ui->body->setLayout(hlay);
    // �߿���
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->addWidget(ui->left);
    hlay->addWidget(ui->cams);

    ///////////////////////////////////////////
    /// ��ʼ���Ҽ��˵�
    // ��ͼ=�� 1 ����
    //         4 ����
    auto m = left_menu_.addMenu(C("��ͼ"));
    auto a = m->addAction(C("1����"));
    connect(a, SIGNAL(triggered()), this, SLOT(View1()));
    a = m->addAction(C("4����"));
    connect(a, SIGNAL(triggered()), this, SLOT(View4()));
    a = m->addAction(C("9����"));
    connect(a, SIGNAL(triggered()), this, SLOT(View9()));
    a = m->addAction(C("16����"));
    connect(a, SIGNAL(triggered()), this, SLOT(View16()));

    // Ĭ��9����
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
    // ���λ����ʾ�Ҽ��˵�
    left_menu_.exec(QCursor::pos());
    event->accept(); 
}

void XViewer::View(int count)
{
    qDebug() << "View" << count;
    // 2x2 3x3 4x4
    // ȷ������
    int cols = sqrt(count);
    // �ܴ�������
    int wid_size = sizeof(cam_wids) / sizeof(QWidget*);
    
    // ��ʼ��������
    auto lay = (QGridLayout*)ui->cams->layout();
    if (!lay)
    {
        lay = new QGridLayout();
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(2);     // Ԫ�ؼ��
        ui->cams->setLayout(lay);
    }
    // ��ʼ������
    for (int i = 0; i < count; ++i)
    {
        if (!cam_wids[i])
        {
            cam_wids[i] = new QWidget();
            cam_wids[i]->setStyleSheet("background-color:rgb(51,51,51);");
        }
        lay->addWidget(cam_wids[i], i/cols, i%cols);
    }

    // �������Ĵ���
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
