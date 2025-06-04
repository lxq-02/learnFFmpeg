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
#include <QDir>
#include <vector>
#include <map>
#include "xcamera_config.h"
#include "xcamera_widget.h"
#include "xcamera_record.h"
#include "xplayvideo.h"
using namespace std;

#define CAM_CONF_PATH "cams.db"

// �����������
//#define C(s) QString::fromLocal8Bit(s)
#define C(s) safeDecode(s)
QString safeDecode(const char* s)
{
    QString str = QString::fromUtf8(s);
    if (str.contains(QChar::ReplacementCharacter)) {
        // UTF-8 ʧ���ˣ��ñ��ر���
        str = QString::fromLocal8Bit(s);
    }
    return str;
}

// ��Ⱦ����
static XCameraWidget* cam_wids[16] = { 0 };

// ��Ƶ¼��
static std::vector<XCameraRecord*> records;

// �洢��Ƶ����ʱ��
struct XCamVideo
{
    QString filepath;
    QDateTime datetime;
};
static map<QDate, vector<XCamVideo>> cam_videos;

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
    hlay->addWidget(ui->left);      // �������б�
    hlay->addWidget(ui->cams);      // �Ҳ�Ԥ������
    hlay->addWidget(ui->playback_wid);  // �طŴ���

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

    a = left_menu_.addAction(C("ȫ����ʼ¼��"));
	connect(a, SIGNAL(triggered()), this, SLOT(StartRecord()));
	a = left_menu_.addAction(C("ȫ��ֹͣ¼��"));
	connect(a, SIGNAL(triggered()), this, SLOT(StopRecord()));

    // Ĭ��9����
    View9();

    // ˢ�����������б�
    XCameraConfig::Instance()->Load(CAM_CONF_PATH);
    ui->time_list->clear();

    RefreshCams();

    // ������ʱ����Ⱦ��Ƶ
    startTimer(1);
    Preview(); // Ĭ����ʾԤ��
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
            cam_wids[i] = new XCameraWidget();
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
    // ����1 �����1
    // ����2 �����2
    QLineEdit name_edit;
    lay.addRow(C("����"), &name_edit);

    QLineEdit url_edit;
    lay.addRow(C("������"), &url_edit);

    QLineEdit sub_url_edit;
    lay.addRow(C("������"), &sub_url_edit);

    QLineEdit save_path_edit;
    lay.addRow(C("����·��"), &save_path_edit);

    QPushButton save;
    save.setText(C("����"));

    connect(&save, &QPushButton::clicked, &dlg, &QDialog::accept);

    lay.addRow("", &save);

    // �༭ ����ԭ������ʾ
    if (index >= 0)
    {
        auto cam = c->GetCam(index);
		name_edit.setText(C(cam.name_));
		url_edit.setText(C(cam.url_));
		sub_url_edit.setText(C(cam.sub_url_));
		save_path_edit.setText(C(cam.save_path_));
    }


    for (;;)
    {
        if (dlg.exec() == QDialog::Accepted) // ������水ť
        {
            // ��������Ƿ�Ϊ��
            if (name_edit.text().isEmpty()
                || url_edit.text().isEmpty()
                || sub_url_edit.text().isEmpty()
                || save_path_edit.text().isEmpty())
            {
                QMessageBox::information(0, C("����"), C("����д�������������Ϣ"));
                continue; // ����ѭ����������ʾ�Ի���
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

    if (index >= 0) // �޸�
    {
        c->SetCam(index, data);
    }
	else // ����
	{
		c->Push(data);
	}
    c->Save(CAM_CONF_PATH); // ���浽�ļ�
    RefreshCams();  // ˢ����ʾ
}

void XViewer::timerEvent(QTimerEvent* ev)
{
    // �ܴ�������
	int wid_size = sizeof(cam_wids) / sizeof(QWidget*);
    for (int i = 0; i < wid_size; ++i)
    {
        if (cam_wids[i])
        {
            // ��Ⱦ�ര����Ƶ
			cam_wids[i]->Draw();
        }
    }
}

void XViewer::AddCam()
{
	SetCam(-1); // -1��ʾ���������
}

void XViewer::SetCam()
{
    int row = ui->cam_list->currentIndex().row();
    if (row < 0)
    {
		QMessageBox::information(this, "error", C("��ѡ��һ����������б༭"));
		return;
    }
	SetCam(row); // ���������
}

void XViewer::DelCam()
{
    int row = ui->cam_list->currentIndex().row();
    if (row < 0)
    {
        QMessageBox::information(this, "error", C("��ѡ��һ����������б༭"));
        return;
    }
    std::stringstream ss;
    ss << QStringLiteral("��ȷ��ɾ�������").toUtf8().constData()
        << ui->cam_list->currentItem()->text().toUtf8().constData()
        << QStringLiteral("��").toUtf8().constData();

        
    if (QMessageBox::information(this, "confirm", C(ss.str().c_str()),
        QMessageBox::Yes,
        QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

	XCameraConfig::Instance()->DelCam(row);
    RefreshCams();
}

void XViewer::StartRecord()
{
    StopRecord();

    qDebug() << "��ʼȫ������ͷ¼��";
    ui->status->setText(C("¼����..."));
    // ��ȡ�����б�
    auto conf = XCameraConfig::Instance();
	int count = conf->GetCamCount();
    for (int i = 0; i < count; ++i)
    {
		auto cam = conf->GetCam(i); // ��ȡ���������
        std::stringstream ss;
        ss << cam.save_path_ << "/" << i << "/";
        QDir dir;
		dir.mkpath(ss.str().c_str()); // ����Ŀ¼
		XCameraRecord* rec = new XCameraRecord();
		rec->set_rtsp_url(cam.url_);
        rec->set_save_path(ss.str());
		rec->set_file_sec(10); // ÿ10��һ���ļ�
        rec->Start();
        records.push_back(rec);

    }
    // ����¼��Ŀ¼
    // �ֱ�ʼ¼���߳�
}

void XViewer::StopRecord()
{
    ui->status->setText(C("�����..."));
    for (auto rec : records)
    {
        rec->Stop();
        delete rec;
    }
    records.clear();
}

void XViewer::Preview()
{
	// Ԥ������
    ui->cams->show();
    ui->playback_wid->hide();
    ui->preview->setChecked(true);
}

void XViewer::Playback()
{
    ui->cams->hide();
    ui->playback_wid->show();
    ui->playback->setChecked(true);
}

void XViewer::SelectCamera(QModelIndex index)
{
	qDebug() << "SelectCamera" << index.row();
	auto conf = XCameraConfig::Instance();
	auto cam = conf->GetCam(index.row()); // ��ȡ���������
	if (cam.name_[0] == '\0') // �������Ϊ��
	{
		return;
	}
    // �����Ƶ�洢·��
    stringstream ss;
	ss << cam.save_path_ << "/" << index.row() << "/";

    // ������Ŀ¼
    QDir dir(C(ss.str().c_str())); // ȷ��Ŀ¼����
	if (!dir.exists())
	{
		return;
	}
    // ��ȡĿ¼���ļ��б�
    QStringList filters;
    filters << "*.mp4" << "*.avi";
    dir.setNameFilters(filters);    // ɸѡ

    // �����������������
    ui->cal->ClearDate();
    cam_videos.clear();

    // �����ļ��б�
    auto files = dir.entryInfoList();
    for (auto file : files)
    {
        //cam_2025_06_05_00_12_04.mp4
        QString filename = file.fileName();
        
        // ȥ��cam_ ��.mp4
		auto tmp = filename.left(filename.lastIndexOf('.'));
        tmp = tmp.right(tmp.length() - 4);

        auto dt = QDateTime::fromString(tmp, "yyyy_MM_dd_hh_mm_ss");
        qDebug() << dt.date();
        ui->cal->AddDate(dt.date());
        //qDebug() << file.fileName();

        XCamVideo video;
        video.datetime = dt;
        video.filepath = file.absoluteFilePath();
		cam_videos[dt.date()].push_back(video); // �洢��Ƶ��Ϣ
    }

    // ������ʾ����
    ui->cal->showNextMonth();
    ui->cal->showPreviousMonth();
}

void XViewer::SelectDate(QDate date)
{
    qDebug() << "SelectDate" << date.toString();
    auto dates = cam_videos[date];
    ui->time_list->clear();
    for (auto date : dates)
    {
        auto item = new QListWidgetItem(date.datetime.time().toString());

        //item ����Զ������� �ļ�·��
		item->setData(Qt::UserRole, QVariant(date.filepath));
        ui->time_list->addItem(item);
    }
}

void XViewer::PlayVideo(QModelIndex index)
{
    qDebug() << "PlayVideo" << index.row();
    auto item = ui->time_list->currentItem();
    if (!item) return;
	QString path = item->data(Qt::UserRole).toString();
    qDebug() << path;
    static XPlayVideo play;
	play.Open(path.toStdString().c_str());
    play.show();
}
