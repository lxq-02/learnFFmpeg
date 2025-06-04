#include "xviewer.h"
#include <QLineEdit>
#include <QGridLayout>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QDebug>
#include <QContextMenuEvent>
#include <QFormLayout>
#include <QDialog>
#include <QMessageBox>
#include <sstream>
#include "xcamera_config.h"
#include "xcamera_widget.h"

#define CAM_CONF_PATH "cams.db"

// 解决中文乱码
//#define C(s) QString::fromLocal8Bit(s)
#define C(s) safeDecode(s)
QString safeDecode(const char* s)
{
    QString str = QString::fromUtf8(s);
    if (str.contains(QChar::ReplacementCharacter)) {
        // UTF-8 失败了，用本地编码
        str = QString::fromLocal8Bit(s);
    }
    return str;
}


static XCameraWidget* cam_wids[16] = { 0 };


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

    // 刷新左侧摄像机列表
    XCameraConfig::Instance()->Load(CAM_CONF_PATH);
    RefreshCams();
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
            cam_wids[i] = new XCameraWidget();
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

void XViewer::RefreshCams()
{
    auto c = XCameraConfig::Instance();
    ui->cam_list->clear();
	int count = c->GetCamCount();
    for (int i = 0; i < count; i++)
    {
        auto cam = c->GetCam(i);
		auto item = new QListWidgetItem(QIcon(":/XViewer/img/cam.png"), C(cam.name_));
        ui->cam_list->addItem(item);
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

void XViewer::SetCam(int index)
{
    auto c = XCameraConfig::Instance();

    QDialog dlg(this);
    dlg.resize(800, 200);
    QFormLayout lay;
    dlg.setLayout(&lay);
    // 标题1 输入框1
    // 标题2 输入框2
    QLineEdit name_edit;
    lay.addRow(C("名称"), &name_edit);

    QLineEdit url_edit;
    lay.addRow(C("主码流"), &url_edit);

    QLineEdit sub_url_edit;
    lay.addRow(C("辅码流"), &sub_url_edit);

    QLineEdit save_path_edit;
    lay.addRow(C("保存路径"), &save_path_edit);

    QPushButton save;
    save.setText(C("保存"));

    connect(&save, &QPushButton::clicked, &dlg, &QDialog::accept);

    lay.addRow("", &save);

    // 编辑 读入原数据显示
    if (index > 0)
    {
        auto cam = c->GetCam(index);
		name_edit.setText(C(cam.name_));
		url_edit.setText(C(cam.url_));
		sub_url_edit.setText(C(cam.sub_url_));
		save_path_edit.setText(C(cam.save_path_));
    }


    for (;;)
    {
        if (dlg.exec() == QDialog::Accepted) // 点击保存按钮
        {
            // 检查输入是否为空
            if (name_edit.text().isEmpty()
                || url_edit.text().isEmpty()
                || sub_url_edit.text().isEmpty()
                || save_path_edit.text().isEmpty())
            {
                QMessageBox::information(0, C("错误"), C("请填写完整的摄像机信息"));
                continue; // 继续循环，重新显示对话框
            }

            break;
        }
        return;
    }

    XCameraData data;
    strncpy_s(data.name_, sizeof(data.name_), name_edit.text().toStdString().c_str(), _TRUNCATE);
    strncpy_s(data.url_, sizeof(data.url_), url_edit.text().toStdString().c_str(), _TRUNCATE);
    strncpy_s(data.sub_url_, sizeof(data.sub_url_), sub_url_edit.text().toStdString().c_str(), _TRUNCATE);
    strncpy_s(data.save_path_, sizeof(data.save_path_), save_path_edit.text().toStdString().c_str(), _TRUNCATE);

    if (index > 0) // 修改
    {
        c->SetCam(index, data);
    }
	else // 新增
	{
		c->Push(data);
	}
    c->Save(CAM_CONF_PATH); // 保存到文件
    RefreshCams();  // 刷新显示
}

void XViewer::AddCam()
{
	SetCam(-1); // -1表示新增摄像机
}

void XViewer::SetCam()
{
    int row = ui->cam_list->currentIndex().row();
    if (row < 0)
    {
		QMessageBox::information(this, "error", C("请选择一个摄像机进行编辑"));
		return;
    }
	SetCam(row); // 设置摄像机
}

void XViewer::DelCam()
{
    int row = ui->cam_list->currentIndex().row();
    if (row < 0)
    {
        QMessageBox::information(this, "error", C("请选择一个摄像机进行编辑"));
        return;
    }
    std::stringstream ss;
    ss << QStringLiteral("您确认删除摄像机").toUtf8().constData()
        << ui->cam_list->currentItem()->text().toUtf8().constData()
        << QStringLiteral("吗？").toUtf8().constData();

        
    if (QMessageBox::information(this, "confirm", C(ss.str().c_str()),
        QMessageBox::Yes,
        QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

	XCameraConfig::Instance()->DelCam(row);
    RefreshCams();
}
