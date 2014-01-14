//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "audioplayer.h"
#ifdef USE_QTMULTIMEDIA
#include <QtCore/qmath.h>
#include <QtCore/qendian.h>
#include <QAudioOutput>
#include <QAudioDeviceInfo>

const int DurationMSeconds = 50;
const int ToneFrequencyHz = 800;
const int DataFrequencyHz = 44100;
const int BufferSize      = 32768;

AudioPlayer::AudioPlayer(QObject *parent)
	: QObject(parent)
{
	invalidDevice=QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).count()==0;
	if(invalidDevice)return;
	m_timeOutTimer=new QTimer(this);
	m_timeOutTimer->setSingleShot(true);
	m_device=QAudioDeviceInfo::defaultOutputDevice();
	m_generator=0;
	m_audioOutput=0;
	m_output=0;
	m_buffer=QByteArray(BufferSize, 0);

	m_format.setFrequency(DataFrequencyHz);
	m_format.setChannels(1);
	m_format.setSampleSize(16);
	m_format.setCodec("audio/pcm");
	m_format.setByteOrder(QAudioFormat::LittleEndian);
	m_format.setSampleType(QAudioFormat::SignedInt);

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (!info.isFormatSupported(m_format))	m_format = info.nearestFormat(m_format);

	connect(m_timeOutTimer, SIGNAL(timeout()), this, SLOT(pullTimerExpired()));
	m_generator = new Generator(m_format, DurationMSeconds*1000, ToneFrequencyHz, this);

	m_audioOutput=new QAudioOutput(m_device, m_format, this);
}

AudioPlayer::~AudioPlayer()
{
	if(!invalidDevice)m_audioOutput->stop();
}

void AudioPlayer::pullTimerExpired()
{
	m_generator->stop();
}

void AudioPlayer::beep()
{
	pullTimerExpired();
	m_generator->start();
	m_audioOutput->start(m_generator);
	m_timeOutTimer->start(100);
}

Generator::Generator(const QAudioFormat &format,
	qint64 durationUs,
	int frequency,
	QObject *parent)
	:   QIODevice(parent)
	,   m_pos(0)
{
	generateData(format, durationUs, frequency);
}

Generator::~Generator()
{

}

void Generator::start()
{
	open(QIODevice::ReadOnly);
}

void Generator::stop()
{
	m_pos = 0;
	close();
}

void Generator::generateData(const QAudioFormat &format, qint64 durationUs, int frequency)
{
	const int channelBytes = format.sampleSize() / 8;
	const int sampleBytes = format.channels() * channelBytes;

	qint64 length = (format.frequency() * format.channels() * (format.sampleSize() / 8))
		* durationUs / 100000;

	Q_ASSERT(length % sampleBytes == 0);
	Q_UNUSED(sampleBytes) // suppress warning in release builds

		m_buffer.resize(length);
	unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
	int sampleIndex = 0;

	while (length) {
		const qreal x = qSin(2 * M_PI * frequency * qreal(sampleIndex % format.frequency()) / format.frequency());
		for (int i=0; i<format.channels(); ++i) {
			if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
				const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
				*reinterpret_cast<quint8*>(ptr) = value;
			} else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
				const qint8 value = static_cast<qint8>(x * 127);
				*reinterpret_cast<quint8*>(ptr) = value;
			} else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
				quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
				if (format.byteOrder() == QAudioFormat::LittleEndian)
					qToLittleEndian<quint16>(value, ptr);
				else
					qToBigEndian<quint16>(value, ptr);
			} else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
				qint16 value = static_cast<qint16>(x * 32767);
				if (format.byteOrder() == QAudioFormat::LittleEndian)
					qToLittleEndian<qint16>(value, ptr);
				else
					qToBigEndian<qint16>(value, ptr);
			}

			ptr += channelBytes;
			length -= channelBytes;
		}
		++sampleIndex;
	}
}

qint64 Generator::readData(char *data, qint64 len)
{
	qint64 total = 0;
	while (len - total > 0) {
		const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
		memcpy(data + total, m_buffer.constData() + m_pos, chunk);
		m_pos = (m_pos + chunk) % m_buffer.size();
		total += chunk;
	}
	return total;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data);
	Q_UNUSED(len);

	return 0;
}

qint64 Generator::bytesAvailable() const
{
	return m_buffer.size() + QIODevice::bytesAvailable();
}
#endif
