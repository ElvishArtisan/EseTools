## Makefile.am
##
## Makefile.am for the esegen(1) timecode generator
##
## (C) Copyright 2014-2025 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing  -std=c++17 -fPIC @QT6_CFLAGS@ @ALSA_CFLAGS@
MOC = @QT6_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

sbin_PROGRAMS = esegend

dist_esegend_SOURCES = config.cpp config.h\
                       esegend.cpp esegend.h

nodist_esegend_SOURCES = cmdswitch.cpp cmdswitch.h\
                         moc_esegend.cpp

esegend_LDADD = @QT6_LIBS@ @ALSA_LIBS@ -lpthread

CLEANFILES = *~\
             moc_*\
             *.obj\
             *.idb\
             *.pdb\
             *ilk
DISTCLEANFILES = cmdswitch.cpp cmdswitch.h
MAINTAINERCLEANFILES = *~\
                       Makefile.in
