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
insert into general_setting (name,value) values('storage_reservedsize','1024');
insert into general_setting (name,value) values('misc_slanguage','zh_CN');
insert into general_setting (name,value) values('misc_aptime','120');
insert into general_setting (name,value) values('misc_smode','div4_4');
insert into general_setting (name,value) values('misc_alogin','true');
insert into general_setting (name,value) values('misc_synctime','true');
insert into general_setting (name,value) values('misc_aconnent','false');
insert into general_setting (name,value) values('misc_afullscreen','true');
insert into general_setting (name,value) values('misc_bootstart','false');