#!/usr/bin/perl -w

# The contents of this file are subject to the MonetDB Public
# License Version 1.0 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at
# http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Monet Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-2005 CWI.
# All Rights Reserved.

# The MonetDB DBI driver implementation. New version based on the MapiLib SWIG.
# by Arjan Scherpenisse <acscherp@science.uva.nl>

package DBD::monetdb;
use strict;

use DBI;
use Carp;
use vars qw($VERSION $drh);
use sigtrap;
# use Data::Dump qw(dump);

$VERSION = '0.03';
$drh = undef;

sub driver {
    return $drh if $drh;

    my $class = shift;
    my $attr  = shift;
    $class .= '::dr';

    $drh = DBI::_new_drh($class, {
				  Name        => 'monetdb',
				  Version     => $VERSION,
				  Attribution => 'DBD::monetdb derived from monetdb.pm by Arjan Scherpenisse',
				 }, {});
}

# The monetdb dsn structure is DBI:monetdb:host:port:dbname:language
sub _parse_dsn {
    my $class = shift;
    my ($dsn, $args) = @_;
    my($hash, $var, $val);
    return if ! defined $dsn;

    while (length $dsn) {
	if ($dsn =~ /([^:;]*)[:;](.*)/) {
	    $val = $1;
	    $dsn = $2;
	} else {
	    $val = $dsn;
	    $dsn = '';
	}
	if ($val =~ /([^=]*)=(.*)/) {
	    $var = $1;
	    $val = $2;
	    if ($var eq 'hostname' || $var eq 'host') {
		$hash->{'host'} = $val;
	    } elsif ($var eq 'db' || $var eq 'dbname') {
		$hash->{'database'} = $val;
	    } else {
		$hash->{$var} = $val;
	    }
	} else {
	    for $var (@$args) {
		if (!defined($hash->{$var})) {
		    $hash->{$var} = $val;
		    last;
		}
	    }
	}
    }
    return $hash;
}


sub _parse_dsn_host {
    my($class, $dsn) = @_;
    my $hash = $class->_parse_dsn($dsn, ['host', 'port']);
    ($hash->{'host'}, $hash->{'port'});
}



package DBD::monetdb::dr;

$DBD::monetdb::dr::imp_data_size = 0;

use MapiLib;
use strict;


sub connect {
    my ($drh, $dsn, $user, $password, $attr) = @_;

    my $data_source_info = DBD::monetdb->_parse_dsn(
        $dsn, ['host','port','database','language']);

    my $lang  = $data_source_info->{language} || 'sql';
    die "!ERROR languages permitted are 'sql', 'mal', and 'mil'\n"
        unless ($lang eq 'mal' || $lang eq 'sql' || $lang eq 'mil');
    my $host  = $data_source_info->{host} || 'localhost';
    my $port  = $data_source_info->{port} || ( $lang eq 'sql' ? 45123 : 50000 );
    $user     ||= 'monetdb';
    $password ||= 'monetdb';

    my $mapi = MapiLib::mapi_connect($host, $port, $user, $password, $lang);
    return $drh->set_err(-1,'Undefined Mapi handle') unless $mapi;
    my $err = MapiLib::mapi_error($mapi);
    return $drh->set_err($err, MapiLib::mapi_error_str($mapi)) if $err;

    my ($outer, $dbh) = DBI::_new_dbh($drh, { Name => $dsn });

    $dbh->STORE('Active', 1 );

    $dbh->{monetdb_connection} = $mapi;
    $dbh->{monetdb_language} = $lang;

    return $outer;
}


sub data_sources {
    return ('dbi:monetdb:');
}



package DBD::monetdb::db;

$DBD::monetdb::db::imp_data_size = 0;
use MapiLib;
use strict;


sub ping {
    my $dbh = shift;
    my $mapi = $dbh->{monetdb_connection};

    MapiLib::mapi_ping($mapi) ? 0 : 1;
}

sub quote {
    my $dbh = shift;
    my ($statement, $type) = @_;
    return 'NULL' unless defined $statement;

    for ($statement) {
	s/	/\\t/g;
	s/\\/\\\\/g;
	s/\n/\\n/g;
	s/\r/\\r/g;
	s/"/\\"/g;
	s/'/''/g;
    }
    return "'$statement'";
}

