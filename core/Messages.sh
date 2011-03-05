#! /bin/sh
$PREPARETIPS > data/tips.cpp
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp` `find digikam -name \*.h.cmake` -o $podir/digikam.pot
rm -f tips.cpp
rm -f rc.cpp
