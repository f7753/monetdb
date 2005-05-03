#!perl -I./t

$| = 1;

use strict;
use warnings;
use DBI();
use DBD_TEST();

use Test::More;

if (defined $ENV{DBI_DSN}) {
  plan tests => 30;
} else {
  plan skip_all => 'Cannot test without DB info';
}

pass('MIL tests');

$ENV{DBI_DSN} .= ';language=mil';

my $dbh = DBI->connect or die "Connect failed: $DBI::errstr\n";
pass('Database connection created');

$dbh->{AutoCommit} = 0;

for ( 7 .. 9 )
{
  my $sth = $dbh->prepare("print( $_ );");
  ok( defined $sth,'Statement handle defined');
  ok( $sth->execute,'execute');
  my $row = $sth->fetch;
  is( $#$row, 0,'last index');
  is( $row->[0], $_,'field 0');
}
{
  local $dbh->{PrintError} = 0;
  my $sth = $dbh->prepare('( xyz 1);');
  ok(!$sth->execute,'execute');
  like( $sth->errstr, qr/!ERROR:/,'Error expected');
}
ok( $dbh->do( $_ ), $_) for 'var b := new( int, str );';
ok( $dbh->do( $_ ), $_) for 'insert( b, 3,"T3");';
{
  my $sth = $dbh->prepare('insert( b, ?, ? );');
  ok( defined $sth,'Statement handle defined');
  ok( $sth->bind_param( 1,  7 , DBI::SQL_INTEGER() ),'bind');
  ok( $sth->bind_param( 2,'T7' ),'bind');
  ok( $sth->execute,'execute');
}
{
  my $sth = $dbh->prepare('print( b );');
  ok( defined $sth,'Statement handle defined');
  ok( $sth->execute,'execute');
  for ( 3, 7 )
  {
    my $row = $sth->fetch;
    is( $row->[0],  $_ ,"fetch  $_");
    is( $row->[1],"T$_","fetch T$_");
  }
}
ok( $dbh->rollback,'Rollback');
ok( $dbh->disconnect,'Disconnect');
