noinst_LTLIBRARIES	= libpinyinime.la

libpinyinime_la_SOURCES = \
	dictbuilder.cpp \
	dictlist.cpp \
	dicttrie.cpp \
	lpicache.cpp \
	matrixsearch.cpp \
	mystdlib.cpp \
	ngram.cpp \
	searchutility.cpp \
	spellingtable.cpp \
	spellingtrie.cpp \
	splparser.cpp \
	userdict.cpp \
	utf16char.cpp \
	utf16reader.cpp \
	sync.cpp

libpinyinime_la_LDFLAGS = -no-undefined -avoid-version \
                          $(LIBTOOL_EXPORT_OPTIONS) \
			  @LTLIBINTL@

libpinyinime_la_CXXFLAGS = -Wall

libpinyinime_la_LIBADD = -lpthread
