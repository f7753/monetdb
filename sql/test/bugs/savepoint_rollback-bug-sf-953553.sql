START TRANSACTION;

SAVEPOINT a1;
SAVEPOINT a2;
SAVEPOINT a3;

RELEASE SAVEPOINT a1;

ROLLBACK TO SAVEPOINT a2;

ROLLBACK;
