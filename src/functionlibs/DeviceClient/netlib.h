// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 NETLIB_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// NETLIB_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef NETLIB_H
#define NETLIB_H

#define AF_INET         2  /* internetwork: UDP, TCP, etc. */

extern "C"{
	unsigned short htonsQ(unsigned short hostshort);
	unsigned long htonlQ(unsigned long hostlong);

	void SleepQ(unsigned long hostshort);
	unsigned long GetTickCountQ();

	unsigned long GetServerAddr(const char* hostname);

	bool GetDiskFreeSpaceExQ(char* diskname,
					unsigned long long* lpFreeBytesAvailableToCaller,
					unsigned long long* lpTotalNumberOfBytes,
					unsigned long long* lpTotalNumberOfFreeBytes);
};

#endif