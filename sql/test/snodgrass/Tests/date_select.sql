-- `<  ',  ` <  = ',  `>  ',  `>=   ', and`<>
SELECT DATE '1996-02-24' + INTERVAL '7' DAY = DATE '1996-03-02';
SELECT DATE '1996-02-24' + INTERVAL '12:30' HOUR TO MINUTE; -- err: disallowed
SELECT DATE '1996-02-24' + INTERVAL '2 12' DAY TO HOUR; -- err: disallowed
SELECT INTERVAL '7' DAY + DATE '1996-02-24' = DATE '1996-03-02';
SELECT DATE '1996-03-02' - INTERVAL '7' DAY = DATE '1996-02-24';
-- SELECT TIMESTAMP '1996-02-24 12:34:56' AT LOCAL;  -- MST: TIMESTAMP '1996-02-24 19:34:56' -- fixme
-- SELECT TIMESTAMP '1996-02-24 12:34:56+02:00' AT LOCAL;  -- (MET DST) MST: TIMESTAMP '1996-02-24 03:34:56' -- fixme
-- SELECT TIMESTAMP '1996-02-24 12:34:56' AT TIME ZONE INTERVAL '-7:00' HOUR TO MINUTE = TIMESTAMP '1996-02-24 19:34:56'; -- fixme
-- SELECT CURRENT_DATE;
-- SELECT CURRENT_TIME;
-- SELECT CURRENT_TIMESTAMP;