sub _count_param {
    my @statement = split //, shift;
    my $num = 0;

    while (defined(my $c = shift @statement)) {
	if ($c eq '"' || $c eq "'") {
	    my $end = $c;
	    while (defined(my $c = shift @statement)) {
		last if $c eq $end;
		@statement = splice @statement, 2 if $c eq '\\';
	    }
	} elsif ($c eq '?') {
	    $num++;
	}
    }
    return $num;
}


sub prepare {
    my ($dbh, $statement, $attr) = @_;

    my $mapi = $dbh->{monetdb_connection};
    my $hdl = MapiLib::mapi_new_handle($mapi);
    my $err = MapiLib::mapi_error($mapi);
    return $dbh->set_err($err, MapiLib::mapi_error_str($mapi)) if $err;

    my ($outer, $sth) = DBI::_new_sth($dbh, { Statement => $statement });

    $sth->STORE('NUM_OF_PARAMS', _count_param($statement));

    $sth->{monetdb_hdl} = $hdl;
    $sth->{monetdb_params} = [];
    $sth->{monetdb_types} = [];
    $sth->{monetdb_rows} = -1;

    return $outer;
}


sub commit {
    my $dbh = shift;
    if ($dbh->FETCH('AutoCommit')) {
	warn 'Commit ineffective while AutoCommit is on';
    } else {
        MapiLib::mapi_query($dbh->{monetdb_connection},'commit;');
    }
    1;
}


sub rollback {
    my $dbh = shift;
    if ($dbh->FETCH('AutoCommit')) {
	warn 'Rollback ineffective while AutoCommit is on';
    } else {
        MapiLib::mapi_query($dbh->{monetdb_connection}, ($dbh->{monetdb_language} ne 'sql') ? 'abort;' : 'rollback;');
    }
    1;
}


sub get_info {
    my($dbh, $info_type) = @_;
    require DBD::monetdb::GetInfo;
    my $v = $DBD::monetdb::GetInfo::info{int($info_type)};
    $v = $v->($dbh) if ref $v eq 'CODE';
    return $v;
}


sub monetdb_catalog_info {
    my($dbh) = @_;
    my $sql = <<'SQL';
select cast( null as varchar( 128 ) ) as table_cat
     , cast( null as varchar( 128 ) ) as table_schem
     , cast( null as varchar( 128 ) ) as table_name
     , cast( null as varchar( 254 ) ) as table_type
     , cast( null as varchar( 254 ) ) as remarks
 where 0 = 1
 order by table_cat
SQL
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute or return;
    return $sth;
}


sub monetdb_schema_info {
    my($dbh) = @_;
    my $sql = <<'SQL';
select cast( null as varchar( 128 ) ) as table_cat
     , "name"                         as table_schem
     , cast( null as varchar( 128 ) ) as table_name
     , cast( null as varchar( 254 ) ) as table_type
     , cast( null as varchar( 254 ) ) as remarks
  from sys."schemas"
 order by table_schem
SQL
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute or return;
    return $sth;
}


my $ttp = {
 'TABLE'           => 't."istable" = true  and t."system" = false and t."temporary" = 0'
,'SYSTEM TABLE'    => 't."istable" = true  and t."system" = true  and t."temporary" = 0'
,'LOCAL TEMPORARY' => 't."istable" = true  and t."system" = false and t."temporary" = 1'
,'VIEW'            => 't."istable" = false                                             '
};


sub monetdb_tabletype_info {
    my($dbh) = @_;
    my $sql = <<"SQL";
select distinct
       cast( null as varchar( 128 ) ) as table_cat
     , cast( null as varchar( 128 ) ) as table_schem
     , cast( null as varchar( 128 ) ) as table_name
     , case
         when $ttp->{'TABLE'          } then cast('TABLE'               as varchar( 254 ) )
         when $ttp->{'SYSTEM TABLE'   } then cast('SYSTEM TABLE'        as varchar( 254 ) )
         when $ttp->{'LOCAL TEMPORARY'} then cast('LOCAL TEMPORARY'     as varchar( 254 ) )
         when $ttp->{'VIEW'           } then cast('VIEW'                as varchar( 254 ) )
         else                                cast('INTERNAL TABLE TYPE' as varchar( 254 ) )
       end                            as table_type
     , cast( null as varchar( 254 ) ) as remarks
  from sys."tables" t
 order by table_type
SQL
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute or return;
    return $sth;
}


