#pragma once
#include <vector>
#include <mutex>

struct XCameraData
{
	char name_[1024] = { 0 };
	char url_[4096] = { 0 };		// 摄像机主码流
	char sub_url_[4096] = { 0 };	// 摄像机辅码流
	char save_path_[4096] = { 0 };	// 视频录制存放目录
};

class XCameraConfig
{
public:
	// 唯一对象实例 单例模式
	static XCameraConfig* Instance()
	{
		static XCameraConfig xc;
		return &xc;
	}

	// 插入摄像机
	void Push(const XCameraData& data);

	/**
	 * 获取摄像头
	 * 
	 * @param index 摄像头索引
	 * @return 返回空对象，可以判断name为空字符串
	 */
	XCameraData GetCam(int index);

	/**
	 * 修改摄像机数据
	 * 
	 * @param index
	 * @param data
	 * @return 
	 */
	bool SetCam(int index, const XCameraData& data);

	bool DelCam(int index);

	/**
	 * 获取相机数量
	 * 
	 * @return 失败返回0
	 */
	int GetCamCount();

	/**
	 * 存储配置文件
	 * 
	 * @param path
	 * @return 
	 */
	bool Save(const char* path);

	/**
	 * 读取配置，存入cams_
	 * 
	 * @param path
	 * @return 
	 */
	bool Load(const char* path);

private:
	XCameraConfig() {}	// 构造私有，单例模式
	std::vector<XCameraData> cams_;
	std::mutex mtx_;
};


