#!/usr/bin/perl -w
# Copyright Notice:
# -----------------
# 
# The contents of this file are subject to the Pathfinder Public License
# compliance with the License.  You may obtain a copy of the License at
# http://monetdb.cwi.nl/Legal/PathfinderLicense-1.1.html
# 
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# 
# The Original Code is the Pathfinder system.
# 
# The Original Code has initially been developed by the Database &
# Information Systems Group at the University of Konstanz, Germany and
# is now maintained by the Database Systems Group at the Technische
# Universitaet Muenchen, Germany.  Portions created by the University of
# Konstanz and the Technische Universitaet Muenchen are Copyright (C)
# 2000-2005 University of Konstanz and (C) 2005-2008 Technische
# Universitaet Muenchen, respectively.  All Rights Reserved.
# $Id$

use strict;
use warnings;
use Getopt::Long; # support long options

# we require Perl's database interfaces
use DBI;

######### DBI wrappers ###########

# connect to database
sub db_connect {
   my $dbh = shift;
   my $dns = shift;

   # connect to the database
   # use available connections if available
   $$dbh = DBI->connect_cached ($dns)
       or die "Could not connect to database with DSN `$dns': " . DBI->errstr;
}

# prepare statement 
sub stmt_prepare {
   my $dbh = shift;
   my $stmt = shift;
   my $final_query = shift;

   # prepare statement 
   $$stmt = $dbh->prepare ($final_query)
       or die "Could not prepare query :" . DBI->errstr;
}

# execute statement wrapper 
sub stmt_execute {
    my $stmt = shift; 
    $stmt->execute () or 
      die "Execute not successful: " . DBI->errstr ;
}

# bind the columns to the corresponding map 
sub build_result_map { 

    my $schema     = shift;
    my $result_map = {};
    my $querytype;
  

    $querytype = $schema->{'Type'};

    # check if query type is known
    die "Query type unknown"
        unless (($querytype eq 'ATOMIC_AND_NODES')
                || ($querytype eq 'ATOMIC_ONLY')
                || ($querytype eq 'NODES_ONLY'));

    # if query contains atomic values
    # collect them
    if (($querytype eq 'ATOMIC_AND_NODES')
        || ($querytype eq 'ATOMIC_ONLY')) {
        foreach my $t ('int', 'str', 'dbl', 'dec', 'bool') {
            if (defined $schema->{"item_$t"}) {
                $result_map->{"item_$t"} = $schema->{"item_$t"};
            }
        }
    }

    # if query contain nodes we have even to bind
    # the additional fields pre, size, etc
    if (($querytype eq 'NODES_ONLY')
        ||($querytype eq 'ATOMIC_AND_NODES')) {
        $result_map->{"pre"}   = $schema->{"pre"}; 
        $result_map->{"size"}  = $schema->{"size"};
        $result_map->{"kind"}  = $schema->{"kind"};
        $result_map->{"value"} = $schema->{"value"};
        $result_map->{"name"}  = $schema->{"name"};
        $result_map->{"uri"}   = $schema->{"uri"};
    }

    return $result_map;
}

# bind the columns
sub bind_result_columns {
     my $sth = shift;
     my $RESULT_MAP = shift;

     my $bound_fields = {};
                 
     while (my ($node_field, $db_col) = each %$RESULT_MAP) {
          $db_col = lc $db_col;
          if (defined $sth->{NAME_lc_hash}{$db_col}) {
              $sth->bind_col(
                  $sth->{NAME_lc_hash}{$db_col} + 1,
                  \$bound_fields->{$node_field}
              );
          }
          else {
              die "DB column $db_col not returned from your SQL query!\n";
          }
     }
     return $bound_fields;
}

# get a new namespace prefix if the uri doesn't exists yet
# or a brand new namespace prefix
# ns_map is a hash
#   uri => prefix
sub namespace_prefix {
    my $ns_map = shift;
    my $uri    = shift;

    my $prefix;

    $prefix = $ns_map->{$uri};

    # check if prefix is defined or
    # if it takes a new prefix
    if (defined($prefix)) {
        return $prefix;
    }
    else {
        my @values = values (%$ns_map); 
    
        if (@values > 0) {
            my $idxMax = 0;
            # find the maximum 
            $values[$idxMax] > $values[$_] or $idxMax = $_ for 1 .. $#values;
            $ns_map->{$uri} = ++$values[$idxMax];
        }
        else {
            $ns_map->{$uri} = 0;
        }
    } 
}

# transform a namespace id to a string
sub namespace_str {
    my $i = shift;
    return "pf".$i."";
}

# wrapper for namespace_str and namespace_prefix
sub ns_prefix {
    my $nsmap = shift;
    my $uri   = shift;
    return namespace_str(
               namespace_prefix(
                          $nsmap,
                          $uri));
}

# set the current schema
sub set_schema {
    my $dbh = shift;
    my $db_schema = shift; 
    
    my $set_schema = "SET CURRENT SCHEMA $db_schema"; 
    $dbh->do ($set_schema)
       or die "set schema not successful";
}

