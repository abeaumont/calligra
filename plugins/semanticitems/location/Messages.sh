#! /bin/sh
source ../../../kundo2_aware_xgettext.sh

$EXTRACTRC `find . -name \*.ui` >> rc.cpp
kundo2_aware_xgettext calligra_semanticitem_location.pot `find . -name \*.cpp`
rm -f rc.cpp
