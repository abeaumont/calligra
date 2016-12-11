#! /bin/sh
# test_kundo2_i18nc:
# $1: kundo2_aware_xgettext.sh 
# $2: xgettext
# $3: msgcat
# $4: podir

# source the kundo2_aware_xgettext.sh script
. $1

# setup environment variables for kundo2_aware_xgettext.sh
XGETTEXT_PROGRAM=$2
MSGCAT=$3
podir=$4

# get common parameters
. parameters.sh
# kundo2_aware_xgettext.sh wants this in one variable
XGETTEXT="$XGETTEXT_PROGRAM $XGETTEXT_FLAGS"

potfile="test_kundo2_i18nc.pot"
cppfile="test_kundo2_i18nc.cpp"

kundo2_aware_xgettext $potfile $cppfile

# check result
if test ! -e $podir/$potfile; then
    echo "FAIL: pot file not created"
    exit 1
fi
if test 1 -ne `grep qtundo-format $podir/$potfile|wc -l`; then
    echo "FAIL: there should be 1 qtundo-format strings"
    exit 2
fi
if test 2 -ne `grep "^msgid \"" $podir/$potfile|wc -l`; then
    echo "FAIL: there should be 2 message strings"
    exit 3
fi

exit 0
