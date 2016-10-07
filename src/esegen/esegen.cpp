// esegen.cpp
//
// esegen(1) Timecode Generator
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

#include <poll.h>

#include <QCoreApplication>

#include "cmdswitch.h"
#include "esegen.h"

void *AlsaCallback(void *ptr)
{

  MainObject *obj=(MainObject *)ptr;

  while(1==1) {
    snd_pcm_writei(obj->ese_pcm,obj->ese_pcm_buffer,obj->ese_buffer_size);
  }

  return NULL;
}

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  QString err_msg;
  QDateTime now=QDateTime::currentDateTime();
  struct pollfd fds;

  new CmdSwitch(qApp->argc(),qApp->argv(),"esegen",ESEGEN_USAGE);

  if(!StartSound(&err_msg,"hw:0")) {
    fprintf(stderr,"esegen: %s\n",(const char *)err_msg.toUtf8());
    exit(256);
  }

  memset(&fds,0,sizeof(fds));
  while(1==1) {
    poll(&fds,0,now.msecsTo(NextTick(now)));
    WritePacket(QDateTime::currentDateTime());
  }
}


void MainObject::WritePacket(const QDateTime &dt)
{
  memset(ese_pcm_buffer,0,ese_buffer_size*sizeof(int32_t));
  char str[256];

  //
  // Time Part
  //
  unsigned ptr=0;
  snprintf(str,256,"%s",(const char *)dt.toString("hhmmss ap").toUtf8());
  MakeSync(false,(int32_t *)ese_pcm_buffer,&ptr);
  for(int i=0;i<6;i++) {
    MakeDigit((0xff&str[i])-'0',(int32_t *)ese_pcm_buffer,&ptr);
  }

  //
  // Date Part
  //
  ptr=ese_samplerate*ese_channels*ESE_PACKET_LENGTH/2.0;
  snprintf(str,256,"%s",(const char *)dt.toString("MMddyy").toUtf8());
  MakeSync(true,(int32_t *)ese_pcm_buffer,&ptr);
  for(int i=0;i<6;i++) {
    MakeDigit((0xff&str[i])-'0',(int32_t *)ese_pcm_buffer,&ptr);
  }
}


void MainObject::MakeSync(bool is_date,int32_t *buffer,unsigned *ptr)
{
  for(int i=0;i<2;i++) {
    if(is_date) {
      MakeOne(buffer,ptr);
    }
    else {
      MakeZero(buffer,ptr);
    }
  }
}


void MainObject::MakeDigit(int digit,int32_t *buffer,unsigned *ptr)
{
  for(int i=0;i<4;i++) {
    if((digit&(1<<(3-i)))!=0) {
      MakeOne(buffer,ptr);
    }
    else {
      MakeZero(buffer,ptr);
    }
  }
}


void MainObject::MakeOne(int32_t *buffer,unsigned *ptr)
{
  for(unsigned i=0;i<((float)ese_samplerate*ESE_SLOT_LENGTH/2.0);i++) {
    for(unsigned j=0;j<ese_channels;j++) {
      buffer[*ptr+i*ese_channels+j]=ESE_ON_LEVEL;
    }
  }
  *ptr+=ESE_SLOT_LENGTH*ese_channels*ese_samplerate;
}


void MainObject::MakeZero(int32_t *buffer,unsigned *ptr)
{
  for(unsigned i=0;i<((float)ese_samplerate*ESE_SLOT_LENGTH/4.0);i++) {
    for(unsigned j=0;j<ese_channels;j++) {
      buffer[*ptr+i*ese_channels+j]=ESE_ON_LEVEL;
    }
  }
  *ptr+=ESE_SLOT_LENGTH*ese_channels*ese_samplerate;
}


QDateTime MainObject::NextTick(const QDateTime &dt) const
{
  QDateTime ret=dt.addSecs(1);
  ret=QDateTime(ret.date(),QTime(ret.time().hour(),ret.time().minute(),ret.time().second(),0));

  return ret;
}


