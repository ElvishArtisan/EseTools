// esegend.cpp
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

#include <QCoreApplication>

#include "cmdswitch.h"
#include "esegend.h"

MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  ese_debug=false;
  QString err_msg;
  QDateTime now;
  struct pollfd fds;

  CmdSwitch *cmd=
    new CmdSwitch("esegend",ESEGEND_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="-d") {
      ese_debug=true;
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      fprintf(stderr,"esegend: unknown option \"%s\"\n",
	      (const char *)cmd->key(i).toUtf8());
      exit(256);
    }
  }

  //
  // Load Configuration
  //
  ese_config=new Config();
  ese_config->load();

  //
  // Start PCM Interface
  //
  if(!StartSound(&err_msg,ese_config->alsaDevice())) {
    fprintf(stderr,"esegend: %s\n",(const char *)err_msg.toUtf8());
    exit(256);
  }

  memset(&fds,0,sizeof(fds));
  while(1==1) {
    now=QDateTime::currentDateTime();

    switch(ese_config->timecodeVersion()) {
    case Config::Tc90:
      WriteTc90Packet(QDateTime::currentDateTime().
		      addMSecs(ese_config->timeOffset()));
      break;

    case Config::Tc89:
      WriteTc89Packet(QDateTime::currentDateTime().
		      addMSecs(ese_config->timeOffset()));
      break;

    default:
      fprintf(stderr,"esegend: unknown timecode version\n");
      exit(256);
      break;
    }
    if(snd_pcm_state(ese_pcm)!=SND_PCM_STATE_RUNNING) {
      snd_pcm_drop(ese_pcm);
      snd_pcm_prepare(ese_pcm);
    }
    snd_pcm_writei(ese_pcm,ese_pcm_buffer,ese_buffer_size);
  }
}


void MainObject::WriteTc89Packet(const QDateTime &dt)
{
  QString str;
  unsigned hour_mode_code=0;

  //
  // Zero Packet
  //
  switch(ese_format) {
  case SND_PCM_FORMAT_S32_LE:
    for(unsigned i=0;i<(ese_buffer_size*ese_channels);i++) {
      ((int32_t *)ese_pcm_buffer)[i]=ESE_32BIT_OFF_LEVEL;
    }
    break;

  case SND_PCM_FORMAT_S16_LE:
    for(unsigned i=0;i<(ese_buffer_size*ese_channels);i++) {
      ((int16_t *)ese_pcm_buffer)[i]=ESE_16BIT_OFF_LEVEL;
    }
    break;
  }  

  //
  // Sync Header
  //
  unsigned ptr=0;
  if(ese_config->hourMode()==Config::Hour24) {
    hour_mode_code=0x40;
  }
  str=dt.toString("hhmmss ap");

  //
  // Sync Bit
  //
  MakeZero(&ptr);

  //
  // Hour
  //
  MakeHexDigit(str.left(2).toInt(NULL,16)+hour_mode_code,&ptr);

  //
  // Minute
  //
  MakeHexDigit(str.mid(2,2).toInt(NULL,16),&ptr);

  //
  // Second
  //
  MakeHexDigit(str.mid(4,2).toInt(NULL,16),&ptr);
}


void MainObject::WriteTc90Packet(const QDateTime &dt)
{
  char str[256];
  QString time_format="hhmmss";

  //
  // Zero Packet
  //
  switch(ese_format) {
  case SND_PCM_FORMAT_S32_LE:
    for(unsigned i=0;i<(ese_buffer_size*ese_channels);i++) {
      ((int32_t *)ese_pcm_buffer)[i]=ESE_32BIT_OFF_LEVEL;
    }
    break;

  case SND_PCM_FORMAT_S16_LE:
    for(unsigned i=0;i<(ese_buffer_size*ese_channels);i++) {
      ((int16_t *)ese_pcm_buffer)[i]=ESE_16BIT_OFF_LEVEL;
    }
    break;
  }  

  if(ese_config->hourMode()==Config::Hour12) {
    time_format="hhmmss ap";
  }

  //
  // Time Part
  //
  unsigned ptr=0;
  snprintf(str,256,"%s",(const char *)dt.toString(time_format).toUtf8());
  MakeSync(false,&ptr);
  for(int i=0;i<6;i++) {
    MakeBCDDigit((0xff&str[i])-'0',&ptr);
  }

  //
  // Date Part
  //
  ptr=ese_samplerate*ese_channels*ESE_PACKET_LENGTH/2.0;
  snprintf(str,256,"%s",(const char *)dt.toString("MMddyy").toUtf8());
  MakeSync(true,&ptr);
  for(int i=0;i<6;i++) {
    MakeBCDDigit((0xff&str[i])-'0',&ptr);
  }
}


void MainObject::MakeSync(bool is_date,unsigned *ptr)
{
  for(int i=0;i<2;i++) {
    if(is_date) {
      MakeOne(ptr);
    }
    else {
      MakeZero(ptr);
    }
  }
}


void MainObject::MakeHexDigit(int digit,unsigned *ptr)
{
  for(int i=0;i<7;i++) {
    if((digit&(1<<(6-i)))!=0) {
      MakeOne(ptr);
    }
    else {
      MakeZero(ptr);
    }
  }
}


void MainObject::MakeBCDDigit(int digit,unsigned *ptr)
{
  for(int i=0;i<4;i++) {
    if((digit&(1<<(3-i)))!=0) {
      MakeOne(ptr);
    }
    else {
      MakeZero(ptr);
    }
  }
}


