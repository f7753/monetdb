CREATE TABLE orders (
o_orderkey BIGINT NOT NULL,
o_custkey INTEGER NOT NULL,
o_orderstatus CHAR(1) NOT NULL,
o_totalprice DECIMAL(12,2) NOT NULL,
o_orderdate DATE NOT NULL,
o_orderpriority CHAR(15) NOT NULL,
o_clerk CHAR(15) NOT NULL,
o_shippriority INTEGER NOT NULL,
o_comment VARCHAR(79) NOT NULL
);

COPY 28 RECORDS into "orders" from STDIN;
1|36901|O|173665.47|1996-01-02|5-LOW|Clerk#000000951|0|nstructions sleep furiously among |
2|78002|O|46929.18|1996-12-01|1-URGENT|Clerk#000000880|0| foxes. pending accounts at the pending, silent asymptot|
3|123314|F|193846.25|1993-10-14|5-LOW|Clerk#000000955|0|sly final accounts boost. carefully regular ideas cajole carefully. depos|
4|136777|O|32151.78|1995-10-11|5-LOW|Clerk#000000124|0|sits. slyly regular warthogs cajole. regular, regular theodolites acro|
5|44485|F|144659.20|1994-07-30|5-LOW|Clerk#000000925|0|quickly. bold deposits sleep slyly. packages use slyly|
6|55624|F|58749.59|1992-02-21|4-NOT SPECIFIED|Clerk#000000058|0|ggle. special, final requests are against the furiously specia|
7|39136|O|252004.18|1996-01-10|2-HIGH|Clerk#000000470|0|ly special requests |
32|130057|O|208660.75|1995-07-16|2-HIGH|Clerk#000000616|0|ise blithely bold, regular requests. quickly unusual dep|
33|66958|F|163243.98|1993-10-27|3-MEDIUM|Clerk#000000409|0|uriously. furiously final request|
34|61001|O|58949.67|1998-07-21|3-MEDIUM|Clerk#000000223|0|ly final packages. fluffily final deposits wake blithely ideas. spe|
35|127588|O|253724.56|1995-10-23|4-NOT SPECIFIED|Clerk#000000259|0|zzle. carefully enticing deposits nag furio|
36|115252|O|68289.96|1995-11-03|1-URGENT|Clerk#000000358|0| quick packages are blithely. slyly silent accounts wake qu|
37|86116|F|206680.66|1992-06-03|3-MEDIUM|Clerk#000000456|0|kly regular pinto beans. carefully unusual waters cajole never|
38|124828|O|82500.05|1996-08-21|4-NOT SPECIFIED|Clerk#000000604|0|haggle blithely. furiously express ideas haggle blithely furiously regular re|
39|81763|O|341734.47|1996-09-20|3-MEDIUM|Clerk#000000659|0|ole express, ironic requests: ir|
64|32113|F|39414.99|1994-07-16|3-MEDIUM|Clerk#000000661|0|wake fluffily. sometimes ironic pinto beans about the dolphin|
65|16252|P|110643.60|1995-03-18|1-URGENT|Clerk#000000632|0|ular requests are blithely pending orbits-- even requests against the deposit|
66|129200|F|103740.67|1994-01-20|5-LOW|Clerk#000000743|0|y pending requests integrate|
67|56614|O|169405.01|1996-12-19|4-NOT SPECIFIED|Clerk#000000547|0|symptotes haggle slyly around the furiously iron|
68|28547|O|330793.52|1998-04-18|3-MEDIUM|Clerk#000000440|0| pinto beans sleep carefully. blithely ironic deposits haggle furiously acro|
69|84487|F|197689.49|1994-06-04|4-NOT SPECIFIED|Clerk#000000330|0| depths atop the slyly thin deposits detect among the furiously silent accou|
70|64340|F|113534.42|1993-12-18|5-LOW|Clerk#000000322|0| carefully ironic request|
71|3373|O|276992.74|1998-01-24|4-NOT SPECIFIED|Clerk#000000271|0| express deposits along the blithely regul|
96|107779|F|68989.90|1994-04-17|2-HIGH|Clerk#000000395|0|oost furiously. pinto|
97|21061|F|110512.84|1993-01-29|3-MEDIUM|Clerk#000000547|0|hang blithely along the regular accounts. furiously even ideas after the|
98|104480|F|69168.33|1994-09-25|1-URGENT|Clerk#000000448|0|c asymptotes. quickly regular packages should have to nag re|
99|88910|F|112126.95|1994-03-13|4-NOT SPECIFIED|Clerk#000000973|0|e carefully ironic packages. pending|
100|147004|O|187782.63|1998-02-28|4-NOT SPECIFIED|Clerk#000000577|0|heodolites detect slyly alongside of the ent|

select rank() over(partition by o_custkey) from orders;