sub monetdb_table_info {
    my($dbh, $c, $s, $t, $tt) = @_;
    my $sql = <<"SQL";
select cast( null as varchar( 128 ) ) as table_cat
     , s."name"                       as table_schem
     , t."name"                       as table_name
     , case
         when $ttp->{'TABLE'          } then cast('TABLE'               as varchar( 254 ) )
         when $ttp->{'SYSTEM TABLE'   } then cast('SYSTEM TABLE'        as varchar( 254 ) )
         when $ttp->{'LOCAL TEMPORARY'} then cast('LOCAL TEMPORARY'     as varchar( 254 ) )
         when $ttp->{'VIEW'           } then cast('VIEW'                as varchar( 254 ) )
         else                                cast('INTERNAL TABLE TYPE' as varchar( 254 ) )
       end                            as table_type
     , cast( null as varchar( 254 ) ) as remarks
  from sys."schemas" s
     , sys."tables"  t
 where t."schema_id" = s."id"
SQL
    my @bv = ();
    $sql .= qq(   and s."name"   like ?\n), push @bv, $s if $s;
    $sql .= qq(   and t."name"   like ?\n), push @bv, $t if $t;
    if ( @$tt ) {
        $sql .= "   and ( 1 = 0\n";
        for ( @$tt ) {
            my $p = $ttp->{uc $_};
            $sql .= "      or $p\n" if $p;
        }
        $sql .= "       )\n";
    }
    $sql .=   " order by table_type, table_schem, table_name\n";
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute(@bv) or return;
    $dbh->set_err(0,"Catalog parameter '$c' ignored") if defined $c;
    return $sth;
}


sub table_info {
    my($dbh, $c, $s, $t, $tt) = @_;
    if ( defined $c && defined $s && defined $t ) {
        if    ( $c eq '%' && $s eq ''  && $t eq '') {
            return monetdb_catalog_info($dbh);
        }
        elsif ( $c eq ''  && $s eq '%' && $t eq '') {
            return monetdb_schema_info($dbh);
        }
        elsif ( $c eq ''  && $s eq ''  && $t eq '' && defined $tt && $tt eq '%') {
            return monetdb_tabletype_info($dbh);
        }
    }
    my @tt;
    if ( defined $tt ) {
        @tt = split /,/, $tt;
        s/^\s*'?//, s/'?\s*$// for @tt;
    }
    return monetdb_table_info($dbh, $c, $s, $t, \@tt);
}


sub column_info {
    my($dbh, $catalog, $schema, $table, $column) = @_;
    my $sql = <<'SQL';
select cast( null            as varchar( 128 ) ) as table_cat
     , s."name"                                  as table_schem
     , t."name"                                  as table_name
     , c."name"                                  as column_name
     , cast( 0               as smallint       ) as data_type          -- TODO
     , c."type"                                  as type_name          -- TODO
     , cast( c."type_digits" as integer        ) as column_size        -- TODO
     , cast( null            as integer        ) as buffer_length      -- TODO
     , cast( c."type_scale"  as smallint       ) as decimal_digits     -- TODO
     , cast( null            as smallint       ) as num_prec_radix     -- TODO
     , case c."null"
         when false then cast( 0 as smallint )  -- SQL_NO_NULLS
         when true  then cast( 1 as smallint )  -- SQL_NULLABLE
       end                                       as nullable
     , cast( null            as varchar( 254 ) ) as remarks
     , c."default"                               as column_def
     , cast( 0               as smallint       ) as sql_data_type      -- TODO
     , cast( null            as smallint       ) as sql_datetime_sub   -- TODO
     , cast( null            as integer        ) as char_octet_length  -- TODO
     , cast( c."number" + 1  as integer        ) as ordinal_position
     , case c."null"
         when false then cast('NO'  as varchar( 254 ) )
         when true  then cast('YES' as varchar( 254 ) )
       end                                       as is_nullable
  from sys."schemas" s
     , sys."tables"  t
     , sys."columns" c
 where t."schema_id" = s."id"
   and c."table_id"  = t."id"
SQL
    my @bv = ();
    $sql .= qq(   and s."name"   like ?\n), push @bv, $schema if $schema;
    $sql .= qq(   and t."name"   like ?\n), push @bv, $table  if $table;
    $sql .= qq(   and c."name"   like ?\n), push @bv, $column if $column;
    $sql .=   " order by table_cat, table_schem, table_name, ordinal_position\n";
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute(@bv) or return;
    $dbh->set_err(0,"Catalog parameter '$catalog' ignored") if defined $catalog;
    return $sth;
}


