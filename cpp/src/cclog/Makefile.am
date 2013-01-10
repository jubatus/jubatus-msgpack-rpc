# Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

if WANT_CCLOG

lib_LTLIBRARIES = libjubatus_cclog.la

libjubatus_cclog_la_includedir = $(includedir)/jubatus/cclog

libjubatus_cclog_la_SOURCES = \
		cclog.cc \
		cclog_null.cc \
		cclog_ostream.cc \
		cclog_syslog.cc \
		cclog_tty.cc

libjubatus_cclog_la_include_HEADERS = \
		cclog.h \
		cclog_null.h \
		cclog_ostream.h \
		cclog_syslog.h \
		cclog_tty.h

EXTRA_DIST = \
		cclog.hmpl

endif
