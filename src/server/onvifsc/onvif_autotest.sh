#!/bin/sh
# function : use to test NVP protocal interfaces
# author : kaga

app=./onvif.out
ip=192.168.1.218
port=80
username=admin
userpwd=
testpoint=(-devinfo
	-getvenc
	-setvenc
	-vencopt
	-getrtsp
	-stime
	-gtime
	-getether
	-setether
	-getimage
	-sethue
	-setcon
	-setsat
	-setsha
	-setbri
)

printusage()
{
	echo "Usage: "$0" [APP] [IP] [PORT] [USER-NAME] [USER-PASSWORD]";
	exit 0;
}

doautotest()
{
	i=0
	len=${#testpoint[*]}
	echo "total test count: "$len;
	while [ $i -lt $len ]
	do
		echo -n "<<<<<<< "$i" >>>>>>>>>>>  ""${testpoint[$i]}";
		#"$app" "$i" "$ip" "$port" "$username" "$userpwd";
		$app ${testpoint[$i]} $ip $port $username $userpwd;
	let i++
	done
}

if [ $# -eq 0 ];then
	printusage;
elif [ $# -ge 3 ];then
	app=$1;
	ip=$2;
	port=$3;
	if [ $# -eq 4 ];then
		username=$4;
	fi
	if [ $# -eq 5 ];then
		userpwd=$5;
	fi
	if [ $# -gt 5 ];then
		printusage;
	fi
	echo "Username: "$username" password: "$userpwd;
	doautotest;
else
	printusage;
fi