############################### Utils #####################################

# implements semantics of boolean columns 
sub boolean {
    my $boolval = shift;

    ($boolval == 0 or $boolval == 1) 
       or die "wrong boolean value";

    return (($boolval == 0)?"false":"true");
}

# read query from stdin or a file
sub read_query {
    my $schema = shift;
    my $final_query = shift;

    my $state = 'START';

    while (<>) {
        chomp;
        # fetch the line
        my $line = $_;

        # start looking for schema information as soon as we
        # read this magic string
        if ($state eq 'START'
            && $line eq '-- !! START SCHEMA INFORMATION ** DO NOT EDIT THESE LINES !!')
        {
            $state = 'IN_SCHEMA_INFORMATION';
        }
        
        #
        # If we are in the IN_SCHEMA_INFORMATION state, look out for
        # relation/attribute names specified in the query
        #
        # We collect all instructions
        #
        #   <name>: <val>
        #
        # in the Perl hash $schema (with key <name> and value <val>).
        #
        elsif ($state eq 'IN_SCHEMA_INFORMATION') {
            # these are the strings we look for
            if ($line =~ /\b(Type|
                             Column\s+\((pre|size|kind|
                                 value|name|uri|item_(int|dec|dbl|str|bool))\)):\s+
                             \b(\w+)\b/x) {


                if ($1 eq 'Type') {
                    $schema->{$1} = $4;
                }
                else {
                    $schema->{$2} = $4;
                }
            }
            elsif ($line eq '-- !! END SCHEMA INFORMATION ** DO NOT EDIT THESE LINES !!') {
                $state = 'DONE_SCHEMA_INFORMATION';
            }
        }
        # simply collect all the other lines into the SQL query
        else {
            $$final_query .= "$line\n";
        }
    }

    die "No schema information found in query.\n"
        unless $state eq 'DONE_SCHEMA_INFORMATION';

    # if Type is NODES_ONLY then only Type has to be defined
    # if Type is ATOM_ONLY or ATOMIC_AND_NODES one of the
    # result items has to be defined
    # FIXME check pre, size ... 
    die "Schema information on result relation incomplete.\n"
        unless (defined $schema->{'Type'}
                && ($schema->{'Type'} eq 'ATOMIC_ONLY'
                    || ((($schema->{'Type'} eq 'NODES_ONLY') ||
                    ($schema->{'Type'} eq 'ATOMIC_AND_NODES'))
                    && (defined $schema->{'pre'}  
                        && defined $schema->{'size'}
                        && defined $schema->{'kind'}
                        && defined $schema->{'value'}
                        && defined $schema->{'name'}
                        && defined $schema->{'uri'}))
                 && ($schema->{'Type'} eq 'NODES_ONLY'
                    || (($schema->{'Type'} eq 'ATOMIC_ONLY'
                    || $schema->{'Type'} eq 'ATOMIC_AND_NODES')
                    && (defined $schema->{'item_int'}
                        || defined $schema->{'item_dec'}
                        || defined $schema->{'item_dbl'} 
                        || defined $schema->{'item_str'}
                        || defined $schema->{'item_bool'})))));
}

