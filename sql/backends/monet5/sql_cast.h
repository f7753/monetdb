/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2014 MonetDB B.V.
 * All Rights Reserved.
 */

/*
 * This file was generated by using the script sql_cast.sh.
 */

sql5_export str bte_2_bte(bte *res, bte *v);
sql5_export str batbte_2_bte(int *res, int *v);

sql5_export str bte_dec2_bte(bte *res, int *s1, bte *v);
sql5_export str bte_dec2dec_bte(bte *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_bte(bte *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_bte(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_bte(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_bte(bte *res, sht *v);
sql5_export str batsht_2_bte(int *res, int *v);

sql5_export str sht_dec2_bte(bte *res, int *s1, sht *v);
sql5_export str sht_dec2dec_bte(bte *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_bte(bte *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_bte(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_bte(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_bte(bte *res, int *v);
sql5_export str batint_2_bte(int *res, int *v);

sql5_export str int_dec2_bte(bte *res, int *s1, int *v);
sql5_export str int_dec2dec_bte(bte *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_bte(bte *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_bte(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_bte(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_bte(bte *res, wrd *v);
sql5_export str batwrd_2_bte(int *res, int *v);

sql5_export str wrd_dec2_bte(bte *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_bte(bte *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_bte(bte *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_bte(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_bte(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_bte(bte *res, lng *v);
sql5_export str batlng_2_bte(int *res, int *v);

sql5_export str lng_dec2_bte(bte *res, int *s1, lng *v);
sql5_export str lng_dec2dec_bte(bte *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_bte(bte *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_bte(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_bte(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_sht(sht *res, sht *v);
sql5_export str batsht_2_sht(int *res, int *v);

sql5_export str sht_dec2_sht(sht *res, int *s1, sht *v);
sql5_export str sht_dec2dec_sht(sht *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_sht(sht *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_sht(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_sht(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_sht(sht *res, int *v);
sql5_export str batint_2_sht(int *res, int *v);

sql5_export str int_dec2_sht(sht *res, int *s1, int *v);
sql5_export str int_dec2dec_sht(sht *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_sht(sht *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_sht(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_sht(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_sht(sht *res, wrd *v);
sql5_export str batwrd_2_sht(int *res, int *v);

sql5_export str wrd_dec2_sht(sht *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_sht(sht *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_sht(sht *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_sht(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_sht(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_sht(sht *res, lng *v);
sql5_export str batlng_2_sht(int *res, int *v);

sql5_export str lng_dec2_sht(sht *res, int *s1, lng *v);
sql5_export str lng_dec2dec_sht(sht *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_sht(sht *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_sht(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_sht(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_int(int *res, int *v);
sql5_export str batint_2_int(int *res, int *v);

sql5_export str int_dec2_int(int *res, int *s1, int *v);
sql5_export str int_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_int(int *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_int(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_int(int *res, wrd *v);
sql5_export str batwrd_2_int(int *res, int *v);

sql5_export str wrd_dec2_int(int *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_int(int *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_int(int *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_int(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_int(int *res, lng *v);
sql5_export str batlng_2_int(int *res, int *v);

sql5_export str lng_dec2_int(int *res, int *s1, lng *v);
sql5_export str lng_dec2dec_int(int *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_int(int *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_int(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_wrd(wrd *res, wrd *v);
sql5_export str batwrd_2_wrd(int *res, int *v);

sql5_export str wrd_dec2_wrd(wrd *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_wrd(wrd *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_wrd(wrd *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_wrd(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_wrd(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_wrd(wrd *res, lng *v);
sql5_export str batlng_2_wrd(int *res, int *v);

sql5_export str lng_dec2_wrd(wrd *res, int *s1, lng *v);
sql5_export str lng_dec2dec_wrd(wrd *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_wrd(wrd *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_wrd(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_wrd(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_lng(lng *res, wrd *v);
sql5_export str batwrd_2_lng(int *res, int *v);

sql5_export str wrd_dec2_lng(lng *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_lng(lng *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_lng(lng *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_lng(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_lng(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_lng(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_lng(lng *res, lng *v);
sql5_export str batlng_2_lng(int *res, int *v);

sql5_export str lng_dec2_lng(lng *res, int *s1, lng *v);
sql5_export str lng_dec2dec_lng(lng *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_lng(lng *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_lng(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_lng(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_lng(int *res, int *v, int *d2, int *s2);

sql5_export str flt_2_bte(bte *res, flt *v);
sql5_export str batflt_2_bte(int *res, int *v);
sql5_export str flt_num2dec_bte(bte *res, flt *v, int *d2, int *s2);
sql5_export str batflt_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str dbl_2_bte(bte *res, dbl *v);
sql5_export str batdbl_2_bte(int *res, int *v);
sql5_export str dbl_num2dec_bte(bte *res, dbl *v, int *d2, int *s2);
sql5_export str batdbl_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str flt_2_sht(sht *res, flt *v);
sql5_export str batflt_2_sht(int *res, int *v);
sql5_export str flt_num2dec_sht(sht *res, flt *v, int *d2, int *s2);
sql5_export str batflt_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str dbl_2_sht(sht *res, dbl *v);
sql5_export str batdbl_2_sht(int *res, int *v);
sql5_export str dbl_num2dec_sht(sht *res, dbl *v, int *d2, int *s2);
sql5_export str batdbl_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str flt_2_int(int *res, flt *v);
sql5_export str batflt_2_int(int *res, int *v);
sql5_export str flt_num2dec_int(int *res, flt *v, int *d2, int *s2);
sql5_export str batflt_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str dbl_2_int(int *res, dbl *v);
sql5_export str batdbl_2_int(int *res, int *v);
sql5_export str dbl_num2dec_int(int *res, dbl *v, int *d2, int *s2);
sql5_export str batdbl_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str flt_2_wrd(wrd *res, flt *v);
sql5_export str batflt_2_wrd(int *res, int *v);
sql5_export str flt_num2dec_wrd(wrd *res, flt *v, int *d2, int *s2);
sql5_export str batflt_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str dbl_2_wrd(wrd *res, dbl *v);
sql5_export str batdbl_2_wrd(int *res, int *v);
sql5_export str dbl_num2dec_wrd(wrd *res, dbl *v, int *d2, int *s2);
sql5_export str batdbl_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str flt_2_lng(lng *res, flt *v);
sql5_export str batflt_2_lng(int *res, int *v);
sql5_export str flt_num2dec_lng(lng *res, flt *v, int *d2, int *s2);
sql5_export str batflt_num2dec_lng(int *res, int *v, int *d2, int *s2);

sql5_export str dbl_2_lng(lng *res, dbl *v);
sql5_export str batdbl_2_lng(int *res, int *v);
sql5_export str dbl_num2dec_lng(lng *res, dbl *v, int *d2, int *s2);
sql5_export str batdbl_num2dec_lng(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_flt(flt *res, bte *v);
sql5_export str batbte_2_flt(int *res, int *v);
sql5_export str bte_num2dec_flt(flt *res, bte *v, int *d2, int *s2);
sql5_export str batbte_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_dbl(dbl *res, bte *v);
sql5_export str batbte_2_dbl(int *res, int *v);
sql5_export str bte_num2dec_dbl(dbl *res, bte *v, int *d2, int *s2);
sql5_export str batbte_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_flt(flt *res, sht *v);
sql5_export str batsht_2_flt(int *res, int *v);
sql5_export str sht_num2dec_flt(flt *res, sht *v, int *d2, int *s2);
sql5_export str batsht_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_dbl(dbl *res, sht *v);
sql5_export str batsht_2_dbl(int *res, int *v);
sql5_export str sht_num2dec_dbl(dbl *res, sht *v, int *d2, int *s2);
sql5_export str batsht_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_flt(flt *res, int *v);
sql5_export str batint_2_flt(int *res, int *v);
sql5_export str int_num2dec_flt(flt *res, int *v, int *d2, int *s2);
sql5_export str batint_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_dbl(dbl *res, int *v);
sql5_export str batint_2_dbl(int *res, int *v);
sql5_export str int_num2dec_dbl(dbl *res, int *v, int *d2, int *s2);
sql5_export str batint_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_flt(flt *res, wrd *v);
sql5_export str batwrd_2_flt(int *res, int *v);
sql5_export str wrd_num2dec_flt(flt *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_2_dbl(dbl *res, wrd *v);
sql5_export str batwrd_2_dbl(int *res, int *v);
sql5_export str wrd_num2dec_dbl(dbl *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_flt(flt *res, lng *v);
sql5_export str batlng_2_flt(int *res, int *v);
sql5_export str lng_num2dec_flt(flt *res, lng *v, int *d2, int *s2);
sql5_export str batlng_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str lng_2_dbl(dbl *res, lng *v);
sql5_export str batlng_2_dbl(int *res, int *v);
sql5_export str lng_num2dec_dbl(dbl *res, lng *v, int *d2, int *s2);
sql5_export str batlng_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str bte_dec2_flt(flt *res, int *s1, bte *v);
sql5_export str bte_dec2dec_flt(flt *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_flt(flt *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_flt(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_flt(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str sht_dec2_flt(flt *res, int *s1, sht *v);
sql5_export str sht_dec2dec_flt(flt *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_flt(flt *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_flt(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_flt(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str int_dec2_flt(flt *res, int *s1, int *v);
sql5_export str int_dec2dec_flt(flt *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_flt(flt *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_flt(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_flt(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_dec2_flt(flt *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_flt(flt *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_flt(flt *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_flt(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_flt(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str lng_dec2_flt(flt *res, int *s1, lng *v);
sql5_export str lng_dec2dec_flt(flt *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_flt(flt *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_flt(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_flt(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_flt(int *res, int *v, int *d2, int *s2);

sql5_export str bte_dec2_dbl(dbl *res, int *s1, bte *v);
sql5_export str bte_dec2dec_dbl(dbl *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_dbl(dbl *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_dbl(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_dbl(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str sht_dec2_dbl(dbl *res, int *s1, sht *v);
sql5_export str sht_dec2dec_dbl(dbl *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_dbl(dbl *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_dbl(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_dbl(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str int_dec2_dbl(dbl *res, int *s1, int *v);
sql5_export str int_dec2dec_dbl(dbl *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_dbl(dbl *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_dbl(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_dbl(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str wrd_dec2_dbl(dbl *res, int *s1, wrd *v);
sql5_export str wrd_dec2dec_dbl(dbl *res, int *S1, wrd *v, int *d2, int *S2);
sql5_export str wrd_num2dec_dbl(dbl *res, wrd *v, int *d2, int *s2);
sql5_export str batwrd_dec2_dbl(int *res, int *s1, int *v);
sql5_export str batwrd_dec2dec_dbl(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batwrd_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str lng_dec2_dbl(dbl *res, int *s1, lng *v);
sql5_export str lng_dec2dec_dbl(dbl *res, int *S1, lng *v, int *d2, int *S2);
sql5_export str lng_num2dec_dbl(dbl *res, lng *v, int *d2, int *s2);
sql5_export str batlng_dec2_dbl(int *res, int *s1, int *v);
sql5_export str batlng_dec2dec_dbl(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batlng_num2dec_dbl(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_bte(bte *res, bte *v);
sql5_export str batbte_2_bte(int *res, int *v);
sql5_export str bte_dec2_bte(bte *res, int *s1, bte *v);
sql5_export str bte_dec2dec_bte(bte *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_bte(bte *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_bte(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_bte(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_bte(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_sht(sht *res, bte *v);
sql5_export str batbte_2_sht(int *res, int *v);
sql5_export str bte_dec2_sht(sht *res, int *s1, bte *v);
sql5_export str bte_dec2dec_sht(sht *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_sht(sht *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_sht(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_sht(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_int(int *res, bte *v);
sql5_export str batbte_2_int(int *res, int *v);
sql5_export str bte_dec2_int(int *res, int *s1, bte *v);
sql5_export str bte_dec2dec_int(int *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_int(int *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_int(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_wrd(wrd *res, bte *v);
sql5_export str batbte_2_wrd(int *res, int *v);
sql5_export str bte_dec2_wrd(wrd *res, int *s1, bte *v);
sql5_export str bte_dec2dec_wrd(wrd *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_wrd(wrd *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_wrd(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_wrd(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str bte_2_lng(lng *res, bte *v);
sql5_export str batbte_2_lng(int *res, int *v);
sql5_export str bte_dec2_lng(lng *res, int *s1, bte *v);
sql5_export str bte_dec2dec_lng(lng *res, int *S1, bte *v, int *d2, int *S2);
sql5_export str bte_num2dec_lng(lng *res, bte *v, int *d2, int *s2);
sql5_export str batbte_dec2_lng(int *res, int *s1, int *v);
sql5_export str batbte_dec2dec_lng(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batbte_num2dec_lng(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_sht(sht *res, sht *v);
sql5_export str batsht_2_sht(int *res, int *v);
sql5_export str sht_dec2_sht(sht *res, int *s1, sht *v);
sql5_export str sht_dec2dec_sht(sht *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_sht(sht *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_sht(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_sht(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_sht(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_int(int *res, sht *v);
sql5_export str batsht_2_int(int *res, int *v);
sql5_export str sht_dec2_int(int *res, int *s1, sht *v);
sql5_export str sht_dec2dec_int(int *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_int(int *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_int(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_wrd(wrd *res, sht *v);
sql5_export str batsht_2_wrd(int *res, int *v);
sql5_export str sht_dec2_wrd(wrd *res, int *s1, sht *v);
sql5_export str sht_dec2dec_wrd(wrd *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_wrd(wrd *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_wrd(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_wrd(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str sht_2_lng(lng *res, sht *v);
sql5_export str batsht_2_lng(int *res, int *v);
sql5_export str sht_dec2_lng(lng *res, int *s1, sht *v);
sql5_export str sht_dec2dec_lng(lng *res, int *S1, sht *v, int *d2, int *S2);
sql5_export str sht_num2dec_lng(lng *res, sht *v, int *d2, int *s2);
sql5_export str batsht_dec2_lng(int *res, int *s1, int *v);
sql5_export str batsht_dec2dec_lng(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batsht_num2dec_lng(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_int(int *res, int *v);
sql5_export str batint_2_int(int *res, int *v);
sql5_export str int_dec2_int(int *res, int *s1, int *v);
sql5_export str int_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_int(int *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_int(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_int(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_int(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_wrd(wrd *res, int *v);
sql5_export str batint_2_wrd(int *res, int *v);
sql5_export str int_dec2_wrd(wrd *res, int *s1, int *v);
sql5_export str int_dec2dec_wrd(wrd *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_wrd(wrd *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_wrd(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_wrd(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_wrd(int *res, int *v, int *d2, int *s2);

sql5_export str int_2_lng(lng *res, int *v);
sql5_export str batint_2_lng(int *res, int *v);
sql5_export str int_dec2_lng(lng *res, int *s1, int *v);
sql5_export str int_dec2dec_lng(lng *res, int *S1, int *v, int *d2, int *S2);
sql5_export str int_num2dec_lng(lng *res, int *v, int *d2, int *s2);
sql5_export str batint_dec2_lng(int *res, int *s1, int *v);
sql5_export str batint_dec2dec_lng(int *res, int *S1, int *v, int *d2, int *S2);
sql5_export str batint_num2dec_lng(int *res, int *v, int *d2, int *s2);

