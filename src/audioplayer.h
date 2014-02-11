// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "main.h"

#ifdef USE_QTMULTIMEDIA
#include <QAudioOutput>
#include <QObject>
#include <QTimer>

class Generator : public QIODevice
{
	Q_OBJECT
public:
	Generator(const QAudioFormat &format, qint64 durationUs, int frequency, QObject *parent);
	~Generator();

	void start();
	void stop();

	qint64 readData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);
	qint64 bytesAvailable() const;

private:
	void generateData(const QAudioFormat &format, qint64 durationUs, int frequency);

private:
	qint64 m_pos;
	QByteArray m_buffer;
};

class AudioPlayer : public QObject
{
	Q_OBJECT

public:
	bool invalidDevice;
	void beep();
	AudioPlayer(QObject *parent);
	~AudioPlayer();

private:
	QByteArray       m_buffer;
	QTimer*          m_timeOutTimer;
	QAudioDeviceInfo m_device;
	Generator*       m_generator;
	QAudioOutput*    m_audioOutput;
	QIODevice*       m_output; // not owned
	QAudioFormat     m_format;
private slots:
	void pullTimerExpired();
};

#endif
#endif // AUDIOPLAYER_H
