include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_v150_sprt

mod_LTLIBRARIES = mod_v150_sprt.la
mod_v150_sprt_la_SOURCES  = mod_v150_sprt.c
mod_v150_sprt_la_CFLAGS   = $(AM_CFLAGS)
mod_v150_sprt_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_v150_sprt_la_LDFLAGS  = -avoid-version -module -no-undefined -shared
