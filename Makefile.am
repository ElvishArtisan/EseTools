## Makefile.am
##
## Top level Makefile.am
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2016-2025 Fred Gleason <fredg@paravelsystems.com>
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

SUBDIRS = helpers\
          conf\
          debian\
          src\
          systemd

deb:	dist
	./build_debs.sh

rpm:	dist
	cp esetools-$(VERSION).tar.gz $(HOME)/rpmbuild/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux esetools.spec
	mv $(HOME)/rpmbuild/RPMS/$(ARCH)/esetools-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/SRPMS/esetools-*.src.rpm $(top_srcdir)/
	rm $(HOME)/rpmbuild/SOURCES/esetools-$(VERSION).tar.gz
	rm -rf $(HOME)/rpmbuild/BUILD/esetools-$(VERSION)

EXTRA_DIST = autogen.sh\
             build_debs.sh.in\
             get_distro.pl\
             get_target.sh\
             link_common.sh\
             PACKAGE_VERSION

CLEANFILES = *~\
             *.deb\
             *.mo\
             *.rpm\
             *.tar.gz\
             *.tgz\
             *.zip

DISTCLEANFILES = *.service\
                 *.spec\
                 config.guess\
                 config.sub\
                 ltmain.sh

MAINTAINERCLEANFILES = *~\
                       *.buildinfo\
                       *.changes\
                       *.deb\
                       *.ddeb\
                       *.dsc\
                       *.spec\
                       *.tar.gz\
                       *.tar.xz\
                       aclocal.m4\
                       compile\
                       config.guess\
                       config.status\
                       config.sub\
                       configure\
                       depcomp\
                       install-sh\
                       libtool\
                       ltmain.sh\
                       Makefile.in\
                       missing\
                       mkinstalldirs
