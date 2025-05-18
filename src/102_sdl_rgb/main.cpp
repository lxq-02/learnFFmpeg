#include <iostream>
#include <SDL.h>

using namespace std;

#pragma comment(lib, "SDL2.lib")
#undef main

static int w = 800;
static int h = 600;

int main(int argc, char *argv[])
{
	// 1、初始化子系统
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}

	// 2、创建窗口
	SDL_Window* window = SDL_CreateWindow("test sdl",
		0,
		0,
		w,
		h,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}

	// 3、创建渲染器
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}

	// 4、创建纹理
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
		SDL_TEXTUREACCESS_STREAMING,	// 可加锁
		w, h);
	if (!texture)
	{
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}

	// 5、渲染逻辑
	// 存放图像的数据
	shared_ptr<unsigned char> rgb(new unsigned char[w * h * 4]);
	// 填充数据
	auto r = rgb.get();

	for (;;)
	{
		// 判断退出
		SDL_Event ev;
		SDL_WaitEventTimeout(&ev, 10);
		if (ev.type == SDL_QUIT)
		{
			SDL_DestroyWindow(window);
			break;
		}

		for (int j = 0; j < h; j++)
		{
			int b = j * w * 4;
			for (int i = 0; i < w * 4; i += 4)
			{
				// ARGB
				r[b + i] = 0;		// B
				r[b + i + 1] = 0;	// G
				r[b + i + 2] = 255;	// R
				r[b + i + 3] = 0;	// A
			}
		}

		// 5.1、将数据传入纹理
		SDL_UpdateTexture(texture, NULL, r, w * 4);

		// 5.2、清理屏幕
		SDL_RenderClear(renderer);

		// 5.3、复制纹理到渲染器
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = w;
		rect.h = h;
		SDL_RenderCopy(renderer, texture,
			NULL, // 原图位置和尺寸
			&rect  // 目标位置和尺寸
		);
		// 5.4、将渲染内容更新到屏幕上
		SDL_RenderPresent(renderer);
	}

	// 6、延迟退出
	SDL_Delay(3000);

	// 7、释放资源
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	


	return 0;
}
