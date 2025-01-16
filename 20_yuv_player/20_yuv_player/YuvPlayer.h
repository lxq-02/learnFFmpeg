#pragma once

#include <QWidget>
#include <SDL.h>
#include <QFile>

extern "C"
{
#include <libavutil/avutil.h>
}

typedef struct {
	const char* filename;
	int width;
	int height;
	AVPixelFormat pixelFormat;
	int fps;
}Yuv;

class YuvPlayer  : public QWidget
{
	Q_OBJECT
public:
	// ״̬
	typedef enum {
		Stopped = 0,
		Playing,
		Paused,
		Finished
	} State;

public:
	YuvPlayer(QWidget *parent = nullptr);
	~YuvPlayer();

	void play();
	void pause();
	void stop();
	bool isPlaying();

	void setYUV(Yuv& yuv);

	State getState();

signals:

private:
	SDL_Window* _window = nullptr;
	SDL_Renderer* _renderer = nullptr;
	SDL_Texture* _texture = nullptr;
	QFile _file;
	int _timerId = 0;
	State _state = Stopped;
	Yuv _yuv;
	bool _playing;

	void timerEvent(QTimerEvent* event);

};
