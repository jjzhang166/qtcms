create table search_record
(
id integer primary key autoincrement,
nWndId integer,
nRecordType integer,
nStartTime integer,
nEndTime integer
);
create table record(
id integer primary key autoincrement,
nWndId integer,
nRecordType integer,
nStartTime integer,
nEndTime integer,
sFilePath char(64)
);
create table RecordFileStatus(
id integer primary key autoincrement,
sFilePath char(64),
nLock integer, //0 :解锁，1：锁定
nDamage integer, //0:可用，1：文件损坏
nInUse integer, //0：没用使用，1:已经使用
nFileNum integer//路径值：0000/0000/0000/0001.dat ==1; 
);