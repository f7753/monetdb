-- set debug=4096;
create table RKB( head int(9) unique, tail int(9));
create table RKA( head int(9) unique, tail int(9));
create table tapestry( a0 int(9) unique, a1 int(9));
insert into RKA values(0,0);
insert into RKA values(1,360);
insert into RKA values(1023,864);
insert into RKB select head+0, tail+0 from RKA;
update RKB set tail=(tail*37) % 1024;
update RKB set tail=(tail*7) % 1024;
insert into tapestry select R0.head, R0.tail from RKB R0;
--drop table _tmp;
create table _tmp( a0 int(9) unique, a1 int(9));
insert into _tmp select * from tapestry where a1>=0 and a1 <=1;
delete from _tmp;
insert into _tmp select * from tapestry where a1>=0 and a1 <=1;
delete from _tmp;
