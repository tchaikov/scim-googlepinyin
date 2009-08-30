icondir = @SCIM_ICONDIR@
icon_DATA = google-pinyin_icon.png

dictdatadir = @SCIM_GOOGLEPINYIN_DATADIR@
dictdata_DATA = dict_pinyin.dat

dictbuilder = ../src/command/pinyinime_dictbuilder
corpusdata = ../src/data/rawdict_utf16_65105_freq.txt ../src/data/valid_utf16.txt

dict_pinyin.dat: 
	make -C ../src/command dictdata

EXTRA_DIST = \
         $(icon_DATA) \
         $(dictdata_DATA)
