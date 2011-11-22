#!/bin/sh
base=/home/doom-grp/Doom/base
host=timo@idnet.ua-corp.com
srvscript=/var/local/builds/misc/packup.sh

maindir=$(pwd)
cd $(dirname $1)
source=$(pwd)/$(basename $1)
outdir=$(pwd)
cd $maindir

echo "list of files : $source"
echo "base          : $base"
echo "host          : $host"
echo "server script : $srvscript"
echo "press enter"
read

flip -u $source

(
cd $base
cat $source | while read i ; do find . -ipath "./$i" | cut -b 3- ; done | tee $outdir/matched.cased.log
)

# find the no match, not even case sensitive
diff -ui $source $outdir/matched.cased.log | grep ^- | cut -b 2- | tee $outdir/missing.log

scp $outdir/missing.log $host:/home/timo/missing.log
ssh $host $srvscript /home/timo/dl.zip /home/timo/missing.log
rm $outdir/dl.zip
scp $host:/home/timo/dl.zip $outdir
scp $host:/home/timo/cased.log $outdir/missing.cased.log
(
cd $base
unzip $outdir/dl.zip

# merge both lists into a single thing
rm $outdir/dl.zip
cat $outdir/missing.cased.log | zip $outdir/dl.zip -@
cat $outdir/matched.cased.log | zip $outdir/dl.zip -@
)
