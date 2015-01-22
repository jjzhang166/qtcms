create table user(
id integer primary key autoincrement,
userName text,
password text,
nLimit integer,
userState integer,
logTime integer,
logOutInterval integer
);
create table user_sub_limit(
id integer primary key autoincrement,
userName text,
mainSingleCode integer,
subCode integer
);
create table user_infomation(
id integer primary key autoincrement,
username text,
password text,
level integer,
mask1 integer,
mask2 integer
);

create table area
(
id integer primary key autoincrement,
pid integer ,
name char(64),
path text default ''
);

create table dev
(
id integer primary key autoincrement,
area_id integer,
address char(64),
port integer,
http integer,
eseeid char(32),
username char(32),
password char(32),
name char(64),
channel_count integer,
connect_method integer,
vendor text
);

create table chl
(
id integer primary key autoincrement,
dev_id integer,
channel_number integer,
name char(64),
stream_id integer
);

create table dev_group
(
id integer primary key autoincrement,
name char(64)
);

create table r_chl_group
(
id integer primary key autoincrement,
chl_id integer,
group_id integer,
name char(64)
);

create table recordtime
 (
 id integer primary key autoincrement,
 chl_id integer,
 schedule_id integer,
 weekday integer,
 starttime char£¨64£©,
endtime char£¨64£©,
enable integer
 );
 
create table general_setting
(
id integer primary key autoincrement,
name char(64),
value text
);

create table window_settings
(
id integer primary key autoincrement,
wnd_id integer,
stretch integer,
chl_id integer
);

create trigger area_delete
before delete on area
for each row
begin
delete from dev where area_id = old.id;
end;

create trigger dev_delete
before delete on dev
for each row
begin
delete from chl where dev_id = old.id;
end;

create trigger chl_insert
after insert on chl
for each row
begin
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,0,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,0,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,0,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,0,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,1,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,1,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,1,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,1,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,2,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,2,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,2,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,2,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,3,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,3,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,3,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,3,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,4,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,4,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,4,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,4,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,5,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,5,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,5,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,5,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,0,6,'1970-01-01 00:00:00','1970-01-01 23:59:59',1);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,1,6,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,2,6,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
insert into recordtime(chl_id,schedule_id,weekday,starttime,endtime,enable) values(new.id,3,6,'1970-01-01 00:00:00','1970-01-01 23:59:59',0);
end;

create trigger chl_delete
before delete on chl
for each row
begin
delete from r_chl_group where chl_id = old.id;
delete from recordtime where chl_id = old.id;
end;

create trigger dev_group_delete
before delete on dev_group
for each row
begin
delete from r_chl_group where group_id = old.id;
end;

pragma recursive_triggers=true;

insert into general_setting (name,value) values('storage_usedisks','D:');
insert into general_setting (name,value) values('storage_cover','true');
insert into general_setting (name,value) values('storage_filesize','128');
insert into general_setting (name,value) values('storage_reservedsize','6000');
insert into general_setting (name,value) values('misc_slanguage','zh_CN');
insert into general_setting (name,value) values('misc_aptime','30');
insert into general_setting (name,value) values('misc_smode','div4_4');
insert into general_setting (name,value) values('misc_alogin','true');
insert into general_setting (name,value) values('misc_synctime','true');
insert into general_setting (name,value) values('misc_aconnent','false');
insert into general_setting (name,value) values('misc_afullscreen','false');
insert into general_setting (name,value) values('misc_bootstart','false');
insert into general_setting (name,value) values('misc_keepCurrentUserPassWord','ture');
insert into general_setting (name,value) values('misc_CurrentUserName','admin');
insert into general_setting (name,value) values('misc_CurrentUserPassWord','');

insert into user (userName,password,nLimit,userState,logTime,logOutInterval) values('admin','d41d8cd98f00b204e9800998ecf8427e','11111111111','0','0','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','1','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','10','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','100','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','1000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','10000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','100000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','1000000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','10000000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','100000000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','1000000000','0');
insert into user_sub_limit (userName,mainSingleCode,subCode) values('admin','10000000000','0');

insert into window_settings (wnd_id,stretch,chl_id) values(0,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(1,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(2,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(3,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(4,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(5,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(6,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(7,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(8,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(9,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(10,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(11,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(12,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(13,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(14,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(15,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(16,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(17,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(18,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(19,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(20,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(21,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(22,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(23,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(24,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(25,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(26,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(27,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(28,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(29,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(30,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(31,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(32,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(33,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(34,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(35,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(36,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(37,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(38,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(39,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(40,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(41,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(42,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(43,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(44,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(45,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(46,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(47,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(48,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(49,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(50,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(51,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(52,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(53,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(54,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(55,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(56,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(57,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(58,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(59,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(60,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(61,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(62,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(63,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(64,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(65,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(66,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(67,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(68,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(69,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(70,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(71,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(72,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(73,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(74,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(75,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(76,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(77,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(78,1,-1);
insert into window_settings (wnd_id,stretch,chl_id) values(79,1,-1);
