dictdatadir = $(datadir)/google-pinyin
dictdata_DATA = dict_pinyin.dat
dict_pinyin.data: rawdict_utf16_65105_freq.txt valid_utf16.txt ../pinyinime_dictbuilder
	../pinyinime_dictbuilder

EXTRA_DIST = \
         $(dictdata_DATA)