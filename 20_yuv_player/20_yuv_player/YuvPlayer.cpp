#include "YuvPlayer.h"

YuvPlayer::YuvPlayer(QObject *parent)
	: Widget(parent)
{}

YuvPlayer::YuvPlayer(QWidget* parent)
{
}

YuvPlayer::~YuvPlayer()
{}

void YuvPlayer::play()
{
}

void YuvPlayer::pause()
{
}

void YuvPlayer::stop()
{
}

bool YuvPlayer::isPlaying()
{
	return false;
}

void YuvPlayer::setYUV(Yuv& yuv)
{
}

State YuvPlayer::getState()
{
	return State();
}

void YuvPlayer::timerEvent(QTimerEvent* event)
{
}
