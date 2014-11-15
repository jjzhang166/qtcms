#########################################################################
# File Name: setupip.sh
# Author: kaga
# mail: kejiazhw@163.com
# Created Time: 2014年08月08日 星期五 21时34分25秒
#########################################################################
#!/bin/bash
_setip=192.168.1.86
_setmask=255.255.255.0
./onvif.out.bak -setether 172.16.0.5 80 admin admin $_setip $_setmask
./onvif.out.bak -setether 192.168.1.15 80 admin admin $_setip $_setmask
./onvif.out.bak -setether 192.168.1.16 80 admin admin $_setip $_setmask
./onvif.out.bak -setether 192.168.1.17 80 admin admin $_setip $_setmask
