select 	cast(null as varchar) as table_cat, 
	cast(s.name as varchar) as table_schem, 
	cast(t.name as varchar) as table_name, 
	case when k.name is null then cast(1 as smallint) else cast(0 as smallint) end as non_unique,
	cast(null as varchar) as index_qualifier, 
	cast(i.name as varchar) as index_name, 
	case i.type when 0 then cast(2 as smallint) else cast(3 as smallint) end as type, 
	cast(kc.nr as smallint) as ordinal_position,
	cast(c.name as varchar) as column_name, 
	cast(null as char(1)) as asc_or_desc, 
	cast(null as integer) as cardinality, 
	cast(null as integer) as pages, 
	cast(null as varchar) as filter_condition 

from 	sys.idxs i, 
	sys.schemas s, 
	sys.tables t, 
	sys.columns c, 
	sys.keycolumns kc, 
	sys.keys k 

where 	i.table_id = t.id
and 	t.schema_id = s.id 
and 	i.id = kc.id 
and 	t.id = c.table_id 
and 	kc."column" = c.name 
and 	t.name = 'c1006309' 
and 	(k.type is null or k.type = 1) 
and 	s.name = 'sys'

order by non_unique, type, index_qualifier, index_name, ordinal_position;
