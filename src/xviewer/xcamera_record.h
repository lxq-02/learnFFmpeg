#pragma once
#include <string>
#include "xtools.h"
class XCameraRecord : public XThread
{
public:
	void set_rtsp_url(std::string url) {	rtsp_url = url;	}
	void set_save_path(std::string s) { save_path = s; }
	void set_file_sec(int s) { file_sec_ = s; }

private:
	void Main();

	std::string rtsp_url;
	std::string save_path; // 保存的根目录
	long long  file_sec_ = 10;		// 多少秒创建一个新文件
};

