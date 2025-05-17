#pragma once

#include <QThread>
#include <SDL.h>

class PlayThread  : public QThread
{
	Q_OBJECT

public:
	PlayThread(QObject *parent);
	~PlayThread();

private:
	void run();
	SDL_Texture* createTexture(SDL_Renderer* renderer);
	void showClick(SDL_Event& event,
		SDL_Renderer* renderer,
		SDL_Texture* texture);
};
