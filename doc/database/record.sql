create table local_record
(
id integer primary key autoincrement,
dev_name chr(32),
dev_chl integer,
win_id integer,
date char(32),
start_time char(32),
end_time char(32),
record_type integer,
file_size integer,
path char(64)
);
