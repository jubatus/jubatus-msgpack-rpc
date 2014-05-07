# Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

AM_CPPFLAGS   = -I../src
AM_C_CPPFLAGS = -I../src

AM_LDFLAGS = \
		../src/cclog/libjubatus_cclog.la \
		../src/jubatus/msgpack/rpc/libjubatus_msgpack-rpc.la \
		-ljubatus_mpio -lmsgpack

check_HEADERS = \
		echo_server.h

check_PROGRAMS = \
		sync_call \
		async_call \
		callback \
		notify \
		async_server \
		error \
		udp \
		unix \
		attack_connect \
		attack_pipeline \
		attack_huge \
		attack_callback

TESTS = $(check_PROGRAMS)

sync_call_SOURCES = sync_call.cc

async_call_SOURCES = async_call.cc

callback_SOURCES = callback.cc

notify_SOURCES = notify.cc

async_server_SOURCES = async_server.cc

error_SOURCES = error.cc

udp_SOURCES = udp.cc

unix_SOURCES = unix.cc

attack_connect_SOURCES = attack_connect.cc

attack_pipeline_SOURCES = attack_pipeline.cc

attack_huge_SOURCES = attack_huge.cc

attack_callback_SOURCES = attack_callback.cc

