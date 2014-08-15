#pragma once
class RepairDatabase
{
public:
	RepairDatabase(void);
	~RepairDatabase(void);
public:
	int fixExceptionalData();//0:成功，1：没有进行修复行为，2：存在修复失败的条目
private:
	int repairRecordDatabase();
	int repairSearchDatabase(); 
};

