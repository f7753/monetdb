CREATE TABLE my_table ( my_column INT );
CREATE INDEX my_index ON my_table(my_column);
CREATE INDEX my_index ON my_table(my_column);
DROP INDEX my_index;
CREATE INDEX my_index ON my_table(my_column);
CREATE UNIQUE INDEX my_index ON my_table(my_column);
CREATE UNIQUE INDEX my_u_index ON my_table(my_column);
INSERT INTO my_table VALUES (1);
INSERT INTO my_table VALUES (1);
INSERT INTO my_table VALUES (2);
SELECT * FROM my_table;
drop table my_table;