sub primary_key_info {
    my($dbh, $catalog, $schema, $table) = @_;
    return $dbh->set_err(-1,'Undefined schema','HY009') unless defined $schema;
    return $dbh->set_err(-1,'Undefined table' ,'HY009') unless defined $table;
    my $sql = <<'SQL';
select cast( null        as varchar( 128 ) ) as table_cat
     , s."name"                              as table_schem
     , t."name"                              as table_name
     , c."column"                            as column_name
     , cast( c."nr" + 1  as smallint       ) as key_seq
     , k."name"                              as pk_name
  from sys."schemas"     s
     , sys."tables"      t
     , sys."keys"        k
     , sys."keycolumns"  c
 where t."schema_id"   = s."id"
   and k."table_id"    = t."id"
   and c."id"          = k."id"
   and s."name"        = ?
   and t."name"        = ?
   and k."type"        = 0
 order by table_schem, table_name, key_seq
SQL
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute($schema, $table) or return;
    $dbh->set_err(0,"Catalog parameter '$catalog' ignored") if defined $catalog;
    return $sth;
}


sub foreign_key_info {
    my($dbh, $c1, $s1, $t1, $c2, $s2, $t2) = @_;
    my $sql = <<'SQL';
select cast( null         as varchar( 128 ) ) as uk_table_cat
     , uks."name"                             as uk_table_schem
     , ukt."name"                             as uk_table_name
     , ukc."column"                           as uk_column_name
     , cast( null         as varchar( 128 ) ) as fk_table_cat
     , fks."name"                             as fk_table_schem
     , fkt."name"                             as fk_table_name
     , fkc."column"                           as fk_column_name
     , cast( fkc."nr" + 1 as smallint       ) as ordinal_position
     , cast( 3            as smallint       ) as update_rule    -- SQL_NO_ACTION
     , cast( 3            as smallint       ) as delete_rule    -- SQL_NO_ACTION
     , fkk."name"                             as fk_name
     , ukk."name"                             as uk_name
     , cast( 7            as smallint       ) as deferability   -- SQL_NOT_DEFERRABLE
     , case  ukk."type"
         when 0 then cast('PRIMARY'   as varchar( 7 ) )
         when 1 then cast('UNIQUE'    as varchar( 7 ) )
         else        cast( ukk."type" as varchar( 7 ) )
       end                                    as unique_or_primary
  from sys."schemas"    uks
     , sys."tables"     ukt
     , sys."keys"       ukk
     , sys."keycolumns" ukc
     , sys."schemas"    fks
     , sys."tables"     fkt
     , sys."keys"       fkk
     , sys."keycolumns" fkc
 where ukt."schema_id"  = uks."id"
   and ukk."table_id"   = ukt."id"
   and ukc."id"         = ukk."id"
   and fkt."schema_id"  = fks."id"
   and fkk."table_id"   = fkt."id"
   and fkc."id"         = fkk."id"
-- and ukk."type"      IN ( 0, 1 )
-- and fkk."type"       = 2
-- and fkk."rkey"       > -1
   and fkk."rkey"       = ukk."id"
   and fkc."nr"         = ukc."nr"
SQL
    my @bv = ();
    $sql .= qq(   and uks."name"       = ?\n), push @bv, $s1 if $s1;
    $sql .= qq(   and ukt."name"       = ?\n), push @bv, $t1 if $t1;
    $sql .= qq(   and fks."name"       = ?\n), push @bv, $s2 if $s2;
    $sql .= qq(   and fkt."name"       = ?\n), push @bv, $t2 if $t2;
    $sql .= qq(   and ukk."type"       = 0\n)                if $t1 && !$t2;
    $sql .= " order by uk_table_schem, uk_table_name, fk_table_schem, fk_table_name, ordinal_position\n";
    my $sth = $dbh->prepare($sql) or return;
    $sth->execute(@bv) or return;
    $dbh->set_err(0,"Catalog parameter '$c1' ignored") if defined $c1;
    $dbh->set_err(0,"Catalog parameter '$c2' ignored") if defined $c2;
    return $sth;
}


sub type_info_all {
    my ($dbh) = @_;
    require DBD::monetdb::TypeInfo;
    return $DBD::monetdb::TypeInfo::type_info_all;
}


