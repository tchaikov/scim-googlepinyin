scim-googlepinyin
=================

scim-googlepinyin is an SCIM port of google-pinyin on android platform. It's basically a translation from its original java code to C++ counterpart. Android google pinyin's core is not changed in porting, but this user interface is slightly modified to simulate the bevaviour of GooglePinyin on Windows.

why scim-googlepinyin
---------------------

I found lots of GNU/Linux users are long for the Google Pinyin for GNU/Linux. And Google is very generous to open source google-pinyin on android. So here it is. SCIM was chosed as the target platform because I use SCIM on a daily basis. Yes, ibus is way better than SCIM in term of compatibility. But I am not quite familiar with it's development yet.

scim-googlepinyin is not
------------------------

scim-googlepinyin is not google-pinyin on andoird.

Due to some limitation of SCIM platform (or GNU/Linux), some features are chopped off. Like
- inline edit
- predict according to inputted character (we are not able to tell what the text is before input cursor)
- predict based on the application we are inputting text to (no way to find out the app)

At the time of writing, android google-pinyin has release its version 1.1.3.  While the code on `android open souce project <http://android.git.kernel.org/?p=platform/packages/inputmethods/PinyinIME.git>`_ is obviously older than the latest version, which lacks some features like synchronize the user lemma with server. And there is legal issue to analyze the binary of android google-pinyin. So instead of reverse engineering, we need to find some other way to improve scim-googlepinyin.

`Google Pinyin for Windows<http://www.google.com/ime/pinyin/>`_  sets a high bar for us. This software is not able to match with Google Pinyin for Windows in serveral ways. To name some major features we are missing:
- lacks a good/large enough language model. The size of IME's LM is around 10MiB while that of andoid google-pinyin is only 1.1MiB.
- no `i' mode support
- no English word input assistance
- not able to export/import user dict in text format
- no doodle or other eye candies

.. Kov Chai <tchaikov@gmail.com>
