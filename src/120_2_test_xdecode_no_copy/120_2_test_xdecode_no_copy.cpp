#include <iostream>
#include <string>
#include <fstream>
#include "XVideoView.h"
#include "xdecode.h"
#include <windows.h>
#include <d3d9.h>
using namespace std;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

struct DXVA2DevicePriv
{
	HMODULE d3dlib;
	HMODULE dxva2lib;
	HANDLE device_handle;
	IDirect3D9* d3d9;
	IDirect3DDevice9* d3d9device;
};

void DrawFrame(AVFrame* frame, AVCodecContext* ctx)
{
	if (!frame->data[3] || !ctx) return;
	cout << "D" << flush;
	auto surface = (IDirect3DSurface9*)frame->data[3];
	auto _ctx = (AVHWDeviceContext*)ctx->hw_device_ctx->data;
	auto priv = (DXVA2DevicePriv*)_ctx->user_opaque;
	auto device = priv->d3d9device;
	static HWND hwnd = nullptr;
	static RECT viewport;
	if (!hwnd)
	{
		hwnd = CreateWindow(
			L"DX", L"Test DXVA",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			200, 200, frame->width, frame->height,
			0, 0, 0, 0
		);
		ShowWindow(hwnd, 1);
		UpdateWindow(hwnd);
		viewport.left = 0;
		viewport.right = frame->width;
		viewport.top = 0;
		viewport.bottom = frame->height;
	}

	// ������ʾ���ھ��
	device->Present(&viewport, &viewport, hwnd, nullptr); // ��ʾ����
	// ��̨�������
	static IDirect3DSurface9* back = nullptr;
	if (!back)
		device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back);
	device->StretchRect(surface, 0, back, &viewport, D3DTEXF_LINEAR);

}

int main(int argc, char* argv[])
{
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = DefWindowProc; // ��Ϣ����
	wc.lpszClassName = L"DX";
	RegisterClassEx(&wc);



	//auto view = XVideoView::Create();
	// �ָ�h264 ����AVPacket
	// ffmpeg -i test.mp4 -s 400x300 400_300_test.h264
	string filename = "400_300_test.h264";
	ifstream ifs(filename, ios::binary);
	if (!ifs) return -1;
	unsigned char inbuf[4096] = { 0 };

	AVCodecID codec_id = AV_CODEC_ID_H264;

	XDecode de;
	// ���ݱ���������������
	auto ctx = de.Create(codec_id, false);
	de.set_context(ctx);
	de.InitHW(); // ��ʼ��Ӳ������
	de.Open();
	
	// �ָ�������
	AVCodecParserContext* parser = av_parser_init(codec_id);
	auto pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	AVFrame* hw_frame = av_frame_alloc(); // Ӳ����ת����
	auto begin = NowMs();
	int count = 0; // ����ͳ��
	bool is_init_win = false; // �Ƿ��ʼ������

	while (!ifs.eof())
	{
		ifs.read((char*)inbuf, sizeof(inbuf));
		if (ifs.eof())
		{
			ifs.clear();
			ifs.seekg(0, ios::beg);
		}
		int data_size = ifs.gcount(); // ��ȡ���ֽ���
		if (data_size <= 0) break;

		auto data = inbuf; // ����ָ��
		while (data_size > 0)	// һ���ж�֡����
		{
			// ͨ��0001 �ض������AVPacket ����֡��С
			int ret = av_parser_parse2(parser, ctx,
				&pkt->data, &pkt->size, // ���
				data, data_size,		// ����
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
			);
			data += ret;
			data_size -= ret; // �Ѵ���
			if (pkt->size)
			{
				//cout << pkt->size << " bytes" << endl;

				// 4������packet�������߳�
				if (!de.Send(pkt))
				{
					break;
				}

				// 5����ȡ��֡��������
				while (de.Recv(frame, false))
				{
					///////////////////////
					/// ��һ֡��ʼ������
					if (!is_init_win)
					{	
						is_init_win = true;
						//view->Init(frame->width, frame->height, (XVideoView::Format)frame->format);
					}
					//view->DrawFrame(frame);
					DrawFrame(frame, ctx);

					count++;
					auto cur = NowMs();
					if (cur - begin > 1000) // һ���Ӽ���һ�� 
					{
						cout << "\nfps = " << count << endl;
						count = 0;
						begin = cur;
					}
				}
			}
		}
	}

	auto frames = de.End();
	for (auto f : frames)
	{
		//view->DrawFrame(f);
		av_frame_free(&f);
	}
	//// ȡ����������
	//int ret = avcodec_send_packet(ctx, nullptr);
	//while (ret >= 0)
	//{
	//	ret = avcodec_receive_frame(ctx, frame);
	//	if (ret < 0)
	//		break;
	//	cout << frame->format << "-" << flush;
	//}

	av_parser_close(parser);
	avcodec_free_context(&ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	return 0;
}
