select cast(true as integer);

create table a (i integer);
insert into a values (1);
insert into a values (3);

select cast(i<2 as integer)+1 from a;
rollback;
