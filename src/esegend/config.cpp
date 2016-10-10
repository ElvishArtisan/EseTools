// config.cpp
//
// Class for esegend(8) configuration.
//
// (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
//     All Rights Reserved.
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

#include <alsa/asoundlib.h>

#include <QSettings>

#include "config.h"

Config::Config()
{
}


Config::HourMode Config::hourMode() const
{
  return config_hour_mode;
}


void Config::setHourMode(Config::HourMode mode)
{
  config_hour_mode=mode;
}


QString Config::alsaDevice() const
{
  return config_alsa_device;
}


void Config::setAlsaDevice(const QString &str)
{
  config_alsa_device=str;
}


unsigned Config::alsaFormat() const
{
  return config_alsa_format;
}


void Config::setAlsaFormat(unsigned fmt)
{
  config_alsa_format=fmt;
}


unsigned Config::sampleRate() const
{
  return config_sample_rate;
}


void Config::setSampleRate(unsigned samprate)
{
  config_sample_rate=samprate;
}


unsigned Config::channels() const
{
  return config_channels;
}


void Config::setChannels(unsigned chans)
{
  config_channels=chans;
}


unsigned Config::periodQuantity() const
{
  return config_period_quantity;
}


void Config::setPeriodQuantity(unsigned periods)
{
  config_period_quantity=periods;
}


void Config::load()
{
  QSettings s(ESEGEND_CONF_FILE,QSettings::IniFormat);

  config_hour_mode=
    (Config::HourMode)s.value("HourMode",(unsigned)ESEGEND_DEFAULT_HOUR_MODE).
    toUInt();
  config_alsa_device=
    s.value("AlsaDevice",ESEGEND_DEFAULT_ALSA_DEVICE).toString();
  QString str=s.value("AlsaFormat",ESEGEND_DEFAULT_ALSA_FORMAT).toString();
  config_alsa_format=SND_PCM_FORMAT_UNKNOWN;
  if(str=="S16_LE") {
    config_alsa_format=SND_PCM_FORMAT_S16_LE;
  }
  if(str=="S32_LE") {
    config_alsa_format=SND_PCM_FORMAT_S32_LE;
  }
  config_sample_rate=s.value("SampleRate",ESEGEND_DEFAULT_SAMPLE_RATE).toUInt();
  config_channels=s.value("Channels",ESEGEND_DEFAULT_CHANNELS).toUInt();
  config_period_quantity=
    s.value("PeriodQuantity",ESEGEND_DEFAULT_PERIOD_QUANTITY).toUInt();
}


void Config::save() const
{
  QSettings s(ESEGEND_CONF_FILE,QSettings::IniFormat);
  s.setValue("HourMode",(unsigned)config_hour_mode);
  s.setValue("AlsaDevice",config_alsa_device);
  switch(config_alsa_format) {
  case SND_PCM_FORMAT_S16_LE:
    s.setValue("AlsaFormat","S16_LE");
    break;

  case SND_PCM_FORMAT_S32_LE:
    s.setValue("AlsaFormat","S32_LE");
    break;

  default:
    s.setValue("AlsaFormat",ESEGEND_DEFAULT_ALSA_FORMAT);
    break;
  }
  s.setValue("SampleRate",config_sample_rate);
  s.setValue("Channels",config_channels);
  s.setValue("PeriodQuantity",config_period_quantity);
}
