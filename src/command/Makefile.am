noinst_PROGRAMS = pinyinime_dictbuilder
pinyinime_dictbuilder_CXXFLAGS = -g3 -Wall -lpthread -D___BUILD_MODEL___
LIBRARY_SRC = \
	    ../share/dictbuilder.cpp \
	    ../share/dictlist.cpp \
	    ../share/dicttrie.cpp \
	    ../share/lpicache.cpp \
	    ../share/mystdlib.cpp \
	    ../share/ngram.cpp \
	    ../share/searchutility.cpp \
	    ../share/spellingtable.cpp \
	    ../share/spellingtrie.cpp \
	    ../share/splparser.cpp \
	    ../share/utf16char.cpp \
	    ../share/utf16reader.cpp

pinyinime_dictbuilder_SOURCES = \
            $(LIBRARY_SRC) \
            pinyinime_dictbuilder.cpp

corpusdata_file = ../data/rawdict_utf16_65105_freq.txt ../data/valid_utf16.txt
dictdata_file = ../../data/dict_pinyin.data

dictdata: $(dictdata_file)

$(dictdata_file): pinyinime_dictbuilder $(corpusdata_file)
	./pinyinime_dictbuilder
