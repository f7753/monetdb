select timestamp '2008-03-01 00:00';
select cast( '10' as interval hour);

select * from generate_series(
	timestamp '2008-03-01 00:00',
	timestamp '2008-03-04 12:00',
	cast( '10' as interval hour));

select * from generate_series(
	timestamp '2008-03-01 00:00',
	timestamp '2008-03-04 12:00',
	cast( '1' as interval day));

select * from generate_series(
	timestamp '2008-03-01 00:00',
	timestamp '2008-03-04 12:00',
	cast( '10' as interval hour))
where value < timestamp '2008-03-03 00:00';

select * from generate_series(
	timestamp '2008-03-01 00:00',
	timestamp '2008-03-04 12:00',
	cast( '10' as interval hour))
where value > timestamp '2008-03-01 00:00'
and value < timestamp '2008-03-22 00:00';

