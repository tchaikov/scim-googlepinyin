AUTOMAKE_OPTIONS 	= gnu
SUBDIRS 		= share

MAINTAINERCLEANFILES 	= Makefile.in 

noinst_LTLIBRARIES = googlepinyin.la
#module_LTLIBRARIES = googlepinyin.la

googlepinyin_la_SOURCES = decoding_info.cpp \
                          pinyin_decoder_service.cpp \
                          pinyin_ime.cpp \
                          pinyin_lookup_table.cpp \
                          candidate_view.cpp \
                          composing_view.cpp \
                          google_imengine.cpp

googlepinyin_la_CXXFLAGS = -I$(top_builddir) -I./include \
                           @SCIM_CFLAGS@

googlepinyin_la_LDFLAGS = -avoid-version \
                          -module \
                          @SCIM_LIBS@

googlepinyin_la_LIBADD = ./share/libpinyinime.la