# check if a namespace uri is in the given array
# if not even insert the uri 
sub ns_used {
    my $usedns = shift;
    my $uri    = shift;

    foreach my $element(@$usedns) {
       if ($element eq $uri) {
           return 1;
       }
    } 

    my @temp = @$usedns;

    $usedns->[$#$usedns + 1] = $uri;
    return 0;
}

# print the xml-header
sub print_xml_header {
    # print XML header for result
    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    # print a root node to make a valid XQuery document
    print "\n<XQuery>";
}

# print the xml-footer
sub print_xml_footer {
    print "\n</XQuery>";
}

sub print_xml {
    my $stmt = shift;
    my $row  = shift;
    my $query_type = shift;

    my @stack = ();       # stack to print all the closing tags
    my $last = 'ELEM';    # assume the last thing we printed was an element
    my $nsmap = {};
    my $used_ns = [];     # contains the used namespaces

    while ($stmt->fetch ()) {
        # is it a node that we currently print?
        if ((($query_type eq 'NODES_ONLY')
            || ($query_type eq 'ATOMIC_AND_NODES')) && (defined $row->{'pre'})) {

            # print trailing > to close an element
            if (($last eq 'ATTR') && ($row->{'kind'} != 2)) {
                print '>';
                $last = 'ELEM';
            }
                

            if ($row->{'kind'} == 1) {           # kind == elem
                # newline between adjacent result elements
                if ($#stack < 0 && $last eq 'ELEM') {
                    print "\n";
                }

                # remove trailing spaces as they are returned by DB2
                $row->{'name'} =~ s/ +$//;

                print '<'.(($row->{uri} ne '')?
                          ns_prefix($nsmap,$row->{uri}).":":
                          "")
                         .$row->{'name'} . '';    # print .tag
                # print namespace 

                # check if namespace is used in this line
                if (($row->{uri} ne '') &&
                    !ns_used ($used_ns, $row->{uri})) {
                    print " xmlns:".ns_prefix($nsmap,$row->{uri}).
                          "=\"".$row->{uri}."\""; # print namespace
                }

                push @stack, [ $row->{'pre'},   # pre
                               $row->{'size'},  # size
                               $row->{'name'},  # tag
                               $row->{'uri'}];  # uri
                $last = 'ATTR';
            }
            elsif ($row->{'kind'} == 2) {         #kind == attr
                if (($row->{uri} ne '') &&
                    !ns_used ($used_ns, $row->{uri})) {
                    print " xmlns:".ns_prefix($nsmap,$row->{uri}).
                          "=\"".$row->{uri}."\""; # print namespace
                }
                print " ".(($row->{uri} ne '')?
                          ns_prefix($nsmap,$row->{uri}).":":
                          "")
                         .$row->{'name'}."=\"".$row->{'value'}."\"";

                $last = 'ATTR';
            }
            elsif ($row->{'kind'} == 3) {        # kind == text
                # remove trailing spaces as they are returned by DB2
                $row->{'value'} =~ s/ +$//;
                print $row->{'value'};            # print doc.prop
                $last = 'NODE';
            }
            elsif ($row->{'kind'} == 4) {           # kind == comm
                # remove trailing spaces as they are returned by DB2
                $row->{'value'} =~ s/ +$//;
                print '<!--'.$row->{'value'} . '-->';  # print doc.prop
                $last = 'NODE';
            }
            elsif ($row->{'kind'} == 5) {           # kind == pi
                # remove trailing spaces as they are returned by DB2
                $row->{'value'} =~ s/ +$//;
                print '<?'.$row->{'value'}.'?>';  # print doc.prop
                $last = 'NODE';
            }

            # print closing tags if necessary
            while ($#stack >= 0
                    && ($stack[-1][0] + $stack[-1][1]) <= $row->{'pre'}) {
               if ($last eq 'ATTR') {
                    print '>';
                    $last = 'ELEM';
                    next;
               }
               print "</".(($stack[-1][3] ne '')?ns_prefix($nsmap, $stack[-1][3]).":"
                          :"").$stack[-1][2].">";

               pop @stack;
               $used_ns = [];
               $last = 'ELEM'
            }
        }
        else {

            # space between adjacent atoms
            if ($last eq 'ATOM') {
                print ' ';
            }

            foreach my $t ('int', 'str', 'dbl', 'dec', 'bool') {

                if (defined $row->{"item_$t"} && defined $row->{"item_$t"}) {
                    # boolean values are stored as integers in
                    # the SQL backend, and we have to deal differently with them
                    print (($t eq "bool")?boolean($row->{"item_bool"}):$row->{"item_$t"});
                    $last = 'ATOM';
                }
            }
        }
    }
}


sub print_help {
    print "Pathfinder XML Serializer\n";
    print "(c) Database Group, Technische Universitaet Muenchen\n\n";

    print "pfserialize.pl -- A simple tool to transform an encoded XML-Document\n";
    print "                  from DB2 to an XML-Document.\n\n";

    print "Usage: pfserialize.pl [options]\n\n";
    print "Options:\n\n";
    print "    --help      Display help\n";
    print "    --schema    To set the schema name in which the docuemnt is stored\n";

    exit 0;
}

#get get options for schema and help 
sub get_options { 
    my $dbschema = shift;
    my $help     = shift;

    # get the options
    GetOptions (
        'help|?' =>   $help,
        'schema=s' => $dbschema
    ) or print_help;
}

#################################################################
#                            Main                               # 
#################################################################

# Database handle
my $dbh; 
# Statement handle
my $stmt;
# query with atoms only, or are there nodes also?
my $query_type;

my $result_map;

# change `XML' to your database name, or the entire string to
# match something other than DB2
my $DSN = 'DBI:DB2:XML';

my $final_query;

# schema information we got from PF
my $schema = {};

# schema we want to use execute our query
my $db_schema;
my $help;

#get options
get_options (\$db_schema, \$help);

# determine if the user wants help and print it, if it is so
if ($help) { print_help; }

# read user input
read_query ($schema, \$final_query);

die "Query Type not defined"
    unless (defined $schema->{'Type'});

$query_type = $schema->{'Type'};

#print $final_query;


# connect to the datase 
db_connect (\$dbh, $DSN);

if ($db_schema) { set_schema ($dbh, $db_schema); }
stmt_prepare ($dbh, \$stmt, $final_query);
stmt_execute ($stmt);

$result_map = build_result_map ($schema);
my $row = bind_result_columns ($stmt, $result_map);

print_xml_header ();
print_xml ($stmt, $row, $query_type);
print_xml_footer ();

print "\n";
# close database connection
$dbh->disconnect;

