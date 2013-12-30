
typedef struct _tagDiskInfo{
	char cDiskName;
	unsigned long long ullFreeBytesToCaller;
	unsigned long long ullTotalBytes;
	unsigned long long ullFreeBytes;
}DiskInfo;

extern int g_diskNum;
extern DiskInfo g_allDiskInof[30];

//获取逻辑盘的所有盘符
//dvrStr为输出字符串
//获取成功返回字符串长度，否则返回0
int getLogicalDriveStrings(char *pstrDriveStr);

//获取指定盘信息，包括总大小，可用空间
//pstrDisk为输入盘符，如：“D:\”
//ullFreeBytesToCaller为可获得的空闲空间（字节）
//ullTotalBytes为磁盘总容量（字节）
//ullFreeBytes为"空闲空间（字节）
//获取成功返回true，否则为false
bool getDiskFreeSpaceEx(const char *pstrDisk, 
						unsigned long long &ullFreeBytesToCaller,
						unsigned long long &ullTotalBytes,
						unsigned long long &ullFreeBytes);

//保存所有盘的信息，保存到g_allDiskInof中
//保存成功返回true，否则返回false
bool saveAllDistInfo();

//获取所有盘的信息，保存到g_allDiskInof中
//获取成功返回盘的个数，结果由diskInfo传出，否则返回0
int getAllDiskInfo(DiskInfo *diskInfo);