bool MainObject::StartSound(QString *err_msg,const QString &dev)
{
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int dir;
  int aerr;
  pthread_attr_t pthread_attr;

  if(snd_pcm_open(&ese_pcm,dev.toUtf8(),
		  SND_PCM_STREAM_PLAYBACK,0)!=0) {
    *err_msg=tr("unable to open ALSA device")+" \""+dev+"\"";
    return false;
  }
  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_hw_params_any(ese_pcm,hwparams);

  //
  // Access Type
  //
  if(snd_pcm_hw_params_test_access(ese_pcm,hwparams,
				   SND_PCM_ACCESS_RW_INTERLEAVED)<0) {
    *err_msg=tr("interleaved access not supported");
    return false;
  }
  snd_pcm_hw_params_set_access(ese_pcm,hwparams,SND_PCM_ACCESS_RW_INTERLEAVED);

  //
  // Sample Format
  //
  if(snd_pcm_hw_params_set_format(ese_pcm,hwparams,
				  SND_PCM_FORMAT_S32_LE)==0) {
    ese_format=SND_PCM_FORMAT_S32_LE;
  }
  else {
    if(snd_pcm_hw_params_set_format(ese_pcm,hwparams,
				    SND_PCM_FORMAT_S16_LE)==0) {
      ese_format=SND_PCM_FORMAT_S16_LE;
    }
    else {
      *err_msg=tr("incompatible sample format");
      return false;
    }
  }

  //
  // Sample Rate
  //
  ese_samplerate=192000;
  snd_pcm_hw_params_set_rate_near(ese_pcm,hwparams,&ese_samplerate,&dir);

  //
  // Channels
  //
  ese_channels=1;
  snd_pcm_hw_params_set_channels_near(ese_pcm,hwparams,&ese_channels);

  //
  // Buffer Parameters
  //
//  ese_period_quantity=ALSA_PERIOD_QUANTITY;
  ese_period_quantity=4;
  snd_pcm_hw_params_set_periods_near(ese_pcm,hwparams,&ese_period_quantity,
				     &dir);
  //  ese_buffer_size=ese_samplerate/2;
  ese_buffer_size=ese_samplerate*0.0334;
  snd_pcm_hw_params_set_buffer_size_near(ese_pcm,hwparams,&ese_buffer_size);

  //
  // Fire It Up
  //
  if((aerr=snd_pcm_hw_params(ese_pcm,hwparams))<0) {
    *err_msg=tr("ALSA device error 1")+": "+snd_strerror(aerr);
    return false;
  }

  switch(ese_format) {
  case SND_PCM_FORMAT_S32_LE:
    ese_pcm_buffer=new int32_t[ese_buffer_size*ese_channels];
    break;

  case SND_PCM_FORMAT_S16_LE:
    ese_pcm_buffer=new int16_t[ese_buffer_size*ese_channels];
    break;

  default:
    *err_msg=tr("inconsistent PCM format");
    return false;
  }

  //
  // Set Wake-up Timing
  //
  snd_pcm_sw_params_alloca(&swparams);
  snd_pcm_sw_params_current(ese_pcm,swparams);
  snd_pcm_sw_params_set_avail_min(ese_pcm,swparams,ese_buffer_size/2);
  if((aerr=snd_pcm_sw_params(ese_pcm,swparams))<0) {
    *err_msg=tr("ALSA device error 2")+": "+snd_strerror(aerr);
    return false;
  }

  //
  // Start the Callback
  //
  pthread_attr_init(&pthread_attr);
  pthread_attr_setschedpolicy(&pthread_attr,SCHED_FIFO);
  pthread_create(&ese_pthread,&pthread_attr,AlsaCallback,this);

  return true;
}

/*
void MainObject::MakeSquarewave(void *buffer,unsigned len) const
{
  int16_t sign16=1;
  int32_t sign32=1;

  printf("BUFFER IN: %p\n",(int32_t *)buffer);

  switch(ese_format) {
  case SND_PCM_FORMAT_S16_LE:
    for(unsigned i=0;i<len;i+=33) {
      for(unsigned j=0;j<33;j++) {
	((int16_t *)buffer)[i+j]=sign16*0xFFFF;
      }
      sign16=-sign16;
    }
    break;

  case SND_PCM_FORMAT_S32_LE:
    for(unsigned i=0;i<len;i+=3) {
      for(unsigned j=0;j<3;j++) {
	((int16_t *)buffer)[i+j]=sign32*1000000000;
      }
      sign32=-sign32;
    }
    break;
  }
}
*/

int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
