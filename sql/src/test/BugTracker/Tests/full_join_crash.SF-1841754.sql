
CREATE TABLE t1 (name TEXT, n INTEGER);
CREATE TABLE t2 (name TEXT, n INTEGER);
CREATE TABLE t3 (name TEXT, n INTEGER);

INSERT INTO t1 VALUES ( 'aa', 11 );
INSERT INTO t2 VALUES ( 'aa', 12 );
INSERT INTO t2 VALUES ( 'bb', 22 );
INSERT INTO t2 VALUES ( 'dd', 42 );
INSERT INTO t3 VALUES ( 'aa', 13 );
INSERT INTO t3 VALUES ( 'bb', 23 );
INSERT INTO t3 VALUES ( 'cc', 33 );

SELECT * FROM t1 FULL JOIN t2 USING (name);
SELECT * FROM t1 FULL JOIN t2 USING (name) FULL JOIN t3 USING (name);

SELECT * FROM t1 natural FULL JOIN t2 ;
SELECT * FROM t1 natural FULL JOIN t2 natural FULL JOIN t3;

drop table t1;
drop table t2;
drop table t3;
