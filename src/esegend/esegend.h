// esegend.h
//
// esegend(8) Timecode Generator
//
//   (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef ESEGEND_H
#define ESEGEND_H

#include <stdint.h>

#include <alsa/asoundlib.h>
#include <pthread.h>

#include <QDateTime>
#include <QObject>

#define ESE_PACKET_LENGTH 0.0338
#define ESE_SLOT_LENGTH 0.000416
#define ESE_ON_LEVEL 2147483647
#define ESE_OFF_LEVEL 0
#define ESEGEND_USAGE "\n"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private:
  void WritePacket(const QDateTime &dt);
  void MakeSync(bool is_date,int32_t *buffer,unsigned *ptr);
  void MakeDigit(int digit,int32_t *buffer,unsigned *ptr);
  void MakeOne(int32_t *buffer,unsigned *ptr);
  void MakeZero(int32_t *buffer,unsigned *ptr);
  QDateTime NextTick(const QDateTime &dt) const;
  bool StartSound(QString *err_msg,const QString &dev);
  void Log(const QString &msg);
  snd_pcm_t *ese_pcm;
  unsigned ese_format;
  unsigned ese_samplerate;
  unsigned ese_channels;
  unsigned ese_period_quantity;
  snd_pcm_uframes_t ese_buffer_size; 
  void *ese_pcm_buffer;
  pthread_t ese_pthread;
  bool ese_debug;
  friend void *AlsaCallback(void *ptr);
};


#endif  // ESEGEND_H