sub tables {
    my $dbh = shift;
    my @args = @_;
    my $mapi = $dbh->{monetdb_connection};

    my @table_list;

    my $hdl = MapiLib::mapi_query($mapi, ($dbh->{monetdb_language} ne 'sql') ? 'ls;' : 'SELECT name FROM tables;');
    die MapiLib::mapi_error_str($mapi) if MapiLib::mapi_error($mapi);

    while (MapiLib::mapi_fetch_row($hdl)) {
	push @table_list, MapiLib::mapi_fetch_field($hdl, 0);
    }
    return MapiLib::mapi_error($mapi)
      ? undef
	: @table_list;
}


sub _ListDBs {
    my $dbh = shift;
    my @database_list;
    push @database_list, MapiLib::mapi_get_dbname($dbh->{monetdb_connection});
    return @database_list;
}


sub _ListTables {
    my $dbh = shift;
    return $dbh->tables;
}


sub disconnect {
    my $dbh = shift;
    my $mapi = $dbh->{monetdb_connection};
    MapiLib::mapi_disconnect($mapi);
    $dbh->STORE('Active', 0 );
    return 1;
}


sub FETCH {
    my $dbh = shift;
    my $key = shift;
    return $dbh->{$key} if $key =~ /^monetdb_/;
    return $dbh->SUPER::FETCH($key);
}


sub STORE {
    my $dbh = shift;
    my ($key, $new) = @_;

    if ($key eq 'AutoCommit') {
	my $old = $dbh->{$key} || 0;
	if ($new != $old) {
	    my $mapi = $dbh->{monetdb_connection};
	    MapiLib::mapi_setAutocommit($mapi, $new);
	    $dbh->{$key} = $new;
	}
	return 1;

    } elsif ($key =~ /^monetdb_/) {
	$dbh->{$key} = $new;
	return 1;
    }
    return $dbh->SUPER::STORE($key, $new);
}


sub DESTROY {
    my $dbh = shift;
    $dbh->disconnect if $dbh->FETCH('Active');
    my $mapi = $dbh->{monetdb_connection};
    MapiLib::mapi_destroy($mapi) if $mapi;
}


package DBD::monetdb::st;

use DBI qw(:sql_types);
use MapiLib;


$DBD::monetdb::st::imp_data_size = 0;


sub bind_param {
    my ($sth, $index, $value, $attr) = @_;
    $sth->{monetdb_params}[$index-1] = $value;
    $sth->{monetdb_types}[$index-1] = ref $attr ? $attr->{TYPE} : $attr;
    return 1;
}


sub execute {
    my($sth, @bind_values) = @_;
    my $statement = $sth->{Statement};
    my $dbh = $sth->{Database};

    $sth->STORE('Active', 0 );  # we don't need to call $sth->finish because
                                # mapi_query_handle() calls finish_handle()

    $sth->bind_param($_, $bind_values[$_-1]) or return for 1 .. @bind_values;

    my $params = $sth->{monetdb_params};
    my $num_of_params = $sth->FETCH('NUM_OF_PARAMS');
    return $sth->set_err(-1, @$params ." values bound when $num_of_params expected")
        unless @$params == $num_of_params;

    for ( 1 .. $num_of_params ) {
        # TODO: parameter type
        my $quoted_param = $dbh->quote($params->[$_-1]);
        $statement =~ s/\?/$quoted_param/;  # TODO: '?' inside quotes/comments
    }
    $sth->trace_msg("    -- Statement: $statement\n", 5);

    my $mapi = $dbh->{monetdb_connection};
    my $hdl = $sth->{monetdb_hdl};
    MapiLib::mapi_query_handle($hdl, $statement);
    my $err = MapiLib::mapi_error($mapi);
    return $sth->set_err($err, MapiLib::mapi_error_str($mapi)) if $err;
    my $result_error = MapiLib::mapi_result_error($hdl);
    return $sth->set_err(-1, $result_error) if $result_error;

    my $rows = MapiLib::mapi_rows_affected($hdl);

    if ( MapiLib::mapi_get_querytype($hdl) != 3 && $dbh->{monetdb_language} eq 'sql') {
        $sth->{monetdb_rows} = $rows;
        return $rows || '0E0';
    }
    my ( @names, @types, @precisions, @nullables );
    my $field_count = MapiLib::mapi_get_field_count($hdl);
    for ( 0 .. $field_count-1 ) {
        push @names     , MapiLib::mapi_get_name($hdl, $_);
        push @types     , MapiLib::mapi_get_type($hdl, $_);
        push @precisions, MapiLib::mapi_get_len ($hdl, $_);
        push @nullables , 0;  # TODO
    }
    $sth->STORE('NUM_OF_FIELDS', $field_count) unless $sth->FETCH('NUM_OF_FIELDS');
    $sth->STORE('NAME'         , \@names     );
#   $sth->STORE('TYPE'         , \@types     );  # TODO: monetdb2dbi
#   $sth->STORE('PRECISION'    , \@precisions);  # TODO
#   $sth->STORE('NULLABLE'     , \@nullables );  # TODO
    $sth->STORE('Active', 1 );

    $sth->{monetdb_rows} = 0;

    return $rows || '0E0';
}


