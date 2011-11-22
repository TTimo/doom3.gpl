#!/bin/sh

echo "current pak files:       $1"
echo "expanded updated source: $2"
echo "new pak file:            $3"
echo "press a key"
read

TMPDIR=`mktemp -d -t`

ls "$1/"*.pk4 | while read i ; do unzip -l $i | cut -b 29- | tee $TMPDIR/`basename $i`.log ; done

ls $TMPDIR/*.log | while read i ; do lines=`cat $i | wc -l` ; tail +4 $i | head -$(( $lines - 5 )) | tee $TMPDIR/`basename $i`.2 ; done

# check cutting off
#ls $TMPDIR/*.log | while read i ; do diff $i $i.2 ; done

cat $TMPDIR/*.log.2 | sort -u | tee $TMPDIR/sorted-unique.log

# now the magical zip command
cd $2
rm $3
cat $TMPDIR/sorted-unique.log | zip -b $TMPDIR $3 -@ 1>/dev/null

md5sum $3
echo "done."

