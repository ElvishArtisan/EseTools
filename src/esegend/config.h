// config.h
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#define ESEGEND_CONF_FILE "/etc/esegend.conf"
#define ESEGEND_DEFAULT_ALSA_DEVICE "hw:0"

class Config
{
 public:
  Config();
  QString alsaDevice() const;
  void setAlsaDevice(const QString &str);
  void load();
  void save() const;

 private:
  QString config_alsa_device;
};


#endif  // CONFIG_H
