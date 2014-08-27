create table local_record
(
id integer primary key autoincrement,
dev_chl integer,
win_id integer,
record_type integer,
start_time integer,
end_time integer,
file_size integer,
path char(64)
);
create table search_record
{
id integer primary key autoincrement,
win_id integer,
record_type integer,
start_time char(32),
end_time char(32)
};