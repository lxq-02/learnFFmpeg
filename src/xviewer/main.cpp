#include "xviewer.h"
#include <iostream>
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QDir>
#include "xcamera_record.h"

#include "xcamera_config.h"
#include "xplayvideo.h"
using namespace std;

#define TEST_CAM_PATH "test.db"


int main(int argc, char *argv[])
{
    const char* save_path = "./video/0";
    QDir dir;
	dir.mkpath(save_path); // 确保目录存在

	//XCameraRecord xr;
 //   xr.set_rtsp_url("rtsp://127.0.0.1/test");
	//xr.set_save_path(save_path);
 //   xr.Start();


    /*auto* xc = XCameraConfig::Instance();
    xc->Load(TEST_CAM_PATH);
    {
        XCameraData cd;
        const char* camera = "camera1";
        const char* path = ".\\camera1\\";
        const char* url = "rtsp://127.0.0.1:/test";
        const char* sub_url = url;
        memcpy(cd.name_, camera, strlen(camera) + 1);
        memcpy(cd.save_path_, path, strlen(path) + 1);
        memcpy(cd.url_, url, strlen(url) + 1);
        memcpy(cd.sub_url_, sub_url, strlen(sub_url) + 1);

        xc->Push(cd);
    }
    {
        XCameraData cd;
        const char* camera = "camera2";
        const char* path = ".\\camera2\\";
        const char* url = "rtsp://127.0.0.1:/test";
        const char* sub_url = url;
        memcpy(cd.name_, camera, strlen(camera) + 1);
        memcpy(cd.save_path_, path, strlen(path) + 1);
        memcpy(cd.url_, url, strlen(url) + 1);
        memcpy(cd.sub_url_, sub_url, strlen(sub_url) + 1);

        xc->Push(cd);
    }

    int cam_count = xc->GetCamCount();
    for (int i = 0; i < cam_count; ++i)
    {
        auto cam = xc->GetCam(i);
        qDebug() << cam.name_ ;
    }
    qDebug() << "-----------------Set-----------------------";

    auto d1 = xc->GetCam(0);
    memcpy(d1.name_, "camera_001", strlen("camera_001") + 1);
    xc->SetCam(0, d1);
    
    xc->Save(TEST_CAM_PATH);

    cam_count = xc->GetCamCount();
    for (int i = 0; i < cam_count; ++i)
    {
        auto cam = xc->GetCam(i);
        qDebug() << cam.name_;
    }
    qDebug() << "-----------------Del-----------------------";
    xc->DelCam(1);
    cam_count = xc->GetCamCount();
    for (int i = 0; i < cam_count; ++i)
    {
        auto cam = xc->GetCam(i);
        qDebug() << cam.name_;
    }*/
    QApplication a(argc, argv);
    XPlayVideo pv;
    pv.Open("v1080.mp4");
    pv.exec();
    return 0;
    XViewer w;
    w.show();
    auto re = a.exec();
    //xr.Stop();
    return re;
}
