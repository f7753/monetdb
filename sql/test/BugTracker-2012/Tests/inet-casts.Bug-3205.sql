create table tab3205(pos int, col inet);
insert into tab3205 values(1, '127.0.0.1');
insert into tab3205 values(2, inet '127.0.0.1');
insert into tab3205 values(3, cast('127.0.0.1' as inet));
select * from tab3205 order by pos;
select * from tab3205 where col = '127.0.0.1' order by pos;
select * from tab3205 where col = inet '127.0.0.1' order by pos;
select * from tab3205 where col = cast('127.0.0.1' as inet) order by pos;
select * from tab3205 where col = inet '127.0.0.1' order by pos;
select * from tab3205 where col = inet '127.0.0.1' order by pos;
select * from tab3205 order by pos;
drop tab3205;
