################################################################################
# Copyright (c) 2010 Krell Institute All Rights Reserved.
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

BUILT_SOURCES = \
	blobs.h blobs.c

pkglib_LTLIBRARIES =

if !BUILD_RUNTIME
if !HAVE_TARGET_LIBMONITOR
pkglib_LTLIBRARIES += hwcsamp.la
endif
endif

if BUILD_INSTRUMENTOR
if !BUILD_OFFLINE
pkglib_LTLIBRARIES += hwcsamp-rt.la
endif
endif

if BUILD_INSTRUMENTOR
if HAVE_LIBMONITOR
OFFLINE_SOURCES = \
	offline.c

pkglib_LTLIBRARIES += hwcsamp-rt-offline.la libhwcsamp-offline.la
endif

if HAVE_TARGET_LIBMONITOR
OFFLINE_SOURCES = \
	offline.c

pkglib_LTLIBRARIES += libhwcsamp-offline.la
endif
endif

if !BUILD_RUNTIME
hwcsamp_la_CXXFLAGS = \
	-I$(top_srcdir)/libopenss-message \
	-I$(top_srcdir)/libopenss-framework \
	@PAPI_CPPFLAGS@

hwcsamp_la_LDFLAGS = \
	-L$(top_srcdir)/libopenss-message \
	-L$(top_srcdir)/libopenss-framework \
	@PAPI_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwcsamp_la_LIBADD = \
	-lopenss-message -lopenss-framework \
	@PAPI_LIBS@

hwcsamp_la_SOURCES = \
	blobs.x $(BUILT_SOURCES) \
	HWCSampCollector.hxx HWCSampCollector.cxx
endif

if BUILD_INSTRUMENTOR
if !BUILD_OFFLINE
if BUILD_MRNET
hwcsamp_rt_la_CFLAGS = \
	-I$(top_srcdir)/libopenss-runtime \
	@PAPI_CPPFLAGS@
else
hwcsamp_rt_la_CFLAGS = \
	-I$(top_srcdir)/libopenss-runtime \
	@PAPI_CPPFLAGS@
endif

hwcsamp_rt_la_LDFLAGS = \
	-L$(top_srcdir)/libopenss-runtime \
	@PAPI_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwcsamp_rt_la_LIBADD = \
	-lopenss-runtime \
	-lopenss-runtime-papi \
	@PAPI_LIBS@ \
	-lpthread -lrt

hwcsamp_rt_la_SOURCES = \
	blobs.x $(BUILT_SOURCES) \
	runtime.c
endif

if HAVE_LIBMONITOR
hwcsamp_rt_offline_la_CFLAGS = \
	-DOPENSS_OFFLINE \
	-I$(top_srcdir)/libopenss-runtime \
	-I$(top_srcdir)/libopenss-runtime/offline \
	@PAPI_CPPFLAGS@

hwcsamp_rt_offline_la_LDFLAGS = \
	-L$(top_srcdir)/libopenss-runtime \
	-L$(top_srcdir)/libopenss-runtime/offline \
	@PAPI_LDFLAGS@ \
	-no-undefined -module -avoid-version

hwcsamp_rt_offline_la_LIBADD = \
	-lopenss-runtime-offline -lopenss-runtime-monitor \
	-lopenss-runtime-papi \
	@PAPI_LIBS@ \
	-lpthread -lrt

hwcsamp_rt_offline_la_SOURCES = \
	blobs.x $(BUILT_SOURCES) \
	$(OFFLINE_SOURCES) \
	runtime.c


libhwcsamp_offline_la_CFLAGS = \
	-static \
	-DOPENSS_OFFLINE \
	-I$(top_srcdir)/libopenss-runtime \
	-I$(top_srcdir)/libopenss-runtime/offline \
	@PAPI_CPPFLAGS@

libhwcsamp_offline_la_LDFLAGS = \
	-static \
	-L$(top_srcdir)/libopenss-runtime \
	-L$(top_srcdir)/libopenss-runtime/offline \
	@PAPI_LDFLAGS@ \
	-no-undefined

libhwcsamp_offline_la_LIBADD = \
	-lopenss-runtime-offline -lopenss-runtime-monitor \
	-lopenss-runtime-papi \
	@PAPI_LIBS@ \
	-lpthread -lrt

libhwcsamp_offline_la_SOURCES = \
	blobs.x $(BUILT_SOURCES) \
	$(OFFLINE_SOURCES) \
	runtime.c
endif

if HAVE_TARGET_LIBMONITOR
libhwcsamp_offline_la_CFLAGS = \
	-static \
	-DOPENSS_OFFLINE \
	-I$(top_srcdir)/libopenss-runtime \
	-I$(top_srcdir)/libopenss-runtime/offline \
	@TARGET_PAPI_CPPFLAGS@

libhwcsamp_offline_la_LDFLAGS = \
	-static \
	-L$(top_srcdir)/libopenss-runtime \
	-L$(top_srcdir)/libopenss-runtime/offline \
	@TARGET_PAPI_LDFLAGS@ \
	-no-undefined

libhwcsamp_offline_la_LIBADD = \
	-lopenss-runtime-offline -lopenss-runtime-monitor \
	-lopenss-runtime-papi \
	@TARGET_PAPI_LIBS@ \
	-lpthread -lrt

libhwcsamp_offline_la_SOURCES = \
	blobs.x $(BUILT_SOURCES) \
	$(OFFLINE_SOURCES) \
	runtime.c
endif
endif



SUFFIXES = .x
CLEANFILES = $(BUILT_SOURCES)

blobs.h blobs.c : blobs.x
	rm -f  $(patsubst %.x, %.h, $<) $(patsubst %.x, %.c, $<)
	$(RPCGEN) -h -o $(patsubst %.x, %.h, $<) $<
	$(RPCGEN) -c -o $(patsubst %.x, %.c, $<) $<