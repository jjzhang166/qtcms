0，进入gsoap目录，解压GSOAP源代码; 解压完成后打开tool/setup.sh,把gsoap_dir修改成你实际的
gsoap源码目录
# ./unzip gsoap_2.8.10.zip
1, 进行tool 目录，生成soap相关代码
# ./setup {CMD} {SC_OPT}
1.1）{CMD} ： {-1|-2|-3|-4|-5|-A} ；命令码
            -1 , 使用gsoap2.8.10 生成soap中间文件
			-2 ，使用gsoap2.8.11 生成soap中间文件
			-3 ，对生成的中间文件进行打补丁
				a）import "wsse.h" ; 用于支持用户验证
				b)由于2.8.10的版本生成event的代码会存在多个命名空间的附本，2.8.11以上没有这个问题，但是2.8.11测试发现对大华的部分设备支持不了；所以将2.8.11生成的中间文件跟2.8.10生成的中间文件进行合并。
			-4 ，生成soap源码，需要指定{SC_OPT}参数
			-5 ，对生成的源码，打补丁
				a）soapStub.h , 去掉 SOAP_WSA_2005的定义; 另外加入SOAP_CLIENT 或者SOAP_SERVERR 的自定义变量，用于服务端客户端代码控制
				b) soapServer.c 去掉重定义的函数：soap_serve_SOAP_ENV__Fault
				c) soapClient.c 去掉重定义的函数：soap_send_SOAP_ENV__Fault & soap_recv_SOAP_ENV__Fault
				d) onvif.nsmap 添加两个命名空间：
					> 	{"tns1", "http://www.onvif.org/ver10/topics", NULL, NULL},
					> 	{"ter", "http://www.onvif.org/ver10/error", NULL, NULL},
			-A ，单条命令生成相关代码，相当于依次执行了以上5条命令。
			
1.2） {SC_OPT} :｛-S|-C|-SC}
			-S , 仅生成服务端代码
			-C ，仅生成客户端代码
			-SC ，同时生成客户端和服务端代码
1.3）gsoap版本说明，目前主要使用2.8.10版本
1.4）关于WSDL相关说明：
	a) 本目录下的所有WSDL文件都将在线的xml schema, wsdl, namespace等索引更改到本地路径的索引（下载到本地目录了）
	b) 部分WSDL文件会引用其他WSDL文件，这样生成代码时会有接口的定义的情况，故引用部分须注释掉
	c) 在typemap.dat 类型定义中指定命名空间的prefix; 防止随机生成ns-prefix的情况
	d) 目前把ONVIF规范中dn:NetworkVideoTransmitter 需要实现，建议实现，可选实现的部分都已经加入进来，具体包括：
	"imaging.wsdl event.wsdl analytics.wsdl devicemgmt.wsdl media.wsdl remotediscovery.wsdl ptz.wsdl event.wsdl analytics.wsdl deviceio.wsdl"
	e) 在增加或删除WSDL文件时，或者WSDL降级或升级可能会出现部分PATCH文件失效的情况，如果出现此情况，根据1.1的说明进行手动PATCH，并且更新相应的PATCH文件
	
例如, 生成客户端和服务端代码：
# ./setup -A -SC 

2，进入项目根目录，进行编译

# make
# make install

相当于
# make BUILD=debug
# make BUILD=debug install

默认是生成调试版本(此时SOAP发送包，接收包以及系统的操作都会以写文件的形式保存），如果要生成发行版本，用以下命令：

# make BUILD=release
# make BUILD=release install

注：2.1），发行的库和相应的头文件分别在./install/{buildmode}/lib,./install/{buildmode}/include目录下
	2.2），如果需要修改GCC工具，打开Makefile文件，修改PRJ_CROSS变量

3， 生成可执行的测试代码
# ./setup -A -SC
# make demo


4， 版本声明
ONVIFLIB V1.0
ONVIF版本：
devicemgmt: V2.3
event: V2.4
imaging: V2.2
media: V2.4.1
analytics: V2.2
PTZ：V2.4
device io: V2.2

5, 项目目录结构说明：
--根目录：Makefile, Makefile.lib Maine.c ...
  |--onvif : onvif库源码目录
  |--generic: onvif 库依赖的基本接口库，如base64,sha1,packbit,xml, env_commmon, debug, ticker.
  |--tool : 使用GSOAP生成soap框架的资源与工具集，并且包括WSDL， ONVIF schema, xml namespace, 脚本
     |--./setup.sh : 用于生成SOAP框架源码的脚本
  |--patch : 补丁，使用gsoap生成的源码须要打一些必要的补丁，才能正常编译和运行良好
  |--other : 主要是此版本用于不同项目下的ENV实现版本，如nvp_env_ipc.c
  |--tmp : 代码编译时生成的临时文件或中间文件
     |-- debug : DEBUG模式下的
	 |-- release : release 模式下
  |--lib : 代码编译后生成的库文件目录
     |--debug
	 |--release
  |--install :默认的安装目录
     |--debug
	 |--release
  |--gosap : 用于保存gsoap的源码包