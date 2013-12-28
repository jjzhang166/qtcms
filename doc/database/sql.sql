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
 schedle_id integer,
 weekday integer,
 starttime char£¨64£©,
endtime char£¨64£©,
enable integer
 );
 
 create table disks_setting
(
id integer primary key autoincrement,
name char(64),
value text,
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

create trigger chl_delete
before delete on chl
for each row
begin
delete from r_chl_group where chl_id = old.id;
end;

create trigger dev_group_delete
before delete on dev_group
for each row
begin
delete from r_chl_group where group_id = old.id;
end;

pragma recursive_triggers=true;

insert into disks_setting (name,value) values('use_disks','D:');
insert into disks_setting (name,value) values('b_cover','true');
insert into disks_setting (name,value) values('file_size','128');
insert into disks_setting (name,value) values('reserved_size','1024');

insert into area (pid,path) values(0,'0|1|');
insert into area (pid,path) values(1,'0|1|2|');
insert into area (pid,path) values(1,'0|1|3|');
insert into area (pid,path) values(2,'0|1|2|4|');
insert into area (pid,path) values(3,'0|1|3|5|');
insert into area (pid,path) values(3,'0|1|3|6|');
insert into area (pid,path) values(3,'0|1|3|7|');
insert into area (pid,path) values(7,'0|1|3|7|8|');
insert into area (pid,path) values(4,'0|1|2|4|9|');
insert into dev (area_id) values(3);
insert into dev (area_id) values(3);
insert into dev (area_id) values(5);
insert into dev (area_id) values(2);
insert into dev (area_id) values(7);
insert into dev (area_id) values(8);
insert into chl (dev_id) values(1);
insert into chl (dev_id) values(1);
insert into chl (dev_id) values(1);
insert into chl (dev_id) values(1);
insert into chl (dev_id) values(2);
insert into chl (dev_id) values(2);
insert into chl (dev_id) values(2);
insert into chl (dev_id) values(2);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(3);
insert into chl (dev_id) values(5);
insert into chl (dev_id) values(5);
insert into chl (dev_id) values(5);
insert into chl (dev_id) values(5);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(6);
insert into chl (dev_id) values(4);
insert into chl (dev_id) values(4);
insert into chl (dev_id) values(4);
insert into chl (dev_id) values(4);
insert into dev_group (id) values(1);
insert into dev_group (id) values(2);
insert into dev_group (id) values(3);
insert into r_chl_group (chl_id,group_id) values(46,1);
insert into r_chl_group (chl_id,group_id) values(47,1);
insert into r_chl_group (chl_id,group_id) values(14,1);
insert into r_chl_group (chl_id,group_id) values(15,1);
insert into r_chl_group (chl_id,group_id) values(18,1);
insert into r_chl_group (chl_id,group_id) values(19,1);
insert into r_chl_group (chl_id,group_id) values(2,2);
insert into r_chl_group (chl_id,group_id) values(11,2);
insert into r_chl_group (chl_id,group_id) values(12,2);
insert into r_chl_group (chl_id,group_id) values(25,2);
insert into r_chl_group (chl_id,group_id) values(46,3);
insert into r_chl_group (chl_id,group_id) values(48,3);
insert into r_chl_group (chl_id,group_id) values(21,3);
insert into r_chl_group (chl_id,group_id) values(25,3);