void MainObject::MakeOne(unsigned *ptr)
{
  switch(ese_format) {
  case SND_PCM_FORMAT_S32_LE:
    for(unsigned i=0;i<((float)ese_samplerate*ESE_SLOT_LENGTH/2.0);i++) {
      for(unsigned j=0;j<ese_channels;j++) {
	((int32_t *)ese_pcm_buffer)[*ptr+i*ese_channels+j]=ESE_32BIT_ON_LEVEL;
      }
    }
    break;

  case SND_PCM_FORMAT_S16_LE:
    for(unsigned i=0;i<((float)ese_samplerate*ESE_SLOT_LENGTH/2.0);i++) {
      for(unsigned j=0;j<ese_channels;j++) {
	((int16_t *)ese_pcm_buffer)[*ptr+i*ese_channels+j]=ESE_16BIT_ON_LEVEL;
      }
    }
    break;
  }
  *ptr+=ESE_SLOT_LENGTH*ese_channels*ese_samplerate;
}


void MainObject::MakeZero(unsigned *ptr)
{
  switch(ese_format) {
  case SND_PCM_FORMAT_S32_LE:
    for(unsigned i=0;i<((float)ese_samplerate*ESE_SLOT_LENGTH/4.0);i++) {
      for(unsigned j=0;j<ese_channels;j++) {
	((int32_t *)ese_pcm_buffer)[*ptr+i*ese_channels+j]=ESE_32BIT_ON_LEVEL;
      }
    }
    break;

  case SND_PCM_FORMAT_S16_LE:
    for(unsigned i=0;i<((float)ese_samplerate*ESE_SLOT_LENGTH/4.0);i++) {
      for(unsigned j=0;j<ese_channels;j++) {
	((int16_t *)ese_pcm_buffer)[*ptr+i*ese_channels+j]=ESE_16BIT_ON_LEVEL;
      }
    }
    break;
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
  switch(ese_config->alsaFormat()) {
  default:   // Autodetect
    if(snd_pcm_hw_params_set_format(ese_pcm,hwparams,
				    SND_PCM_FORMAT_S16_LE)==0) {
      ese_format=SND_PCM_FORMAT_S16_LE;
      Log("using S16_LE format");
    }
    else {
      if(snd_pcm_hw_params_set_format(ese_pcm,hwparams,
				      SND_PCM_FORMAT_S32_LE)==0) {
	ese_format=SND_PCM_FORMAT_S32_LE;
	Log("using S32_LE format");
      }
      else {
	*err_msg=tr("incompatible sample format");
	return false;
      }
    }
    break;

  case SND_PCM_FORMAT_S16_LE:
    if(snd_pcm_hw_params_set_format(ese_pcm,hwparams,
				    SND_PCM_FORMAT_S16_LE)==0) {
      ese_format=SND_PCM_FORMAT_S16_LE;
      Log("using S16_LE format");
    }
    else {
      fprintf(stderr,"esegend: unsupported format S16_LE\n");
      exit(256);
    }
    break;

  case SND_PCM_FORMAT_S32_LE:
    if(snd_pcm_hw_params_set_format(ese_pcm,hwparams,
				    SND_PCM_FORMAT_S32_LE)==0) {
      ese_format=SND_PCM_FORMAT_S32_LE;
      Log("using S32_LE format");
    }
    else {
      fprintf(stderr,"esegend: unsupported format S32_LE\n");
      exit(256);
    }
    break;
  }

  //
  // Sample Rate
  //
  ese_samplerate=ese_config->sampleRate();
  snd_pcm_hw_params_set_rate_near(ese_pcm,hwparams,&ese_samplerate,&dir);
  Log(QString::asprintf("requested %u, got %u samples/sec",
			ese_config->sampleRate(),ese_samplerate));

  //
  // Channels
  //
  ese_channels=ese_config->channels();
  snd_pcm_hw_params_set_channels_near(ese_pcm,hwparams,&ese_channels);
  Log(QString::asprintf("requested %u, got %u channels",
			ese_config->channels(),ese_channels));

  //
  // Buffer Parameters
  //
  ese_period_quantity=ese_config->periodQuantity();
  snd_pcm_hw_params_set_periods_near(ese_pcm,hwparams,&ese_period_quantity,
				     &dir);
  Log(QString::asprintf("requested %u, got %u periods",
			ese_config->periodQuantity(),ese_period_quantity));
  ese_buffer_size=ese_samplerate*0.0334;
  snd_pcm_hw_params_set_buffer_size_near(ese_pcm,hwparams,&ese_buffer_size);
  Log(QString::asprintf("requested %u, got %lu frame buffer",
			(unsigned)(ese_samplerate*0.0334),ese_buffer_size));

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
    memset(ese_pcm_buffer,0,sizeof(int32_t)*ese_buffer_size*ese_channels);
    break;

  case SND_PCM_FORMAT_S16_LE:
    ese_pcm_buffer=new int16_t[ese_buffer_size*ese_channels];
    memset(ese_pcm_buffer,0,sizeof(int16_t)*ese_buffer_size*ese_channels);
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

  return true;
}


void MainObject::Log(const QString &msg)
{
  if(ese_debug) {
    fprintf(stderr,"%s\n",(const char *)msg.toUtf8());
  }
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
