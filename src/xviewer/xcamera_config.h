#pragma once
#include <vector>
#include <mutex>

struct XCameraData
{
	char name_[1024] = { 0 };
	char url_[4096] = { 0 };		// �����������
	char sub_url_[4096] = { 0 };	// �����������
	char save_path_[4096] = { 0 };	// ��Ƶ¼�ƴ��Ŀ¼
};

class XCameraConfig
{
public:
	// Ψһ����ʵ�� ����ģʽ
	static XCameraConfig* Instance()
	{
		static XCameraConfig xc;
		return &xc;
	}

	// ���������
	void Push(const XCameraData& data);

	/**
	 * ��ȡ����ͷ
	 * 
	 * @param index ����ͷ����
	 * @return ���ؿն��󣬿����ж�nameΪ���ַ���
	 */
	XCameraData GetCam(int index);

	/**
	 * �޸����������
	 * 
	 * @param index
	 * @param data
	 * @return 
	 */
	bool SetCam(int index, const XCameraData& data);

	bool DelCam(int index);

	/**
	 * ��ȡ�������
	 * 
	 * @return ʧ�ܷ���0
	 */
	int GetCamCount();

	/**
	 * �洢�����ļ�
	 * 
	 * @param path
	 * @return 
	 */
	bool Save(const char* path);

	/**
	 * ��ȡ���ã�����cams_
	 * 
	 * @param path
	 * @return 
	 */
	bool Load(const char* path);

private:
	XCameraConfig() {}	// ����˽�У�����ģʽ
	std::vector<XCameraData> cams_;
	std::mutex mtx_;
};


