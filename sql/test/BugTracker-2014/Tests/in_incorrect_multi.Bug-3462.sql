SELECT * FROM SYS.ARGS
 WHERE FUNC_ID NOT IN (SELECT * FROM SYS.FUNCTIONS);
SELECT * FROM SYS.ARGS
 WHERE FUNC_ID NOT IN (SELECT FUNC_ID FROM SYS.FUNCTIONS);