sub fetch {
    my ($sth) = @_;
    return $sth->set_err(-900,'Statement handle not marked as Active')
      unless $sth->FETCH('Active');
    my $hdl = $sth->{monetdb_hdl};
    my $field_count = MapiLib::mapi_fetch_row($hdl);
    unless ( $field_count ) {
        $sth->STORE('Active', 0 );
        my $mapi = $sth->{Database}{monetdb_connection};
        my $err = MapiLib::mapi_error($mapi);
        $sth->set_err($err, MapiLib::mapi_error_str($mapi)) if $err;
        return;
    }
    my @row = map MapiLib::mapi_fetch_field($hdl, $_), 0 .. $field_count-1;
    map { s/\s+$// } @row if $sth->FETCH('ChopBlanks');

    $sth->{monetdb_rows}++;
    return $sth->_set_fbav(\@row);
}

*fetchrow_arrayref = \&fetch;

sub rows {
    my $sth = shift;
    return $sth->{monetdb_rows};
}


sub finish {
    my ($sth) = @_;
    my $hdl = $sth->{monetdb_hdl};
    if ( MapiLib::mapi_finish($hdl) ) {
        my $mapi = $sth->{Database}{monetdb_connection};
        my $err = MapiLib::mapi_error($mapi) || -1;
        return $sth->set_err($err, MapiLib::mapi_error_str($mapi));
    }
    return $sth->SUPER::finish;  # sets Active off
}


sub FETCH {
    my $sth = shift;
    my $key = shift;

    return $sth->{NAME} if $key eq 'NAME';
    return $sth->{$key} if $key =~ /^monetdb_/;
    return $sth->SUPER::FETCH($key);
}


sub STORE {
    my $sth = shift;
    my ($key, $value) = @_;

    if ($key eq 'NAME') {
	$sth->{NAME} = $value;
	return 1;
    } elsif ($key =~ /^monetdb_/) {
	$sth->{$key} = $value;
	return 1;
    }
    return $sth->SUPER::STORE($key, $value);
}


sub DESTROY {
    my ($sth) = @_;
    $sth->STORE('Active', 0 );  # we don't need to call $sth->finish because
                                # mapi_close_handle() calls finish_handle()
    MapiLib::mapi_close_handle($sth->{monetdb_hdl}) if $sth->{monetdb_hdl};
}


1;
__END__

=head1 NAME

DBD::monetdb - MonetDB Driver for DBI

=head1 SYNOPSIS

  use DBI();

  my $dbh = DBI->connect('dbi:monetdb:');

  my $sth = $dbh->prepare('SELECT * FROM env');
  $sth->execute;
  $sth->dump_results;

=head1 EXAMPLE

  #!/usr/bin/perl

  use strict;
  use DBI;

  # Connect to the database.
  my $dbh = DBI->connect('dbi:monetdb:host=localhost',
    'joe', "joe's password", { RaiseError => 1 } );

  # Drop table 'foo'. This may fail, if 'foo' doesn't exist.
  # Thus we put an eval around it.
  eval { $dbh->do('DROP TABLE foo') };
  print "Dropping foo failed: $@\n" if $@;

  # Create a new table 'foo'. This must not fail, thus we don't
  # catch errors.
  $dbh->do('CREATE TABLE foo (id INTEGER, name VARCHAR(20))');

  # INSERT some data into 'foo'. We are using $dbh->quote() for
  # quoting the name.
  $dbh->do('INSERT INTO foo VALUES (1, ' . $dbh->quote('Tim') . ')');

  # Same thing, but using placeholders
  $dbh->do('INSERT INTO foo VALUES (?, ?)', undef, 2, 'Jochen');

  # Now retrieve data from the table.
  my $sth = $dbh->prepare('SELECT id, name FROM foo');
  $sth->execute;
  while ( my $row = $sth->fetch ) {
    print "Found a row: id = $row->[0], name = $row->[1]\n";
  }

  # Disconnect from the database.
  $dbh->disconnect;

=head1 DESCRIPTION

DBD::monetdb is a Pure Perl client interface for the MonetDB Database Server.
However, it requires the SWIG generated MapiLib - a wrapper module for
libMapi.

From perl you activate the interface with the statement

  use DBI;

After that you can connect to multiple MonetDB database servers
and send multiple queries to any of them via a simple object oriented
interface. Two types of objects are available: database handles and
statement handles. Perl returns a database handle to the connect
method like so:

  $dbh = DBI->connect("dbi:monetdb:host=$host",
    $user, $password, { RaiseError => 1 } );

Once you have connected to a database, you can can execute SQL
statements with:

  my $sql = sprintf('INSERT INTO foo VALUES (%d, %s)',
    $number, $dbh->quote('name'));
  $dbh->do($sql);

See L<DBI> for details on the quote and do methods. An alternative
approach is

  $dbh->do('INSERT INTO foo VALUES (?, ?)', undef, $number, $name);

in which case the quote method is executed automatically. See also
the bind_param method in L<DBI>.

If you want to retrieve results, you need to create a so-called
statement handle with:

  $sth = $dbh->prepare("SELECT id, name FROM $table");
  $sth->execute;

This statement handle can be used for multiple things. First of all
you can retreive a row of data:

  my $row = $sth->fetch;

If your table has columns ID and NAME, then $row will be array ref with
index 0 and 1.

=head2 Class Methods

=over

=item B<connect>

  use DBI();

  $dsn = 'dbi:monetdb:';
  $dsn = "dbi:monetdb:host=$host";
  $dsn = "dbi:monetdb:host=$host;port=$port";

  $dbh = DBI->connect($dsn, $user, $password);

=over

=item host

The default host to connect to is 'localhost', i.e. your workstation.

=item port

The port where MonetDB daemon listens to. Default for SQL is 45123.

=back

=back

=head2 MetaData Methods

All MetaData methods are supported. However, column_info() currently doesn't
provide much datatype related information.
The foreign_key_info() method returns a SQL/CLI like result set,
because it provides additional information about unique keys.

=head2 Private MetaData Methods

=over 4

=item ListDBs

    @dbs = $dbh->func('_ListDBs');

Returns a list of all databases managed by the MonetDB SQL daemon.

=item ListTables

B<WARNING>: This method is obsolete due to DBI's $dbh->tables().

    @tables = $dbh->func('_ListTables');

Once connected to the desired database on the desired MonetDB server with the
DBI connect() method, we may extract a list of the tables that have been
created within that database.

"ListTables" returns an array containing the names of all the tables present
within the selected database. If no tables have been created, an empty list
is returned.

    @tables = $dbh->func('_ListTables');
    foreach $table (@tables) {
        print "Table: $table\n";
    }

=back

=head1 HANDLE ATTRIBUTES

The following attributes are currently not supported:

  LongReadLen
  LongTruncOk

=head2 Database Handle Attributes

The following attributes are currently not supported:

  RowCacheSize

=head2 Statement Handle Attributes

The following attributes are currently not supported:

  TYPE
  PRECISION
  SCALE
  NULLABLE
  CursorName
  RowsInCache

=head1 OPERATING SYSTEM SUPPORT

This module has not been tested on all these OSes.

=over 4

=item * MacOS 9.x

TODO with MacPerl5.6.1r.

=item * MacOS X

TODO with perl5.6.0 build for darwin.

=item * Windows2000

TODO with ActivePerl5.6.1 build631.

=item * FreeBSD 3.4 and 4.x

TODO with perl5.6.1 build for i386-freebsd.

TODO with perl5.005_03 build for i386-freebsd.

=back

=head1 AUTHORS

Martin Kersten <lt>Martin.Kersten@cwi.nl<gt>

=head1 COPYRIGHT AND LICENCE

Copyright (C) 2003-2004 CWI Netherlands. All rights reserved.

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=head1 SEE ALSO

=head2 MonetDB

  Homepage    : http://monetdb.cwi.nl
  SourceForge : http://sourceforge.net/projects/monetdb

=head2 Perl modules

L<DBI>, L<MapiLib>, L<DBD::monetdbPP>

=cut
