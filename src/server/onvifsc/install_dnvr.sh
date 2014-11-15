#########################################################################
# File Name: install_ipc.sh
# Author: kaga
# mail: kejiazhw@163.com
# Created Time: 2014年07月01日 星期二 20时22分22秒
#########################################################################
#!/bin/bash

install_dir=../hzj_nvr/app
CROSS=arm-hisiv100nptl-linux-

if [ "$1" = "-debug" ];then
	make PRJ_CROSS=$CROSS BUILD=debug; make BUILD=debug install
	echo "WARNING: install debug version !!!!!"
	cp install/debug/lib/libonvif.a $install_dir/lib_ext/
	cp install/debug/include/onvif.h $install_dir/include/
else
	make PRJ_CROSS=$CROSS BUILD=release; make BUILD=release install
	echo "INFO: install release version !!!!!"
	cp install/release/lib/libonvif.a $install_dir/lib_ext/
	cp install/release/include/onvif.h $install_dir/include/
fi
cp generic/packbit.c $install_dir/generic/
cp generic/packbit.h $install_dir/include/
#cp generic/generic.c $install_dir/generic/
#cp generic/generic.h $install_dir/include/
cp generic/sha1.c $install_dir/generic/
cp generic/sha1.h $install_dir/include/
cp generic/env_common.h $install_dir/
cp generic/nvp_define.h $install_dir/
cp other/onvif_spook.c $install_dir/
cp other/onvif_spook.h $install_dir/
cp other/nvp_env_dnvr.c $install_dir/
cp nvp_trace.c $install_dir/
