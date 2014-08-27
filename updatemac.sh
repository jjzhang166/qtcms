#! /bin/sh
read -p "witch one do you want to update?1 for debug;2 for release;3 for all." -n 1 update_type
echo ""
case $update_type in
1)
	dstdir=./mac/debug/QTTest.app/Contents/MacOS
	;;
2)
	dstdir=./mac/release/QTTest.app/Contents/MacOS
	;;
a|A)	;;
*)
	echo "error type"
	exit 0
	;;
esac

skindir=${dstdir}/skins
defaultskindir=${skindir}/default
pluginsdir=${dstdir}/plugins
mactopdir=./mac

if [ ! -d ${mactopdir} ]
then
mkdir ${mactopdir}
fi

if [ ! -d ${skindir} ]
then
mkdir ${skindir}
fi

if [ ! -d ${defaultskindir} ]
then
mkdir ${defaultskindir}
fi

if [ ! -d ${pluginsdir} ]
then
mkdir ${pluginsdir}
fi

cp -R web/* ${defaultskindir}/
mv ${defaultskindir}/MainSet.ini ${dstdir}

