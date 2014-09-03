create table search_record
(
id integer primary key autoincrement,
nWndId integer,
nRecordType integer,
nStartTime integer,
nEndTime integer,
);
create table record(
id integer primary key autoincrement,
nWndId integer,
nRecordType integer,
nStartTime integer,
nEndTime integer,
sFilePath char(64)
);