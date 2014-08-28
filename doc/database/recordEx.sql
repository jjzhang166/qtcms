create table search_record
{
id integer primary key autoincrement,
win_id integer,
record_type integer,
start_time char(32),
end_time char(32),
path char(64)
};