#include "xcamera_config.h"
#include <fstream>
using namespace std;

// 插入摄像机，线程安全
void XCameraConfig::Push(const XCameraData& data)
{
	unique_lock<mutex> lock(mtx_);
	cams_.push_back(data);
}

XCameraData XCameraConfig::GetCam(int index)
{
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || index > cams_.size())
	{
		return XCameraData();
	}
	return cams_[index];
}

bool XCameraConfig::SetCam(int index, const XCameraData& data)
{
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || index > cams_.size())
	{
		return false;
	}
	cams_[index] = data;
	return true;
}

bool XCameraConfig::DelCam(int index)
{
	unique_lock<mutex> lock(mtx_);
	if (index < 0 || index > cams_.size())
	{
		return false;
	}
	cams_.erase(cams_.begin() + index);
	return true;
}

int XCameraConfig::GetCamCount()
{
	unique_lock<mutex> lock(mtx_);
	return cams_.size();
}

bool XCameraConfig::Save(const char* path)
{
	if (!path) return false;
	ofstream ofs(path, ios::binary);
	if (!ofs) return false;

	unique_lock<mutex> lock(mtx_);
	for (auto cam : cams_)
	{
		ofs.write((char*) & cam, sizeof(cam));
	}
	ofs.close();
	return true;
}

bool XCameraConfig::Load(const char* path)
{
	if (!path) return false;
	ifstream ifs(path, ios::binary);
	if (!ifs) return false;

	unique_lock<mutex> lock(mtx_);
	XCameraData data;
	cams_.clear();
	for (;;)
	{
		ifs.read((char*)&data, sizeof(data));
		if (ifs.gcount() != sizeof(data))
		{
			ifs.close();
			return true;
		}
		cams_.push_back(data);
	}
	ifs.close();
	return true;
}
