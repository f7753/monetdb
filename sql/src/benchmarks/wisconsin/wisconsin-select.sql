-- select *  from tenk1 where (unique2 > 301) and (unique2 < 402);
select *  from tenk1 where unique2 between 301 and 402;
-- select *  from tenk1 where (unique1 > 647) and (unique1 < 1648);
select *  from tenk1 where unique1 between 647 and 1648;
select * from tenk1 where unique2 = 2001;
-- select * from tenk1 where (unique2 > 301) and (unique2 < 402);
select *  from tenk1 where unique2 between 301 and 402;
select t1.*, t2.unique1 AS t2unique1, t2.unique2 AS t2unique2, t2.two AS t2two, t2.four AS t2four, t2.ten AS t2ten, t2.twenty AS t2twenty, t2.hundred AS t2hundred, t2.thousand AS t2thousand, t2.twothousand AS t2twothousand, t2.fivethous AS t2fivethous, t2.tenthous AS t2tenthous, t2.odd AS t2odd, t2.even AS t2even, t2.stringu1 AS t2stringu1, t2.stringu2 AS t2stringu2, t2.string4 AS t2string4  from tenk1 t1, tenk1 t2 where (t1.unique2 = t2.unique2) and (t2.unique2 < 1000);
select t.*,B.unique1 AS Bunique1,B.unique2 AS Bunique2,B.two AS Btwo,B.four AS Bfour,B.ten AS Bten,B.twenty AS Btwenty,B.hundred AS Bhundred,B.thousand AS Bthousand,B.twothousand AS Btwothousand,B.fivethous AS Bfivethous,B.tenthous AS Btenthous,B.odd AS Bodd,B.even AS Beven,B.stringu1 AS Bstringu1,B.stringu2 AS Bstringu2,B.string4 AS Bstring4  from tenk1 t, Bprime B where t.unique2 = B.unique2;
select t1.*,o.unique1 AS ounique1,o.unique2 AS ounique2,o.two AS otwo,o.four AS ofour,o.ten AS oten,o.twenty AS otwenty,o.hundred AS ohundred,o.thousand AS othousand,o.twothousand AS otwothousand,o.fivethous AS ofivethous,o.tenthous AS otenthous,o.odd AS oodd, o.even AS oeven,o.stringu1 AS ostringu1,o.stringu2 AS ostringu2,o.string4 AS ostring4  from onek o, tenk1 t1, tenk1 t2 where (o.unique2 = t1.unique2) and (t1.unique2 = t2.unique2) and (t1.unique2 < 1000) and (t2.unique2 < 1000);
select two, four, ten, twenty, hundred, string4  from tenk1;
select *  from onek;
select MIN(unique2) as x  from tenk1;
