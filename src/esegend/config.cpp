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

#include <QSettings>

#include "config.h"

Config::Config()
{
}


QString Config::alsaDevice() const
{
  return config_alsa_device;
}


void Config::setAlsaDevice(const QString &str)
{
  config_alsa_device=str;
}


void Config::load()
{
  QSettings s(ESEGEND_CONF_FILE,QSettings::IniFormat);

  config_alsa_device=
    s.value("AlsaDevice",ESEGEND_DEFAULT_ALSA_DEVICE).toString();
}


void Config::save() const
{
  QSettings s(ESEGEND_CONF_FILE,QSettings::IniFormat);
  s.setValue("AlsaDevice",config_alsa_device);
}
