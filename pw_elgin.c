/*
** pw_elgin -- a program to look up elgin watch data
**
** Copyright (C) 1999, 2000 Wayne Schlitt (wayne@midwestcs.com)
** Version 1  05/24/00
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

static char *versionInfo = "1.0";


#define _GNU_SOURCE


#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include <stdarg.h>


#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif


/* default directory of the data files */
#ifndef LIBDIR
#define LIBDIR	"./" /* change this to .../lib someday */
#endif


typedef unsigned char uchar;

#define TRUE 1
#define FALSE 0

#define array_elem( a )	(sizeof( a )/sizeof( a[0] ))
#define str_prefix( a, b )	(strncmp( a, b, strlen( b ) ) == 0)

double sqr( double a )
{
    return a * a;
}




#define MAX_SN_RANGES	20000

#define MAX_GRADE	1000
#define GRADE_UNKNOWN	-1

#define MAX_CLASS	136
#define CLASS_UNKNOWN	0

#define MAX_SIZE	85
#define MIN_SIZE	-25
#define SIZE_UNKNOWN	(MAX_SIZE+1)
#define NUM_SIZES	(SIZE_UNKNOWN - MIN_SIZE + 1)

#define PLATE_FULL	0
#define PLATE_3_4	1
#define PLATE_UNKNOWN	2

#define	STYLE_HC	1	/* hunter case			*/
#define	STYLE_OF	2	/* open face			*/
#define STYLE_CVT	3	/* convertable			*/
#define STYLE_SS	4	/* sweep second			*/
#define STYLE_NONE	5	/* no second			*/
#define STYLE_UNKNOWN	6
#define NUM_STYLES	7

#define FINISH_GILDED	0	/* gold gilded movement		*/
#define FINISH_NICKEL	1	/* nickel damaskeening mvt	*/
#define FINISH_TWO_TONE 2	/* nickel damaskeening *and* gilded
				 * in a pattern.  Common on other brands
				 * not in Elgin's */
#define FINISH_FLAT	3	/* flat nickel, no damaskeening */
#define FINISH_UNKNOWN	4
#define NUM_FINISHES	5

#define MIN_MODEL	1
#define MAX_MODEL	24
#define MODEL_UNKNOWN	0
#define NUM_MODELS	25

#define SETTING_KEY	0	/* key set, key wind		*/
#define SETTING_LEVER	1	/* lever set, pend wind		*/
#define SETTING_PEND	2	/* pendant set, pend wind	*/
#define SETTING_AUTO	3	/* pendant set, automatic wind  */
#define SETTING_HACK	4	/* PS/PW w/hack setting (can stop sec hand) */
#define SETTING_UNKNOWN	5
#define NUM_SETTINGS	6

#define TRAIN_SLOW	0	/* slow train (4.5 bps)		*/
#define TRAIN_QUICK	1	/* quick train (5 bps)		*/
#define TRAIN_QS	2	/* both quick and slow for this model	*/
#define TRAIN_FAST	3	/* fast train (5.5 bps)		*/
#define TRAIN_VFAST	4	/* very fast train (6 bps)	*/
#define TRAIN_JBUG	5	/* jitter bug train (40 bps?)	*/
#define TRAIN_UNKNOWN	6
#define NUM_TRAINS	7

#define MAX_JEWELS	24
#define MIN_JEWELS	7
#define JEWELS_UNKNOWN	0
#define NUM_JEWELS	25

#define DEFAULT_WIND_HRS	40 /* is it really this long?		*/


#define REG_FREE	0	/* regulator can be freely moved	*/

#define REG_ELGIN	1	/* std Elgin micrometric regulator with */
				/* "notched spool on a threaded rod"	*/

#define REG_RR_PAT	2	/* "Railroad Patent Regulator"		*/
				/* like std Elgin, but a spring under   */
				/* the balance cock to force reg to     */
				/* one side of the notched spool.	*/
				/* looks like an elgin from the top,	*/
				/* but has a small pin near balance 	*/
				/* jewel end of the regulator		*/

#define REG_OLD_RR_PAT	3	/* "Old style Patent Regulator"		 */
				/* this was used on some of the early	*/
				/* elgin watches.  Like the RR Patent,  */
				/* it had a spring under the balance    */
				/* cock, but it used a screw on the top */
				/* of the cock to adjusted it.		*/

#define REG_BALL	4	/* The elgin "Ball Watch Co" style	*/
				/* has a spring on top and a "jack 	*/
				/* screw" type rod that you turn with a */
				/* pin on one side of the balance cock  */
				
#define REG_HULBURD	5	/* hulburds were just weird.		*/ 
#define REG_DURABALANCE 6	/* the regulator is part of the balance */
#define REG_UNKNOWN	7
#define NUM_REGS	8


#define BARREL_GOING	0	/* "normal" mainspring barrel		*/
#define BARREL_MOTOR	1	/* barrel has shaft with 3 screws	*/
#define BARREL_JMOTOR	2	/* jeweled motor barrel			*/
#define BARREL_UNKNOWN	3
#define NUM_BARRELS	4


#define MAX_PAT_DIGITS	8
#define MAX_CHAR_PER_DIGIT	(sizeof( "[0123456789]" ) - 1)

typedef struct 
{
    int		num_digits;
    char	digit_set[ MAX_PAT_DIGITS ][ 10 ];
    char	digits_in_set[ MAX_PAT_DIGITS ];
    int		digit_val[ MAX_PAT_DIGITS ];
    int		min_val[ MAX_PAT_DIGITS ];
    int		max_val[ MAX_PAT_DIGITS ];
} num_pat_type;



#define N_NONE		0	/* no name printed on mvt		*/
#define N_ADVANCE	1
#define N_AGE		2
#define N_AVERY		3	/* Thomas M. Avery	2nd Prez	*/
#define N_CHIEF		4
#define N_CULVER	5	/* Howard Z. Culver	On 1884 board	*/
#define N_DEXTER_ST	6
#define N_FATHER_TIME	7
#define N_FARGO		8	/* Charles (Chas) Fargo	On 1884 board	*/
#define N_FARWELL	9	/* J.V. Farwell		Chicago investor?    */
#define N_FERRY		10	/* William H. Ferry	Chicago investor?    */
#define N_FRANCES_RUBIE	11	/* ???					*/
#define N_GAIL_BORDEN	12	/* "of the condensed milk fame"		*/
#define N_LADY_ELGIN	13
#define N_LADY_RAYMOND	14
#define N_LAFLIN	15	/* Matthew (Mat) Laflin	An original director */
#define N_LEADER	16
#define N_LORD_ELGIN	17	/* George P. Lord	1st Biz Manager	     */
				/* see notes below about the Lord Elgin name */
#define N_OGDEN		18	/* M.D. Odgen		Chicago investor?    */
#define N_OVERLAND	19
#define N_RAYMOND	20	/* Benjamin W. Raymond	1st Prez	*/
#define N_RYERSON	21	/* Joseph T. Ryerson	An original director */
#define N_TAYLOR	22	/* Henry H. Taylor	An original director */
#define N_VERITAS	23
#define N_WHEELER	24	/* George M. Wheeler	An original director */
#define N_STANDARD	25
#define N_INTER_OCEAN	26
#define N_N_W_CO	27
#define N_SUN_DIAL	28
#define N_ATLAS		29
#define N_ACME		30
#define N_SOLAR		31
#define N_149		32
#define N_150		33
#define N_270		34
#define N_274		35
#define N_280		36
#define N_333		37
#define N_348		38
#define N_349		39
#define N_HULBURD	40	/* Charles H. Hulburd	3rd Prez	*/
#define N_DELUXE	41
#define N_STRAND	42
#define N_SENIOR_17	43
#define N_LORD_ELGIN_SERIES	44
#define N_TRANSIT	45

#define MAX_NAME	46
#define N_DEFER		MAX_NAME /* defer to the default	*/
#define N_UNKNOWN	(MAX_NAME+1)	/* don't know which name */


char *mvt_names[ MAX_NAME ] =
{ "", "ADV", "AGE", "AVR", "CHF", "HZC", "DEX",
  "FT", "FGO", "FAR", "FRY", "FR", "GB",
  "LDY", "LDR", "LAF", "LED", "LE", "OGD",
  "OVR", "BWR", "RYR", "TAY", "VER", "GMW",
  "STD", "NWC", "INT",
  "SUN", "ATL", "ACM", "SOL",
  "149", "150", "270", "274", "280", "333", "348", "349",
  "HUL", "DLX",
  "STR", "S17", "LES", "TRN"};

char *mvt_names_long[ MAX_NAME ] = 
{ "None", "Advance", "Age", "Avery", "Chief", "Culver", "Dexter St",
  "Father Time", "Fargo", "Farwell", "Ferry", "Frances Rubie", "Gail Borden",
  "Lady Elgin", "Lady Raymond", "Laflin", "Leader", "Lord Elgin", "Ogden",
  "Overland", "Raymond", "Ryerson", "Taylor", "Veritas", "Wheeler",
  "Standard", "N W Co", "Inter-Ocean",
  "Sun Dial", "Atlas", "Acme", "Solar",
  "149", "150", "270", "274", "333", "348", "280", "349",
  "Hulburd", "Deluxe",
  "Strand", "Senior 17", "LE Series", "Transit" };


/*
 * Notes on the "Lord Elgin" name:
 *
 * While Elgin did have a high muckity-muck with the last name of "Lord"
 * who worked for them, it is not clear that the Lord Elgin line was named
 * after him.  G. P. Lord retired(?) in 1876, some 30 years before the first
 * "Lord Elgin" watch was created.  It is possible that they had even
 * forgotten that they even had a "G. P. Lord" working for them.  Also, Elgin
 * seems to have switched to "made up" names like "Veritas" and "Father Time".
 *
 * On the other hand, the also resurected the "Frances Rubie" name and created
 * the "Lady Raymond" name in this same general timeframe.
 *
 * Anyway, I find this "G.P. Lord" relationship interesting and amusing. 
 */



#define MAX_ADJ_NAME 11
#define ADJ_UNKNOWN  (MAX_ADJ_NAME-1)
char *adj_names[ MAX_ADJ_NAME ] = {"U", "A", "AT", "AP", "A1P", "A2P", "A3P", "A4P", "A5P", "A6P", "?"}; 


#define MAX_GRADE_NAME	8

typedef struct
{
    int		grade;
    int		class;
    int		dial_size;
    int		mvt_size;
    int		plate;
    int		bridge;
    int		style;
    int		finish;
    int		min_model;
    int		max_model;
    int		setting;
    int		train;
    int		min_jewels;
    int		max_jewels;
    int		min_adj;
    int		max_adj;
    int		wi_option;	/* is a wind indicator available? */	
    int		wind_hrs;	/* how long watch stays wound */
    int		regulator;	/* type of regulator adjustment		*/
    int		barrel;
    int		name[ MAX_GRADE_NAME ];
    char	*comments;	/* comments about this grade		*/
    char	*comment_lines;	/* comments following this line		*/

    /* the following info is calculated based off of the sn data */
    int		first_snr;
    int		last_snr;
    int		min_cnt;
    int		max_cnt;
    int		total_cnt;
    int		num_runs;
    int		num_consecutive;
    double	avg_cnt;
    double	std_dev;

    char	sn_comments;	/* sn range w/this grade has comments */
    char	sn_comment_lines; /* sn range w/this grade has comment lines */
    
    char	first_in_size;
    char	last_produced_in_size;
    char	last_grade_in_size;
    char	first_in_bridge;
    char	first_in_bridge_in_size;
    char	first_in_style;
    char	first_in_style_in_size;
    char	first_in_finish;
    char	first_in_finish_in_size;
    char	first_in_model_in_size;
    char	first_in_setting;
    char	first_in_setting_in_size;
    char	first_in_jewel;
    char	first_in_jewel_in_size;
    char	first_in_wi_option;
    char	first_in_wi_option_in_size;
    char	first_in_wind_hrs;
    char	first_in_regulator;
    char	first_in_regulator_in_size;
    char	first_in_barrel;
    char	first_in_barrel_in_size;
    char	first_in_name[ MAX_GRADE_NAME ];
    char	first_in_name_in_size[ MAX_GRADE_NAME ];
    
    int		flag_notes;	/* print foot notes on this grade?	*/
} grade_type;


typedef struct
{
    int		sn;
    uchar	letter;
    int		cnt;
    int		grade;
    int		name;
    int		line;
    char	*comments;	/* comments about this grade		*/
    char	*comment_lines;	/* comments following this line		*/

    int		year;

    int		next_grade;
    int		next_size;
    int		next_bridge;
    int		next_bridge_in_size;
    int		next_style;
    int		next_style_in_size;
    int		next_finish;
    int		next_finish_in_size;
    int		next_model_in_size;
    int		next_setting;
    int		next_setting_in_size;
    int		next_jewel;
    int		next_jewel_in_size;
    int		next_wi_option;
    int		next_wi_option_in_size;
    int		next_wind_hrs;
    int		next_regulator;
    int		next_regulator_in_size;
    int		next_barrel;
    int		next_barrel_in_size;
    int		next_name[ MAX_GRADE_NAME ];
    int		next_name_in_size[ MAX_GRADE_NAME ];
} sn_range_type;

typedef struct
{
    int		grade[ MAX_GRADE ];
    int		size[ NUM_SIZES ];
    int		bridge;
    int		bridge_in_size[ NUM_SIZES ];
    int		style[ NUM_STYLES ];
    int		style_in_size[ NUM_SIZES ][ NUM_STYLES ];
    int		finish[ NUM_FINISHES ];
    int		finish_in_size[ NUM_SIZES ][ NUM_FINISHES ];
    int		model_in_size[ NUM_SIZES ][ NUM_MODELS ];
    int		setting[ NUM_SETTINGS ];
    int		setting_in_size[ NUM_SIZES ][ NUM_SETTINGS ];
    int		jewel[ NUM_JEWELS ];
    int		jewel_in_size[ NUM_SIZES ][ NUM_JEWELS ];
    int		wi_option;
    int		wi_option_in_size[ NUM_SIZES ];
    int		wind_hrs;
    int		regulator[ NUM_REGS ];
    int		regulator_in_size[ NUM_SIZES ][ NUM_REGS ];
    int		barrel[ NUM_BARRELS ];
    int		barrel_in_size[ NUM_SIZES ][ NUM_BARRELS ];
    int		name[ MAX_GRADE_NAME ];
    int		name_in_size[ NUM_SIZES ][ MAX_GRADE_NAME ];
} run_info_type;

typedef struct
{
    run_info_type	first;
    run_info_type	prev_tot_in_gr;
    run_info_type	prev_snr_idx;
} first_runs_type;


typedef struct 
{
    int		year;
    int		sn;
} sn_year_type;


sn_year_type sn_year[] = {
	{1867,	    100},
	{1868,	  31000},
	{1869,	  71000},
	{1870,	 101000},
	{1871,	 126000},
	{1872,	 152000},
	{1873,	 176000},
	{1874,	 210000},
	{1875,	 310000},
	{1876,	 410000},
	{1877,	 510000},
	{1878,	 552000},
	{1879,	 601000},
	{1880,	 701000},
	{1881,	 801000},
	{1882,	1000000},
	{1883,	1440000},
	{1884,	1650000},
	{1885,	1850000},
	{1886,	2000000},
	{1887,	2550000},
	{1888,	3000000},
	{1889,	3550000},
	{1890,	4000000},
	{1891,	4400000},
	{1892,	4890000},
	{1893,	5000000},
	{1894,	5500000},
	{1895,	6000000},
	{1896,	6550000},	/* Apr 6 1986 mtrl cat has sn up to 6944000 */
	{1897,	7000000},
	{1898,	7550000},
	{1899,	8100000},
	{1900,	9100000},
	{1901,	9350000},
	{1902,	9755000},
	{1903, 10100000},
	{1904, 11000000},
	{1905, 12100000},
	{1906, 12500000},
	{1907, 13100000},
	{1908, 13550000},
	{1909, 14100000},
	{1910, 15100000},
	{1911, 16000000},
	{1912, 17000000},
	{1913, 17550000},
	{1914, 18000000},
	{1915, 18500000},
	{1916, 19000000},
	{1917, 20000000},
	{1918, 21000000},
	{1919, 22000000},
	{1920, 23000000},
	{1921, 24000000},
	{1922, 25000000},
	{1923, 26000000},
	{1924, 27000000},
	{1925, 28000000},
	{1926, 29000000},
	{1927, 30000000},
	{1928, 32000000},
	{1929, 33000000},
	{1930, 33300000},
	{1931, 33500000},
	{1932, 33700000},
	{1933, 34000000},
	{1934, 35000000},
	{1935, 35500000},
	{1936, 36200000},
	{1937, 37000000},
	{1938, 37900000},
	{1939, 38200000},
	{1940, 39100000},
	{1941, 40200000},
	{1942, 41100000},
	{1943, 42200000},
	{1944, 42600000},
	{1945, 43200000},
	{1946, 44000000},
	{1947, 45000000},
	{1948, 46000000},
	{1949, 47000000},
	{1950, 48000000},
	{1951, 50000000},
	{1952, 52000000},
	{1953, 53300000},
	{1954, 54000000},
	{1955, 54500000},
	{1956, 55000000}
};



int cvt_sn_letter( uchar letter, int sn )
{
    switch( letter )
    {
    case '_':
	return 0;

    case 'X':			/* shugart doesn't say where this split is */
	if( sn < 500000 )
	    return 38000000;
	else
	    return 39000000;

#define SHUGART
#ifdef SHUGART
	/* according to Shugart's, these four letters are represent the
	 * 42 million prefix.  This contradicts what is shown in the
	 * 1950 MC, but it makes the production quantities make more sense.
	 *
	 * With all of these at 42 million, the yearly production output
	 * drops until 1941, and then shoots up in 1942 as would be expected
	 * when the US entered WWII.  The production peaks in 1944, and then
	 * drops dramatically in 1945 after the war ends.
	 *
	 * With the prefixes determined by the 1950 MC, the production starts
	 * to rise in 1939, and actually drops to a low level in 1943-1944.
	 *
	 * Another way to "fix" this problem would be to adjust the sn_year[]
	 * values, but I have no idea what to adjust them too, and the various
	 * sources seem to agree for this time period.
	 *
	 * A further thing to ponder: The gold flashed movements are
	 * supposed to have been made during WWII "because nickel was
	 * a criticl metal".  The non-lettered gold flashed watches
	 * are in the 41,000,000 range and the current sn_year[]
	 * ranges would place that in early 1942, which sounds about
	 * right.  There are also gold flashed watches in the "Y"
	 * ranges, which also appear to be in the 1942-1944 range.
	 */

    case 'C':
    case 'E':
    case 'T':
    case 'Y':
	return 42000000;
#else
    case 'C':			/* shugart says 42,000,000 but elgin SN list
				 * shows C's ending at 40,000,000 */
	return 39000000;

    case 'E':			/* shugart says 42,000,000 but elgin SN list
				 * shows E's between C's and T/Y's, so this
				 * is my guess. */
	return 40000000;

    case 'T':			/* shugart says 42,000,000 and elgin SN list
				 * shows T's ending at 43,000,000, but there
				 * are also some before the Y's */
	if( sn < 300000 )
	    return 41000000;
	else
	    return 42000000;

    case 'Y':			/* shugart says 42,000,000 but elgin SN list
				 * shows Y's ending at 42,000,000 */
	return 41000000;
#endif

    case 'L':
	return 43000000;

    case 'U':
	return 44000000;

    case 'J':
	return 45000000;

    case 'V':
	return 46000000;

    case 'H':
	return 47000000;

    case 'N':
	return 48000000;

    case 'F':
	return 49000000;

    case 'S':
	return 50000000;

    case 'R':
	return 51000000;

    case 'P':
	return 52000000;

    case 'K':
	return 53000000;

    case 'I':
	return 54000000;
	break;

    case '!':		/* special case for "grades with no known ranges" */
	return -2;

    default:
	return -1;
	break;
    }
}


int cvt_sn_grade( uchar letter, int sn, int grade )
{
    switch( grade )
    {
	/* grades before about G=85 are too disorganized to figure out
	 * which are retro-grades and which arent.  After all, the
	 * "first" watch made was a G=69, and G=1 wasn't made until ca
	 * 1879 */
	 
    case 95:
	/* first sn is 2180001, but based on surounding ones, I would expect
	 * something around 1200001-1500001 */
	return 0;

    case 109:
	/* first sn is 4212001, but based on surounding ones, I would expect
	 * something around 2850001-3010001 */
	return 0;

/* 140?   early first SN */
/* 141?   early first SN */
/* 142?   early first SN */
/* 145? <= 74? */
/* 147?  looks suspicious, but I can't find anything which it might be */
/* 148?   early first SN */
/* 167?   early first SN */
/* 168?   early first SN */

    case 169:
	/* looks like the first run of G=169 were really converted 142s */
	/* later G=169's were converted to 228's */
	/* need a check here */
	return 0;

    case 170:
	/* looks like the first run of G=170 were really converted 146s */
	/* need a check here */
	return 0;

    case 174:
	/* looks like the first two runs of G=174 were really converted 112s */
	if( letter == '_' && sn < 5396001 )
	    return 1500000;
	return 0;
	
    case 176:
	/* looks like the first run of G=176 were really converted 122s */
	if( letter == '_' && sn < 5411001 )
	    return 1400000;
	return 0;

/* 185?   early first SN */

    case 186:
	/* looks like the first two runs of G=186 were really converted 157s */
/*	if( letter == '_' && sn < 5411001 )
	return 700000; */
	return 0;

/* 187?   late first SN */
/* 220?   early first SN */

    case 224:
	/* looks like the first 3 runs of G=224 were really converted 198s */
	/* need a check here */
	return 0;
	
    case 226:
	/* looks like all runs of G=226 were really converted 141s */
	/* need a check here */
	return 0;

    case 227:
	/* looks like all runs of G=227 were really converted 145s */
	/* need a check here */
	return 0;

    case 228:
	/* looks like all runs of G=228 were really converted 169s */
	/* (G=169's were converted from 142's) */
	/* need a check here */
	return 0;

    case 229:
	/* looks like all runs of G=229 were really converted 170s */
	/* (G=170's were converted from 146's) */
	/* need a check here */
	return 0;

    case 230:
	/* looks like the first run of G=230 were really converted 132s */
	/* need a check here */
	return 0;

    case 231:
	/* looks like all runs of G=231 were really converted 133s */
	/* need a check here */
	return 0;

/* 241?  early first SN   looks like this is from 154s*/
/* 242?  late first SN */
/* 243?  late first SN */
/* 244?  early first SN   looks like this is from 159 */
/* 246?  late first SN */
/* 257?  looks like this is from 220 */
/* 258?  looks like this is from 221 */
/* 259?  looks like this is from 233, maybe? */
/* 260?  early first SN   looks like this is from 235 */
/* 261?  early first SN   looks like this is from 217 */
/* 262?  looks like this is from 218, maybe? */
/* 264?  looks like this is from 204 */

    case 265:
	/* looks like all runs of G=265 were really converted 165s */
	/* need a check here */
	return 0;

    case 266:
	/* looks like all the runs of G=266 were really converted 166s */
	/* need a check here */
	return 0;

/* 271?   early first SN */
/* 272?   early first SN */

    case 273:
	/* looks like all the runs of G=273 were really converted 184s */
	/* need a check here */
	return 0;

/* 283?  early first SN   looks like it is related to G=148 */
/* 299?  late first SN */
/* 309?  early first SN   looks like it is related to G=251 */
/* 316?  early first SN   looks like it is related to G=261 */
/* 319?  first SN looks like it is half the final run of G=268 */
/* 320?  early first SN */
/* 321?  first SN looks like it is half the final run of G=275 */
/* 339?  first SN looks like it is half the final run of G=305 */
/* 340?  first SN looks like it is half the final run of G=306 */
/* 344?  early first SN   looks like it is related to G=302 */
/* 345?  early first SN   looks like it is related to G=304 */
/* 349?  early first SN */
/* 379?  looks like it is related to G=336 */
/* 382?  early first SN   looks like it is related to G=340 */
/* 383?  early first SN   looks like it is related to G=344 */
/* 384?  looks like it is related to G=345 */
/* 412?  early first SN */
/* 428?  late first SN */
/* 429?  late first SN */
/* 430?  late first SN */
/* 431?  late first SN */
/* 437?  early first SN */
/* 496?  late first SN */
/* 505?  early first SN */
/* 522?  early first SN */
/* 537?  late first SN */
/* 538?  late first SN */
/* 539?  late first SN */
/* 581?  early first SN */

    }

    return 0;
}


int elgin_prod_year( sn_range_type *s )
{
    int		i;
    static int	last_sn = 0;
    static int	last_i = 0;

    int		sn_offset;

    int		sn = s->sn;

    
    if( (sn < 101 && s->letter == '_') || sn < 1 )
    {
	fprintf( stderr, "%s:%d invalid serial number: %d\n",
		 __FILE__, __LINE__, sn );

	return 0;
    }

    
    sn_offset = cvt_sn_letter( s->letter, sn );
    if( sn_offset < 0 )
    {
	if( sn_offset != -2 )
	    fprintf( stderr, "%s:%d invalid letter serial number: "
		     "'%c' (%02x)\n",
		     __FILE__, __LINE__, s->letter, s->letter );

	return 0;
    }
    
    sn += sn_offset;
    
    
/*    sn += cvt_sn_grade( s->letter, s->sn, s->grade ); */

    if( sn < last_sn )
    {
	last_sn = 0;
	last_i = 0;
    }

    for( i = last_i; i < array_elem( sn_year ); i++ )
	if( sn <= sn_year[i].sn )
	{
	    last_i = i;
	    last_sn = sn;
	    return sn_year[i].year - 1;
	}


    fprintf( stderr, "%s:%d invalid serial number: %d\n",
	     __FILE__, __LINE__, sn );
    
    return 9999;
}



/*
 *****************************************************************************
 * WARNING!
 *****************************************************************************
 *
 * These are nasty macros that have all sorts of really bad side effects.
 * The alternative is to cut and paste a lot of code, which I think is worse.
 *
 */


#define DEFUN_FIND_COUNT_VAR( var, subscript, check ) \
int find_count_in_##var( uchar letter, int sn, \
			grade_type *g, sn_range_type *snr, \
			first_runs_type *fr ) \
{ \
    int		tot_in_gr; \
    int		snr_idx; \
 \
    check;\
 \
    snr_idx = fr->prev_snr_idx.var subscript; \
    tot_in_gr = fr->prev_tot_in_gr.var subscript; \
    if( snr_idx == -1 \
	|| snr[ snr_idx ].letter != letter \
	|| snr[ snr_idx ].sn > sn \
	) \
    { \
	tot_in_gr = 0; \
	snr_idx = fr->first.var subscript; \
    } \
     \
    do \
    { \
	if( snr[ snr_idx ].letter != letter \
	    || sn > snr[ snr_idx ].sn + snr[ snr_idx ].cnt ) \
	    tot_in_gr += snr[ snr_idx ].cnt; \
	else \
	{ \
	    fr->prev_tot_in_gr.var subscript = tot_in_gr; \
	    fr->prev_snr_idx.var subscript = snr_idx; \
	     \
	    return tot_in_gr + sn - snr[ snr_idx ].sn + 1; \
	} \
         \
	snr_idx = snr[ snr_idx ].next_##var; \
    } \
    while( snr_idx != -1 ); \
     \
    fprintf( stderr, "%s:%d  could not find sn %d's count in " #var ".\n", \
	     __FILE__, __LINE__, sn ); \
 \
    return -1; \
}

#define DEFUN_FIND_COUNT_VAR_IN_SIZE( var, subscript, check ) \
int find_count_in_##var##_in_size( uchar letter, int sn, \
			grade_type *g, sn_range_type *snr, \
			first_runs_type *fr ) \
{ \
    int		tot_in_gr; \
    int		snr_idx; \
 \
    check; \
 \
    snr_idx = fr->prev_snr_idx.var##_in_size[ g->mvt_size - MIN_SIZE ]subscript; \
    tot_in_gr = fr->prev_tot_in_gr.var##_in_size[ g->mvt_size - MIN_SIZE ]subscript; \
    if( snr_idx == -1 \
	|| snr[ snr_idx ].letter != letter \
	|| snr[ snr_idx ].sn > sn \
	) \
    { \
	tot_in_gr = 0; \
	snr_idx = fr->first.var##_in_size[ g->mvt_size - MIN_SIZE ]subscript; \
    } \
     \
    do \
    { \
	if( snr[ snr_idx ].letter != letter \
	    || sn > snr[ snr_idx ].sn + snr[ snr_idx ].cnt ) \
	    tot_in_gr += snr[ snr_idx ].cnt; \
	else \
	{ \
	    fr->prev_tot_in_gr.var##_in_size[ g->mvt_size - MIN_SIZE ]subscript = tot_in_gr; \
	    fr->prev_snr_idx.var##_in_size[ g->mvt_size - MIN_SIZE ]subscript = snr_idx; \
	     \
	    return tot_in_gr + sn - snr[ snr_idx ].sn + 1; \
	} \
 \
	snr_idx = snr[ snr_idx ].next_##var##_in_size; \
    } \
    while( snr_idx != -1 ); \
     \
    fprintf( stderr, "%s:%d  could not find sn %d's count in " #var ".\n", \
	     __FILE__, __LINE__, sn ); \
 \
    return -1; \
}

#define SET_COUNT_VAR( var, subscript, cond ) \
	if( fr->first.var subscript == -1 ) \
	{ \
	    if( cond ) \
            { \
                g->first_in_##var = TRUE; \
	        g->flag_notes = TRUE; \
            } \
	    fr->first.var subscript = i; \
	    pr.first.var subscript = i; \
	} else { \
	    snr[ pr.first.var subscript ].next_##var = i; \
	    pr.first.var subscript = i; \
	}

#define SET_COUNT_VAR_IN_SIZE( var, subscript, cond ) \
	if( fr->first.var##_in_size[ size_idx ] subscript == -1 ) \
	{ \
	    if( cond ) \
            { \
	        g->first_in_##var##_in_size = TRUE; \
	        g->flag_notes = TRUE; \
            } \
	    fr->first.var##_in_size[ size_idx ] subscript = i; \
	    pr.first.var##_in_size[ size_idx ] subscript = i; \
	} else { \
	    snr[ pr.first.var##_in_size[ size_idx ] subscript ].next_##var##_in_size = i; \
	    pr.first.var##_in_size[ size_idx ] subscript = i; \
	}


#define CHECK_COUNT_VAR( var, subscript, msg, limit ) \
	cnt = find_count_in_##var( s->letter, out_np.min_val[ 0 ], \
				   g, snr, fr ); \
	if( cnt < limit ) \
	    add_note( &note,"%d of %s" msg, cnt, var##_to_str( subscript ) ); \


#define CHECK_COUNT_VAR_IN_SIZE( var, subscript, msg, limit ) \
	cnt = find_count_in_##var##_in_size( s->letter, out_np.min_val[ 0 ], \
				   g, snr, fr ); \
	if( cnt < limit \
	    && cnt != find_count_in_##var(  s->letter, out_np.min_val[ 0 ], \
				            g, snr, fr ) \
            && ( !g->first_in_size \
		 || cnt != find_count_in_size( s->letter, \
					       out_np.min_val[ 0 ], \
					       g, snr, fr ) \
		 || find_count_in_grade( s->letter, out_np.min_val[ 0 ], g, snr, fr ) != find_count_in_size( s->letter, out_np.min_val[ 0 ], g, snr, fr ) \
		 ) \
          ) \
	    add_note( &note,"%d of %s" msg " in %ss", cnt, var##_to_str( subscript ), \
                    size_to_str( g->mvt_size ) ); \



DEFUN_FIND_COUNT_VAR( grade, [ g->grade ], /* */ );
DEFUN_FIND_COUNT_VAR( size, [ g->mvt_size - MIN_SIZE ], /* */ );
/* create a bogus function so that check_count_bridge_in_size works */
int find_count_in_bridge( uchar letter, int sn,
			grade_type *g, sn_range_type *snr,
			first_runs_type *fr )
{
    return -1;
}
DEFUN_FIND_COUNT_VAR_IN_SIZE( bridge, /* */, if( !g->bridge ) \
    { \
	fprintf( stderr, "%s:%d  grade %d is not a bridge model.\n", \
		 __FILE__, __LINE__, g->grade ); \
	 \
	return -1; \
    } \
)
DEFUN_FIND_COUNT_VAR( style, [ g->style ], /* */ );
DEFUN_FIND_COUNT_VAR_IN_SIZE( style, [ g->style ], /* */ );
DEFUN_FIND_COUNT_VAR( finish, [ g->finish ] , /* */ );
DEFUN_FIND_COUNT_VAR_IN_SIZE( finish, [ g->finish ], /* */ );
/* create a bogus function so that check_count_model_in_size works */
int find_count_in_model( uchar letter, int sn,
			grade_type *g, sn_range_type *snr,
			first_runs_type *fr )
{
    return -1;
}
DEFUN_FIND_COUNT_VAR_IN_SIZE( model, [ g->min_model ], /* */ );
DEFUN_FIND_COUNT_VAR( setting, [ g->setting ], /* */ );
DEFUN_FIND_COUNT_VAR_IN_SIZE( setting, [ g->setting ], /* */ );
DEFUN_FIND_COUNT_VAR( jewel, [ g->max_jewels ], /* */ );
DEFUN_FIND_COUNT_VAR_IN_SIZE( jewel, [ g->max_jewels ], /* */ );
DEFUN_FIND_COUNT_VAR( wi_option, /* */, if( !g->wi_option ) \
    { \
	fprintf( stderr, "%s:%d  grade %d is not a WI model.\n", \
		 __FILE__, __LINE__, g->grade ); \
	 \
	return -1; \
    } \
)
DEFUN_FIND_COUNT_VAR_IN_SIZE( wi_option, /* */, if( !g->wi_option ) \
    { \
	fprintf( stderr, "%s:%d  grade %d is not a WI model.\n", \
		 __FILE__, __LINE__, g->grade ); \
	 \
	return -1; \
    } \
)
DEFUN_FIND_COUNT_VAR( wind_hrs, /* */, if( g->wind_hrs == DEFAULT_WIND_HRS ) \
    { \
	fprintf( stderr, "%s:%d  grade %d is has the default wind hours.\n", \
		 __FILE__, __LINE__, g->grade ); \
	 \
	return -1; \
    } \
)
DEFUN_FIND_COUNT_VAR( regulator, [ g->regulator ], /* */ );
DEFUN_FIND_COUNT_VAR_IN_SIZE( regulator, [ g->regulator ], /* */ );
DEFUN_FIND_COUNT_VAR( barrel, [ g->barrel ], /* */ );
DEFUN_FIND_COUNT_VAR_IN_SIZE( barrel, [ g->barrel ], /* */ );



char sts_strings[ NUM_SIZES ][ 32 ];

char *size_to_str( int size )
{
    int		idx;
    
    idx = size - MIN_SIZE;

    if( (size < MIN_SIZE || size > MAX_SIZE) && size != SIZE_UNKNOWN )
    {
	fprintf( stderr, "%s:%d invalid size of %d\n",
		 __FILE__, __LINE__, size );
	size = SIZE_UNKNOWN;
    }
    
    
    if( size == SIZE_UNKNOWN )
	return "?";
    else if( sts_strings[ idx ][0] != '\0' )
	return sts_strings[ idx ];
    else if( size < 0 )
    {
	sprintf( sts_strings[ idx ], "%d/0", 1 - size );
	return sts_strings[ idx ];
    }
    else
    {
	sprintf( sts_strings[ idx ], "%d", size );
	return sts_strings[ idx ];
    }
}


char *plate_to_str( int plate )
{
    switch( plate )
    {
    case PLATE_FULL:
	return "fp";

    case PLATE_3_4:
	return "3/4";

    default:
	fprintf( stderr, "%s:%d invalid plate of %d\n",
		 __FILE__, __LINE__, plate );
	/* fall through */

    case PLATE_UNKNOWN:
	return "?";
    }
}


char *bridge_to_str( int bridge )
{
    if( bridge )
	return "bridge";
    else
	return "non-bridge";
}


char *style_to_str( int style )
{
    switch( style )
    {
    case STYLE_HC:
	return "HC";

    case STYLE_OF:
	return "OF";

    case STYLE_CVT:
	return "CVT";

    case STYLE_SS:
	return "SS";

    case STYLE_NONE:
	return "NS";

    default:
	fprintf( stderr, "%s:%d invalid style of %d\n",
		 __FILE__, __LINE__, style );
	/* fall through */

    case STYLE_UNKNOWN:
	return "?";
    }
}


char *style_to_instr( int style )
{
    switch( style )
    {
    case STYLE_HC:
	return "h";

    case STYLE_OF:
	return "o";

    case STYLE_CVT:
	return "c";

    case STYLE_SS:
	return "s";

    case STYLE_NONE:
	return "-";

    default:
	fprintf( stderr, "%s:%d invalid style of %d\n",
		 __FILE__, __LINE__, style );
	/* fall through */

    case STYLE_UNKNOWN:
	return "?";
    }
}


char *finish_to_str( int finish )
{
    switch( finish )
    {
    case FINISH_GILDED:
	return "gilded";
	break;

    case FINISH_NICKEL:
	return "nickel";
	break;

    case FINISH_TWO_TONE:
	return "two tone";
	break;

    case FINISH_FLAT:
	return "flat nickel";
	break;

    default:
	fprintf( stderr, "%s:%d invalid finish of %d\n",
		 __FILE__, __LINE__, finish );
	/* fall through */

    case FINISH_UNKNOWN:
	return "?";
	break;
    }
}


char *model_to_str( int model )
{
    static int	buf_cnt = 0;
    static char	buf[ 8 ][ 32 ];
    
    if( ++buf_cnt >= array_elem( buf ) )
	buf_cnt = 0;

    if( model == 1 )
	sprintf( buf[ buf_cnt ], "1st" );
    else if( model == 2 )
	sprintf( buf[ buf_cnt ], "2nd" );
    else if( model == 3 )
	sprintf( buf[ buf_cnt ], "3rd" );
    else
	sprintf( buf[ buf_cnt ], "%dth", model );

    return buf[ buf_cnt ];
}


char *finish_to_instr( int finish )
{
    switch( finish )
    {
    case FINISH_GILDED:
	return "g";
	break;

    case FINISH_NICKEL:
	return "n";
	break;

    case FINISH_TWO_TONE:
	return "t";
	break;

    case FINISH_FLAT:
	return "f";
	break;

    default:
	fprintf( stderr, "%s:%d invalid finish of %d\n",
		 __FILE__, __LINE__, finish );
	/* fall through */

    case FINISH_UNKNOWN:
	return "?";
	break;
    }
}


char *setting_to_str( int setting )
{
    switch( setting )
    {
    case SETTING_KEY:
	return "key";

    case SETTING_LEVER:
	return "lever";

    case SETTING_PEND:
	return "pend";

    case SETTING_AUTO:
	return "auto";
	
    case SETTING_HACK:
	return "hack";
	
    default:
	fprintf( stderr, "%s:%d invalid setting of %d\n",
		 __FILE__, __LINE__, setting );
	/* fall through */

    case SETTING_UNKNOWN:
	return "?";
    }
}


char *setting_to_instr( int setting )
{
    switch( setting )
    {
    case SETTING_KEY:
	return "k";

    case SETTING_LEVER:
	return "l";

    case SETTING_PEND:
	return "p";

    case SETTING_AUTO:
	return "a";
	
    case SETTING_HACK:
	return "h";
	
    default:
	fprintf( stderr, "%s:%d invalid setting of %d\n",
		 __FILE__, __LINE__, setting );
	/* fall through */

    case SETTING_UNKNOWN:
	return "?";
    }
}


char *train_to_str( int train )
{
    switch( train )
    {
    case TRAIN_SLOW:
	return "slow train (4.5 bps)";

    case TRAIN_QUICK:
	return "quick train (5 bps)";

    case TRAIN_QS:
	return "either quick or slow train";

    case TRAIN_FAST:
	return "fast train (5.5 bps)";

    case TRAIN_VFAST:
	return "very fast train (6 bps)";

    case TRAIN_JBUG:
	return "jitter bug (40 bps)";

    default:
	fprintf( stderr, "%s:%d invalid train of %d\n",
		 __FILE__, __LINE__, train );
	/* fall through */

    case TRAIN_UNKNOWN:
	return "?";
    }
}


char *train_to_instr( int train )
{
    switch( train )
    {
    case TRAIN_SLOW:
	return "s";

    case TRAIN_QUICK:
	return "q";

    case TRAIN_QS:
	return "b";

    case TRAIN_FAST:
	return "f";

    case TRAIN_VFAST:
	return "v";

    case TRAIN_JBUG:
	return "j";

    default:
	fprintf( stderr, "%s:%d invalid train of %d\n",
		 __FILE__, __LINE__, train );
	/* fall through */

    case TRAIN_UNKNOWN:
	return "?";
    }
}


char *jewel_to_str( int jewel )
{
    static int	buf_cnt = 0;
    static char	buf[ 8 ][ 32 ];
    
    if( ++buf_cnt >= array_elem( buf ) )
	buf_cnt = 0;

    sprintf( buf[ buf_cnt ], "%d", jewel );

    return buf[ buf_cnt ];
}


char *regulator_to_str( int regulator )
{
    switch( regulator )
    {
    case REG_FREE:
	return "free";

    case REG_ELGIN:
	return "Elgin patent";

    case REG_RR_PAT:
	return "Railroad patent";

    case REG_OLD_RR_PAT:
	return "Old Style patent";

    case REG_BALL:
	return "Ball";

    case REG_HULBURD:
	return "Hulburd";

    case REG_DURABALANCE:
	return "DuraBalance";

    default:
	fprintf( stderr, "%s:%d invalid regulator type of %d\n",
		 __FILE__, __LINE__, regulator );
	/* fall through */

    case REG_UNKNOWN:
	return "?";
    }
}


char *reg_to_instr( int regulator )
{
    switch( regulator )
    {
    case REG_FREE:
	return "f";

    case REG_ELGIN:
	return "e";

    case REG_RR_PAT:
	return "r";

    case REG_OLD_RR_PAT:
	return "o";
	
    case REG_BALL:
	return "b";

    case REG_HULBURD:
	return "h";

    case REG_DURABALANCE:
	return "d";

    default:
	fprintf( stderr, "%s:%d invalid regulator type of %d\n",
		 __FILE__, __LINE__, regulator );
	/* fall through */

    case REG_UNKNOWN:
	return "?";
    }
}


char *barrel_to_str( int barrel )
{
    switch( barrel )
    {
    case BARREL_GOING:
	return "going";

    case BARREL_MOTOR:
	return "motor";

    case BARREL_JMOTOR:
	return "jewelled motor";

    default:
	fprintf( stderr, "%s:%d invalid barrel type of %d\n",
		 __FILE__, __LINE__, barrel );
	/* fall through */

    case BARREL_UNKNOWN:
	return "?";
    }
}


char *barrel_to_instr( int barrel )
{
    switch( barrel )
    {
    case BARREL_GOING:
	return "gb";

    case BARREL_MOTOR:
	return "mb";

    case BARREL_JMOTOR:
	return "jb";

    default:
	fprintf( stderr, "%s:%d invalid barrel type of %d\n",
		 __FILE__, __LINE__, barrel );
	/* fall through */

    case BARREL_UNKNOWN:
	return "?";
    }
}



char *range_to_str( int min_val, int max_val )
{
    static int	buf_cnt = 0;
    static char	buf[ 8 ][ 32 ];
    
    if( ++buf_cnt >= array_elem( buf ) )
	buf_cnt = 0;

    if( min_val == max_val )
	sprintf( buf[ buf_cnt ], "%d", min_val );
    else
	sprintf( buf[ buf_cnt ], "%d-%d", min_val, max_val );

    return buf[ buf_cnt ];
}



char *adj_to_str( int min_val, int max_val )
{
    static int	buf_cnt = 0;
    static char	buf[ 8 ][ 32 ];
    
    if( ++buf_cnt >= array_elem( buf ) )
	buf_cnt = 0;

    if( min_val == max_val )
	if( min_val == 1 )
	    sprintf( buf[ buf_cnt ], "Adj" );
	else
	    sprintf( buf[ buf_cnt ], "%s", adj_names[ min_val ] );
    else
	sprintf( buf[ buf_cnt ], "%s-%s",
		 adj_names[ min_val ], adj_names[ max_val ] );

    return buf[ buf_cnt ];
}



char *grade_to_code_str( grade_type *g )
{
    static int	buf_cnt = 0;
    static char	buf[ 8 ][ 32 ];
    char	*p;
    
    if( ++buf_cnt >= array_elem( buf ) )
	buf_cnt = 0;


    p = buf[ buf_cnt ];

    *p++ = style_to_instr( g->style )[0];
    
    if( g->bridge )
	*p++ = 'b';
    else if( g->plate == PLATE_FULL )
	*p++ = 'f';
    else if( g->plate == PLATE_3_4 )
	*p++ = '3';
    else
	*p++ = '?';
    
    *p++ = finish_to_instr( g->finish )[0];

    if( g->min_model == MODEL_UNKNOWN )
	*p++ = '?';
    else
	p += sprintf( p, "%d", g->min_model );

    *p++ = setting_to_instr( g->setting )[0];

    if( g->train != TRAIN_QUICK )
	*p++ = train_to_instr( g->train )[0];


    *p = '\0';
	

    return buf[ buf_cnt ];
}



char *sn_to_str( uchar letter, int sn )
{
    static int	buf_cnt = 0;
    static char	buf[ 8 ][ 32 ];
    char	*p;
    
    if( ++buf_cnt >= array_elem( buf ) )
	buf_cnt = 0;


    p = buf[ buf_cnt ];

    if( letter == '_' )
	sprintf( p, "%d", sn );
    else
	sprintf( p, "%c%d", letter, sn );

    return buf[ buf_cnt ];
}



#define ENP_OK		0
#define ENP_INVALID_NUM	1
#define ENP_NOT_A_NUM	2

int expand_num_pat( char *num_pat, num_pat_type *np )
{
    int		i, j;
    int		digit, prev_digit;
    int		found_digit;
    int		digits_in_pat;
    int		star_count;
    int		could_be_max;


    memset( np, 0, sizeof( *np ) );
    
    /*
     * figure out the number of non-'*' digits that the pattern has so
     * that we can figure out what to do with the '*'
     */

    digits_in_pat = 0;
    for( i = 0; num_pat[ i ] != '\0'; i++ )
    {
	digits_in_pat++;

	if( num_pat[ i ] == '[' )    
	{
	    for( ; num_pat[ i ] != '\0' && num_pat[ i ] != ']'; i++ )
		;
	}
    }
    

    /* expand all '?' and set patterns to be simple sets.  That is, '?'
     * gets translated into a set of all digits, and sets are expanded
     * to not use ranges.  Also, sets have their digits sorted.
     *
     * this also validates the pattern.
     */

    np->num_digits = 0;
    star_count = -1;
    j = 0;
    for( i = 0; num_pat[ i ] != '\0'; i++ )
    {
	/* ignore any 3-digit seperators, both american (,) or european (.) */
	if( num_pat[ i ] == ',' || num_pat[ i ] == '.' )
	    continue;
	
	if( j >= MAX_PAT_DIGITS )
	{
	    if( num_pat[ i ] == '*' )
	    {
		fprintf( stderr,
			 "%s:%d star_count calculated incorrectly: %d\n",
			 __FILE__, __LINE__, star_count );
		break;
	    }
	    else
		return ENP_INVALID_NUM;	/* pattern too long		*/
	}

	if( isdigit( num_pat[ i ] ) )
	{
	    np->digit_set[ j ][ num_pat[ i ] - '0' ] = TRUE;
	    j++;
	}
	
	else if( num_pat[ i ] == '?'
		 || num_pat[ i ] == 'x' || num_pat[ i ] == 'X'
	    )
	{
	    memset( np->digit_set[ j ], TRUE, sizeof( np->digit_set[ j ] ) );
	    j++;
	}

	else if( num_pat[ i ] == '*' )
	{
	    if( star_count == -1 )
	    {
		star_count = MAX_PAT_DIGITS - digits_in_pat + 1;

		/* should '*' should expand the number to only 7 digits? */
		could_be_max = FALSE;

		if( j == 0 )
		    could_be_max = TRUE;
		
		else if( np->digit_set[ 0 ][ 0 ]
			 || np->digit_set[ 0 ][ 1 ]
			 || np->digit_set[ 0 ][ 2 ]
			 || np->digit_set[ 0 ][ 3 ]
		    )
		    could_be_max = TRUE;


		else if( j < 2
			 || (np->digit_set[ 0 ][ 4 ]
			     && ( np->digit_set[ 1 ][ 0 ]
				  || np->digit_set[ 1 ][ 1 ]
				  || np->digit_set[ 1 ][ 2 ]
				  || np->digit_set[ 1 ][ 3 ]
				 )
			     )
		    )
		    could_be_max = TRUE;

		else if( j < 2
			 || (np->digit_set[ 0 ][ 5 ]
			     && np->digit_set[ 1 ][ 0 ]
			     )
			 )
		    could_be_max = TRUE;

		if( !could_be_max )
		    star_count--;
	    }
	    
	    memset( np->digit_set[ j ], TRUE, sizeof( np->digit_set[ j ] ) );
	    j++;

	    star_count--;
	    if( star_count > 0 )
		i--;
	}

	else if( num_pat[ i ] == '[' )
	{
	    i++;
	    
	    found_digit = FALSE;
	    prev_digit = 10;
	    while( TRUE )
	    {
		if( num_pat[ i ] == '\0' )
		    return ENP_INVALID_NUM; /* end of set ']' not found	*/

		else if( isdigit( num_pat[ i ] ) )
		{
		    prev_digit = num_pat[ i ] - '0';
		    np->digit_set[ j ][ prev_digit ] = TRUE;
		    found_digit = TRUE;
		}

		else if( num_pat[ i ] == ']' )
		    break;
		
		else if( num_pat[ i ] == '-' )
		{
		    i++;

		    if( !isdigit( num_pat[ i ] ) )
			return ENP_INVALID_NUM;

		    digit = num_pat[ i ] - '0';

		    if( digit <= prev_digit )
			return ENP_INVALID_NUM;

		    while( prev_digit <= digit )
			np->digit_set[ j ][ prev_digit++ ] = TRUE;

		    prev_digit = 10;
		    found_digit = TRUE;
		}
		else
		    return ENP_INVALID_NUM;

		i++;
	    }
	    j++;
	    
	    if( !found_digit )
		return ENP_INVALID_NUM;	/* empty set			*/
	}

	else if( np->num_digits > 1 )
	    return ENP_INVALID_NUM;	/* has to be a SN pat w/ bad char */

	else
	    return ENP_NOT_A_NUM;	/* invalid character: maybe keyword? */
    }
    np->num_digits = j;
    
    if( np->num_digits == 0 )
	return ENP_NOT_A_NUM;		/* empty pattern		*/

    for( i = np->num_digits - 1; i >= 0; i-- )
    {
	np->digits_in_set[ i ] = 0;

	if( i == np->num_digits - 1 )
	    np->digit_val[ i ] = 1;
	else
	    np->digit_val[ i ] = 10 * np->digit_val[ i + 1 ];

	for( j = 0; j < 10; j++ )
	    if( np->digit_set[ i ][ j ] )
		np->digits_in_set[ i ]++;

	if( np->digits_in_set[ i ] == 0 )
	{
	    fprintf( stderr, "%s:%d  internal error:  no digits in set %d\n",
		     __FILE__, __LINE__, i );

	    return ENP_INVALID_NUM;
	}


	for( j = 0; j < 10; j++ )
	    if( np->digit_set[ i ][ j ] )
	    {
		if( i == np->num_digits - 1 )
		    np->min_val[ i ] = j;
		else
		    np->min_val[ i ]
			= np->min_val[ i + 1 ] + j * np->digit_val[ i ];
		break;
	    }
	
	for( j = 9; j >= 0; j-- )
	    if( np->digit_set[ i ][ j ] )
	    {
		if( i == np->num_digits - 1 )
		    np->max_val[ i ] = j;
		else
		    np->max_val[ i ]
			= np->max_val[ i + 1 ] + j * np->digit_val[ i ];
		break;
	    }
    }
    
    return ENP_OK;
}



char *num_pat_to_str( char pref, num_pat_type np )
{
    static char	expanded_pat[ MAX_PAT_DIGITS * MAX_CHAR_PER_DIGIT + 1 + 1 ];
    char	*p;
    int		i, j, k;
    int		digit_printed;
    
    p = expanded_pat;

    if( pref != '\0' )
	*p++ = pref;
    
    digit_printed = FALSE;
    
    for( i = 0; i < np.num_digits; i++ )
    {
	for( j = 0; j < 10; j++ )
	    if( np.digit_set[ i ][ j ] )
		break;
	
	if( np.digits_in_set[ i ] == 1 )
	{
	    if( j == 0 )
	    {
		if( digit_printed )
		    *p++ = '0';
	    } else {
		digit_printed = TRUE;
		*p++ = j + '0';
	    }
	}
	
	else if( np.digits_in_set[ i ] == 10 )
	{
	    if( pref != '*' || digit_printed )
	    {
		digit_printed = TRUE;
		*p++ = '?';
	    }
	}

	else
	{
	    digit_printed = TRUE;
	    *p++ = '[';

	    do
	    {
		*p++ = j + '0';

		for( k = j+1; k < 10; k++ )
		    if( !np.digit_set[ i ][ k ] )
			break;

		if( k > j + 2 )
		{
		    *p++ = '-';
		    *p++ = k-1 + '0';
		} else {
		    for( k = j+1; k < 10; k++ )
			if( !np.digit_set[ i ][ k ] )
			    break;
			else
			    *p++ = k + '0';
		}

		if( k == 10 )
		    break;

		for( j = k; j < 10; j++ )
		    if( np.digit_set[ i ][ j ] )
			break;
	    }
	    while( j != 10 );

	    *p++ = ']';
	}
    }

    *p++ = '\0';
    
    return expanded_pat;
}
	    
    

/*
 * note:  np is passed as a pointer just to keep the structure from being
 *        copied a billion times and to make debugging with gdb easier.  It
 *        is never modified.
 */

int num_pat_in_rangex( num_pat_type *np, int digit, int partial_val,
		       num_pat_type *out_np,
		       int min_range, int max_range )
{
    int		i;
    int		found_valid;
    int		tmp;
    
    /* did we search all digits, if so, we found a match */
    if( digit == np->num_digits
	&& partial_val >= min_range
	&& partial_val <= max_range
	)
    {
	return TRUE;
    }
    

    /* are we completely out of the range? */
    if( partial_val + np->max_val[ digit ] < min_range
	|| partial_val + np->min_val[ digit ] > max_range )
	return FALSE;

    /* are we complete within the range? */
    if( partial_val + np->min_val[ digit ] >= min_range
	&& partial_val + np->max_val[ digit ] <= max_range )
    {
	for( i = digit; i < np->num_digits; i++ )
	{
	    memcpy( out_np->digit_set[ i ], np->digit_set[ i ],
		    sizeof( out_np->digit_set[ i ] ) );
	    out_np->digits_in_set[ i ] = np->digits_in_set[ i ];
	    out_np->min_val[ i ] = np->min_val[ i ];
	    out_np->max_val[ i ] = np->max_val[ i ];
	    out_np->digit_val[ i ] = np->digit_val[ i ];
	}

	return TRUE;
    }

    /* maybe we only have a partial match */
    found_valid = FALSE;
    for( i = 0; i < 10; i++ )
    {
	if( !np->digit_set[ digit ][ i ] )
	    continue;
	
	if( !num_pat_in_rangex( np, digit + 1,
				partial_val + i * np->digit_val[ digit ],
				out_np,
				min_range, max_range ) )
	    continue;
	
	found_valid = TRUE;
		
	if( !out_np->digit_set[ digit ][ i ] )
	    out_np->digits_in_set[ digit ]++;
	
	out_np->digit_set[ digit ][ i ] = TRUE;
		    
	if( digit == np->num_digits - 1 )
	    tmp = i;
	else
	    tmp = out_np->min_val[ digit + 1 ]
		+ i * out_np->digit_val[ digit ];

	if( tmp < out_np->min_val[ digit ] )
	    out_np->min_val[ digit ] = tmp;
		    
	if( digit == np->num_digits - 1 )
	    tmp = i;
	else
	    tmp = out_np->max_val[ digit + 1 ]
		+ i * out_np->digit_val[ digit ];

	if( tmp > out_np->max_val[ digit ] )
	    out_np->max_val[ digit ] = tmp;
    }

    return found_valid;
}



int num_pat_in_range( num_pat_type np, 
		      num_pat_type *out_np,
		      int min_range, int max_range )
{
    int		i;
    

    /* initialize the output pattern */

    out_np->num_digits = np.num_digits;

    memset( out_np->digit_set, FALSE, sizeof( out_np->digit_set ) );

    for( i = 0; i < np.num_digits; i++ )
    {
	out_np->digits_in_set[ i ] = 0;
	out_np->digit_val[ i ] = np.digit_val[ i ];
	out_np->min_val[ i ] = np.digit_val[ i ] * 10;
	out_np->max_val[ i ] = 0;
    }
	
    return num_pat_in_rangex( &np, 0, 0, out_np, min_range, max_range );
}

		
	

void read_sn_info( FILE *sn_file, sn_range_type *snr, int *num_snr,
		   grade_type *gr )
{
    int		line;
    int		len;

    int		sn, prev_sn[ 256 ];
    int		grade;
    uchar	letter;
    int		ret;

    char	in_line[4096], *p;

    char	name_str[81];
    int		name_val;
    int		i;


    *num_snr = 0;
    for( i = 0; i < array_elem( prev_sn ); i++ ) 
	prev_sn[ i ] = -1;
    line = 0;
    
    while( 1 )
    {
	if( fgets( in_line, sizeof( in_line ), sn_file ) == NULL )
	    break;
	
	line++;

	/* record any comment lines */
	for( p = in_line; isspace( *p ); p++ )
	    ;
	
	if( *p == '#' || *p == '\0' )
	{
	    len = strlen( in_line ) + 1; /* for the '\0'		*/

	    if( snr[ *num_snr ].comment_lines == NULL )
	    {
		snr[ *num_snr ].comment_lines = malloc( len );

		strcpy( snr[ *num_snr ].comment_lines, in_line );
	    } else {
		len += strlen( snr[ *num_snr ].comment_lines ) + 1;
		snr[ *num_snr ].comment_lines
		    = realloc( snr[ *num_snr ].comment_lines, len );
	    
		strcat( snr[ *num_snr ].comment_lines, in_line );
	    }

	    continue;
	}
	

	/* clean up the line to make it easier to parse */
	p = strchr( in_line, '\n' );
	if( p != NULL )
	    *p = '\0';

	for( p = in_line; isspace( *p ); p++ )
	    ;

	if( *p == '\0' )
	{
	    /* blank lines should be considered comment lines... */
	    fprintf( stderr, "%s:%d  unexpected blank line in "
		     "serial number database.\n",
		     __FILE__, __LINE__ );
	    continue;
	}

	++*num_snr;

	/* fetch comment */
	p = strchr( in_line, '#' );
	if( p != NULL )
	{
	    *p = '\0';
	    p++;
	    while( isspace( *p ) )
		p++;
	    snr[ *num_snr ].comments = strdup( p );
	}
	

	/* fetch everything else */
	name_str[0] = '\0';
	ret = sscanf( in_line, "%c %d %d %s\n",
		      &letter, &sn, &grade, name_str );

	if( ret != 4 && ret != 3 )
	    printf( "line %d wrong number of fields found (%d): %s\n",
		    line, ret, in_line );

	if( sn < 0
	    || (letter == '_' && sn > 50001001 )
	    || (letter != '_' && sn >  1000001 )
	    )
	    printf( "line %d  invalid serial number:  %c %d\n",
		    line, letter, sn );

	if( prev_sn[ letter ] >= sn )
	    printf( "line %d  serial numbers not in order:  %c %d >= %d\n",
		    line, letter, prev_sn[ letter ], sn );

	if( cvt_sn_letter( letter, sn ) == -1 )
	    printf( "line %d  serial number %d has invalid letter of:  "
		    "%d '%c'\n",
		    line, sn, letter, letter );

	if( grade < 0
	    || grade >= MAX_GRADE
	    || gr[ grade ].grade == GRADE_UNKNOWN )
	    printf( "line %d  unknown grade:  %d\n",
		    line, grade );

	if( strcmp( name_str, "-" ) == 0 )
	{
	    name_val = N_NONE;

	    for( i = 0; i < MAX_GRADE_NAME; i++ )
		if( name_val == gr[ grade ].name[ i ] )
		    break;

	    if( i == MAX_GRADE_NAME )
	    {
		printf( "line %d  serial number %d has name of "
			"\"%s\" which isn't valid for grade %d.\n",
			line, sn, name_str, grade );
	    }
	}
	
	else if( name_str[0] != '\0' )
	{
	    name_val = N_NONE;
	    for( i = 1; i < MAX_NAME; i++ )
		if( strcmp( name_str, mvt_names[i] ) == 0 )
		{
		    name_val = i;
		    break;
		}

	    if( name_val == N_NONE )
	    {
		printf( "line %d  serial number %d has invalid name of "
			"\"%s\"\n",
			line, sn, name_str );
	    }

	    for( i = 0; i < MAX_GRADE_NAME; i++ )
		if( name_val == gr[ grade ].name[ i ] )
		    break;

	    if( i == MAX_GRADE_NAME )
	    {
		printf( "line %d  serial number %d has name of "
			"\"%s\" which isn't valid for grade %d.\n",
			line, sn, name_str, grade );
	    }
		
	    else if( gr[ grade ].name[ 1 ] == N_DEFER )
	    {
		printf( "line %d  serial number %d has name of "
			"\"%s\" which is redundant for grade %d.\n",
			line, sn, name_str, grade );
	    }
		
	}
	else
	    name_val = N_DEFER;

    
	prev_sn[ letter ] = sn;

	snr[ *num_snr ].sn = sn;
	snr[ *num_snr ].letter = letter;
	snr[ *num_snr ].grade = grade;
	snr[ *num_snr ].name = name_val;
	snr[ *num_snr ].line = line;

	snr[ *num_snr ].year = elgin_prod_year( &snr[ *num_snr ] );
	snr[ *num_snr ].next_grade = -1;
	snr[ *num_snr ].next_size = -1;
	snr[ *num_snr ].next_bridge = -1;
	snr[ *num_snr ].next_bridge_in_size = -1;
	snr[ *num_snr ].next_style = -1;
	snr[ *num_snr ].next_style_in_size = -1;
	snr[ *num_snr ].next_finish = -1;
	snr[ *num_snr ].next_finish_in_size = -1;
	snr[ *num_snr ].next_model_in_size = -1;
	snr[ *num_snr ].next_setting = -1;
	snr[ *num_snr ].next_setting_in_size = -1;
	snr[ *num_snr ].next_jewel = -1;
	snr[ *num_snr ].next_jewel_in_size = -1;
	snr[ *num_snr ].next_wi_option = -1;
	snr[ *num_snr ].next_wi_option_in_size = -1;
	snr[ *num_snr ].next_regulator = -1;
	snr[ *num_snr ].next_regulator_in_size = -1;
	snr[ *num_snr ].next_barrel = -1;
	snr[ *num_snr ].next_barrel_in_size = -1;
	for( i = 0;i < MAX_GRADE_NAME; i++ )
	{
	    snr[ *num_snr ].next_name[ i ] = -1;
	    snr[ *num_snr ].next_name_in_size[ i ] = -1;
	}
    }

}


void read_grade_info( FILE *gr_file, grade_type *gr, int do_verify )
{
    int		i, j;
    
    int		line;

    int		grade, prev_grade;
    int		class;
    int		dial_size;
    int		mvt_size;
    int		plate;
    int		style;
    int		bridge;
    int		finish;
    int		min_model, max_model;
    int		setting;
    int		train;
    int		min_jewels, max_jewels;
    int		min_adj, max_adj;
    int		wi_option;
    int		wind_hrs;
    int		regulator;
    int		barrel;

    char	*name_str;
    int		name_val;

    char	*adj_str;
    int		has_max_adj;

    char	in_line[4096], in_line_copy[4096];
    char	*p, *end_p;
    int		len;


    prev_grade = 0;

    
    memset( gr, 0, sizeof( *gr ) * MAX_GRADE );
    for( grade = 0; grade < MAX_GRADE; grade++ )
    {
	gr[ grade ].grade = GRADE_UNKNOWN;
	gr[ grade ].class = CLASS_UNKNOWN;
	gr[ grade ].mvt_size = SIZE_UNKNOWN;
	gr[ grade ].dial_size = SIZE_UNKNOWN;
	gr[ grade ].plate = PLATE_UNKNOWN;
	gr[ grade ].bridge = FALSE;
	gr[ grade ].style = STYLE_UNKNOWN;
	gr[ grade ].finish = FINISH_UNKNOWN;
	gr[ grade ].min_model = MODEL_UNKNOWN;
	gr[ grade ].max_model = MODEL_UNKNOWN;
	gr[ grade ].setting = SETTING_UNKNOWN;
	gr[ grade ].train = TRAIN_UNKNOWN;
	gr[ grade ].min_jewels = 0;
	gr[ grade ].max_jewels = 0;
	gr[ grade ].min_adj = ADJ_UNKNOWN;
	gr[ grade ].max_adj = ADJ_UNKNOWN;
	gr[ grade ].wi_option = FALSE;
	gr[ grade ].wind_hrs = DEFAULT_WIND_HRS;
	gr[ grade ].regulator = REG_UNKNOWN;
	gr[ grade ].barrel = BARREL_UNKNOWN;
	
	for( j = 0; j < array_elem( gr[ 0 ].name ); j++ )
	    gr[ grade ].name[ j ] = N_DEFER;

	gr[ grade ].comments = NULL;
	gr[ grade ].comment_lines = NULL;

	gr[ grade ].first_snr = -1;
	gr[ grade ].last_snr = -1;
    }
    gr[ 0 ].grade = 0;
	

    line = 0;
    while( 1 )
    {
	if( fgets( in_line, sizeof( in_line ), gr_file ) == NULL )
	    break;

	line++;

	/* record any comment lines */
	for( p = in_line; isspace( *p ); p++ )
	    ;
	
	if( *p == '#' || *p == '\0' )
	{
	    len = strlen( in_line ) + 1; /* for the '\0'		*/

	    if( gr[ prev_grade ].comment_lines == NULL )
	    {
		gr[ prev_grade ].comment_lines = malloc( len );

		strcpy( gr[ prev_grade ].comment_lines, in_line );
	    } else {
		len += strlen( gr[ prev_grade ].comment_lines );
		gr[ prev_grade ].comment_lines
		    = realloc( gr[ prev_grade ].comment_lines, len );
	    
		strcat( gr[ prev_grade ].comment_lines, in_line );
	    }

	    continue;
	}
	

	/* clean up the line to make it easier to parse */
	strcpy( in_line_copy, in_line );
	p = strchr( in_line_copy, '\n' );
	if( p != NULL )
	    *p = '\0';

	for( p = in_line_copy; *p != '\0'; p++ )
	    if( isspace( *p ) )
		*p = ' ';
	*p++ = ' ';		/* so the str_prefix( p, "? " ) doesn't fail */
	*p = '\0';

	for( p = in_line_copy; *p == ' '; p++ )
	    ;

	if( *p == '\0' )
	{
	    /* blank lines should be considered comment lines... */
	    fprintf( stderr, "%s:%d  unexpected blank line in "
		     "grade database\n",
		     __FILE__, __LINE__ );
	    continue;
	}


	/* fetch grade */
	grade = strtol( p, &end_p, 10 );

	if( p == end_p )
	{
	    printf( "grade db: line %d grade not found in line: %s\n",
		    line, in_line );
	    
	    continue;
	}
	else if( grade <= 0 || grade >= MAX_GRADE )
	{
	    printf( "grade db: line %d invalid grade of %d\n",
		    line, grade );
	    
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;

	if( do_verify && grade != prev_grade + 1 )
	{
	    if( grade == prev_grade + 2 )
		printf( "grade db: line %d gap in grades %d\n",
			line, prev_grade + 1 );
	    else
		printf( "grade db: line %d gap in grades %d .. %d\n",
			line, prev_grade + 1, grade - 1 );
	}
	

	/* fetch class */
	if( str_prefix( p, "? " ) )
	{
	    class = CLASS_UNKNOWN;
	    end_p = p + 1;
	} else {
	    class = strtol( p, &end_p, 10 );
	
	    if( p == end_p )
	    {
		printf( "grade db: line %d class not found in line: %s\n",
			line, in_line );
		
		continue;
	    }
	    else if( class <= 0 || class > MAX_CLASS )
	    {
		printf( "grade db: line %d invalid class of %d\n",
			line, class );
	    
		continue;
	    }
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch dial size */
	if( str_prefix( p, "? " ) )
	{
	    dial_size = SIZE_UNKNOWN;
	    end_p = p + 1;
	} else {
	    dial_size = strtol( p, &end_p, 10 );

	    if( p == end_p )
	    {
		printf( "grade db: line %d dial_size not found in "
			"line: %s\n",
			line, in_line );
		    
		continue;
	    }
		
	    if( str_prefix( end_p, "/0 " ) )
	    {
		dial_size = 1 - dial_size;
		end_p += sizeof( "/0 " ) - 1;
	    }

	    if( dial_size < MIN_SIZE || dial_size > MAX_SIZE )
	    {
		printf( "grade db: line %d invalid dial_size of %d\n",
			line, dial_size );
		
		continue;
	    }
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch mvt size */
	if( str_prefix( p, "? " ) )
	{
	    mvt_size = SIZE_UNKNOWN;
	    end_p = p + 1;
	}
	else if( str_prefix( p, "- " ) )
	{
	    mvt_size = dial_size;
	    end_p = p + 1;
	} else {
	    mvt_size = strtol( p, &end_p, 10 );

	    if( p == end_p )
	    {
		printf( "grade db: line %d mvt_size not found in "
			"line: %s\n",
			line, in_line );
		    
		continue;
	    }
		
	    if( str_prefix( end_p, "/0 " ) )
	    {
		mvt_size = 1 - mvt_size;
		end_p += sizeof( "/0 " ) - 1;
	    }

	    if( mvt_size < MIN_SIZE || mvt_size > MAX_SIZE
		|| dial_size < mvt_size )
	    {
		printf( "grade db: line %d invalid mvt_size of %d\n",
			line, mvt_size );
		
		continue;
	    }
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch plate */
	if( str_prefix( p, "? " ) )
	{
	    plate = PLATE_UNKNOWN;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "fp " ) )
	{
	    plate = PLATE_FULL;
	    end_p = p + sizeof( "fp " ) - 1;
	}
	else if( str_prefix( p, "3/4 " ) )
	{
	    plate = PLATE_3_4;
	    end_p = p + sizeof( "3/4 " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid plate specification: %s\n",
		    line, p );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch style and bridge */
	/* (why does Elgin tie bridge to style not plate?  I haven't a clue) */
	if( str_prefix( p, "? " ) )
	{
	    style = STYLE_UNKNOWN;
	    bridge = FALSE;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "h " ) )
	{
	    style = STYLE_HC;
	    bridge = FALSE;
	    end_p = p + sizeof( "h " ) - 1;
	}
	else if( str_prefix( p, "hb " ) )
	{
	    style = STYLE_HC;
	    bridge = TRUE;
	    end_p = p + sizeof( "hb " ) - 1;
	}
	else if( str_prefix( p, "o " ) )
	{
	    style = STYLE_OF;
	    bridge = FALSE;
	    end_p = p + sizeof( "o " ) - 1;
	}
	else if( str_prefix( p, "ob " ) )
	{
	    style = STYLE_OF;
	    bridge = TRUE;
	    end_p = p + sizeof( "ob " ) - 1;
	}
	else if( str_prefix( p, "c " ) )
	{
	    style = STYLE_CVT;
	    bridge = FALSE;
	    end_p = p + sizeof( "c " ) - 1;
	}
	else if( str_prefix( p, "cb " ) )
	{
	    style = STYLE_CVT;
	    bridge = TRUE;
	    end_p = p + sizeof( "cb " ) - 1;
	}
	else if( str_prefix( p, "s " ) )
	{
	    style = STYLE_SS;
	    bridge = FALSE;
	    end_p = p + sizeof( "s " ) - 1;
	}
	else if( str_prefix( p, "- " ) )
	{
	    style = STYLE_NONE;
	    bridge = FALSE;
	    end_p = p + sizeof( "- " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid style specification\n",
		    line );
		
	    continue;
	}

	if( do_verify && bridge && plate != PLATE_3_4 )
	    printf( "grade db: line %d warning bridge style, "
		    "but not 3/4 pl\n ",
		    line );

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch finish */
	if( str_prefix( p, "? " ) )
	{
	    finish = FINISH_UNKNOWN;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "g " ) )
	{
	    finish = FINISH_GILDED;
	    end_p = p + sizeof( "g " ) - 1;
	}
	else if( str_prefix( p, "n " ) )
	{
	    finish = FINISH_NICKEL;
	    end_p = p + sizeof( "n " ) - 1;
	}
	else if( str_prefix( p, "t " ) )
	{
	    finish = FINISH_TWO_TONE;
	    end_p = p + sizeof( "t " ) - 1;
	}
	else if( str_prefix( p, "f " ) )
	{
	    finish = FINISH_FLAT;
	    end_p = p + sizeof( "f " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid finish specification\n",
		    line );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch model */
	if( str_prefix( p, "? " ) )
	{
	    min_model = MODEL_UNKNOWN;
	    max_model = MODEL_UNKNOWN;
	    end_p = p + 1;
	} else {
	    min_model = strtol( p, &end_p, 10 );
	
	    if( p == end_p )
	    {
		printf( "grade db: line %d model number not found in line: %s\n",
			line, in_line );
		
		continue;
	    }

	    if( min_model < MIN_MODEL || min_model > MAX_MODEL )
	    {
		printf( "grade db: line %d invalid min_model of %d\n",
			line, min_model );
		
		continue;
	    }

	    if( end_p[0] == '-' )
	    {
		p = end_p + 1;
		
		max_model = strtol( p, &end_p, 10 );
	
		if( p == end_p )
		{
		    printf( "grade db: line %d model number not found "
			    "in line: %s\n",
			    line, in_line );
		
		    continue;
		}

		if( max_model < MIN_MODEL || max_model > MAX_MODEL )
		{
		    printf( "grade db: line %d invalid max_model of %d\n",
			    line, max_model );
		
		    continue;
		}
	    }
	    else
		max_model = min_model;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch setting */
	if( str_prefix( p, "? " ) )
	{
	    setting = SETTING_UNKNOWN;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "k " ) )
	{
	    setting = SETTING_KEY;
	    end_p = p + sizeof( "k " ) - 1;
	}
	else if( str_prefix( p, "l " ) )
	{
	    setting = SETTING_LEVER;
	    end_p = p + sizeof( "l " ) - 1;
	}
	else if( str_prefix( p, "p " ) )
	{
	    setting = SETTING_PEND;
	    end_p = p + sizeof( "p " ) - 1;
	}
	else if( str_prefix( p, "a " ) )
	{
	    setting = SETTING_AUTO;
	    end_p = p + sizeof( "a " ) - 1;
	} 
	else if( str_prefix( p, "h " ) )
	{
	    setting = SETTING_HACK;
	    end_p = p + sizeof( "h " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid setting specification\n",
		    line );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch train */
	if( str_prefix( p, "? " ) )
	{
	    train = TRAIN_UNKNOWN;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "b " ) )
	{
	    train = TRAIN_QS;
	    end_p = p + sizeof( "b " ) - 1;
	}
	else if( str_prefix( p, "s " ) )
	{
	    train = TRAIN_SLOW;
	    end_p = p + sizeof( "s " ) - 1;
	}
	else if( str_prefix( p, "q " ) )
	{
	    train = TRAIN_QUICK;
	    end_p = p + sizeof( "q " ) - 1;
	}
	else if( str_prefix( p, "f " ) )
	{
	    train = TRAIN_FAST;
	    end_p = p + sizeof( "f " ) - 1;
	}
	else if( str_prefix( p, "v " ) )
	{
	    train = TRAIN_VFAST;
	    end_p = p + sizeof( "v " ) - 1;
	}
	else if( str_prefix( p, "j " ) )
	{
	    train = TRAIN_JBUG;
	    end_p = p + sizeof( "j " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid train specification\n",
		    line );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch jewels */
	if( str_prefix( p, "? " ) )
	{
	    min_jewels = JEWELS_UNKNOWN;
	    max_jewels = JEWELS_UNKNOWN;
	    end_p = p + 1;
	} else {
	    
	    min_jewels = strtol( p, &end_p, 10 );
	
	    if( p == end_p )
	    {
		printf( "grade db: line %d jewels number not found "
			"in line: %s\n",
			line, in_line );
		
		continue;
	    }

	    if( min_jewels < MIN_JEWELS || min_jewels > MAX_JEWELS )
	    {
		printf( "grade db: line %d invalid min_jewels of %d\n",
			line, min_jewels );
		
		continue;
	    }

	    if( end_p[0] == '-' )
	    {
		p = end_p + 1;
		
		max_jewels = strtol( p, &end_p, 10 );
	
		if( p == end_p )
		{
		    printf( "grade db: line %d jewels number not found "
			    "in line: %s\n",
			    line, in_line );
		
		    continue;
		}

		if( max_jewels < MIN_JEWELS || max_jewels > MAX_JEWELS )
		{
		    printf( "grade db: line %d invalid max_jewels of %d\n",
			    line, max_jewels );
		
		    continue;
		}
	    }
	    else
		max_jewels = min_jewels;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch adjustments */
	adj_str = p;
	while( *p != ' ' && *p != '-' )
	    p++;

	has_max_adj = (*p == '-');
	*p++ = '\0';
	    
	min_adj = -1;
	for( j = 0; j < MAX_ADJ_NAME; j++ )
	    if( strcmp( adj_str, adj_names[j] ) == 0 )
	    {
		min_adj = j;
		break;
	    }

	if( min_adj == -1 )
	{
	    printf( "line %d  grade %d has invalid number of "
		    "adjustments of \"%s\"\n",
		    line, grade, adj_str );
	    continue;
	}

	if( has_max_adj )
	{
	    adj_str = p;
	    while( *p != ' ' && *p != '-' )
		p++;
	    *p++ = '\0';

	    max_adj = -1;
	    for( j = 0; j < MAX_ADJ_NAME; j++ )
		if( strcmp( adj_str, adj_names[j] ) == 0 )
		{
		    max_adj = j;
		    break;
		}

	    if( max_adj == -1 || max_adj <= min_adj )
	    {
		printf( "line %d  grade %d has invalid number of "
			"adjustments of \"%s\"\n",
			line, grade, adj_str );
		continue;
	    }
	}
	else
	    max_adj = min_adj;
	
	while( *p == ' ' )
	    p++;



	/* fetch wind indicator available flag */
	if( str_prefix( p, "- " ) )
	{
	    wi_option = FALSE;
	    end_p = p + sizeof( "- " ) - 1;
	}
	else if( str_prefix( p, "WI " ) )
	{
	    wi_option = TRUE;
	    end_p = p + sizeof( "WI " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid wind indicator flag\n",
		    line );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch wind hours */
	if( str_prefix( p, "- " ) )
	{
	    wind_hrs = DEFAULT_WIND_HRS;
	    end_p = p + 1;
	} else {
	    wind_hrs = strtol( p, &end_p, 10 );

	    if( p == end_p )
	    {
		printf( "grade db: line %d wind hrs not found in "
			"line: %s\n",
			line, in_line );
		    
		continue;
	    }
		
	    if( wind_hrs <= 0 || wind_hrs > 1000 )
	    {
		printf( "grade db: line %d invalid wind_hrs of %d\n",
			line, wind_hrs );
		
		continue;
	    }
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch regulator type */
	if( str_prefix( p, "? " ) )
	{
	    regulator = REG_UNKNOWN;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "f " ) )
	{
	    regulator = REG_FREE;
	    end_p = p + sizeof( "f " ) - 1;
	}
	else if( str_prefix( p, "e " ) )
	{
	    regulator = REG_ELGIN;
	    end_p = p + sizeof( "e " ) - 1;
	}
	else if( str_prefix( p, "r " ) )
	{
	    regulator = REG_RR_PAT;
	    end_p = p + sizeof( "r " ) - 1;
	}
	else if( str_prefix( p, "o " ) )
	{
	    regulator = REG_OLD_RR_PAT;
	    end_p = p + sizeof( "o " ) - 1;
	}
	else if( str_prefix( p, "b " ) )
	{
	    regulator = REG_BALL;
	    end_p = p + sizeof( "b " ) - 1;
	}
	else if( str_prefix( p, "h " ) )
	{
	    regulator = REG_HULBURD;
	    end_p = p + sizeof( "h " ) - 1;
	}
	else if( str_prefix( p, "d " ) )
	{
	    regulator = REG_DURABALANCE;
	    end_p = p + sizeof( "d " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid regulator specification\n",
		    line );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;


	/* fetch barrel type */
	if( str_prefix( p, "? " ) )
	{
	    barrel = BARREL_UNKNOWN;
	    end_p = p + sizeof( "? " ) - 1;
	}
	else if( str_prefix( p, "gb " ) )
	{
	    barrel = BARREL_GOING;
	    end_p = p + sizeof( "gb " ) - 1;
	}
	else if( str_prefix( p, "mb " ) )
	{
	    barrel = BARREL_MOTOR;
	    end_p = p + sizeof( "mb " ) - 1;
	}
	else if( str_prefix( p, "jb " ) )
	{
	    barrel = BARREL_JMOTOR;
	    end_p = p + sizeof( "jb " ) - 1;
	} else {
	    
	    printf( "grade db: line %d invalid barrel specification\n",
		    line );
		
	    continue;
	}

	for( p = end_p; *p == ' '; p++ )
	    ;

#if 0
	if( plate == PLATE_FULL )
	    barrel = BARREL_GOING;
	else if( max_jewels >= 23 )
	    barrel = BARREL_JMOTOR;
	else if( mvt_size == 18 && max_model >=8 )
	    barrel = BARREL_MOTOR;
	else if( max_jewels < 15 )
	    barrel = BARREL_GOING;
	else
	    barrel = BARREL_UNKNOWN;
#endif
	    


	/* fetch names */
	for( i = 0; i < array_elem( gr[ 0 ].name ); i++ )
	{
	    gr[ grade ].name[ i ] = N_DEFER;

	    if( *p == '\0' || *p == '#' )
		continue;
	    
	    name_str = strsep( &p, " " );
	    
	    if( strcmp( name_str, "-" ) == 0 )
		gr[ grade ].name[ i ] = N_NONE;
	    
	    else if( name_str != NULL && *name_str != '\0' )
	    {
		
		name_val = N_NONE;
		for( j = 1; j < MAX_NAME; j++ )
		    if( strcmp( name_str, mvt_names[j] ) == 0 )
		    {
			name_val = j;
			break;
		    }

		if( name_val == N_NONE )
		{
		    printf( "line %d  grade %d has invalid name of "
			    "\"%s\"\n",
			    line, grade, name_str );
		    continue;
		}
		gr[ grade ].name[ i ] = name_val;
	    }

	    while( *p == ' ' )
		p++;
	}


	/* fetch comments */
	if( *p == '#' )
	{
	    p++;

	    while( isspace( *p ) )
		p++;

	    len = strlen( p );
	    
	    if( len == 0 )
		gr[ grade ].comments = NULL;
	    else
	    {
		while( len > 0 && isspace( p[ len-1 ] ) )
		{
		    p[ len-1 ] = '\0';
		    len--;
		}
		
		if( len == 0 )
		    gr[ grade ].comments = NULL;
		else
		    gr[ grade ].comments = strdup( p );
	    }

	    p = "\0";
	}
	

	/* make sure we fetched everything */
	if( *p != '\0' )
	    printf( "grade db: line %d unexpected data at end of line: %s\n",
		    line, in_line );
	    


	/* update prev values */
	prev_grade = grade;
		
	
	gr[ grade ].grade = grade;
	gr[ grade ].class = class;
	gr[ grade ].dial_size = dial_size;
	gr[ grade ].mvt_size = mvt_size;
	gr[ grade ].plate = plate;
	gr[ grade ].style = style;
	gr[ grade ].bridge = bridge;
	gr[ grade ].finish = finish;
	gr[ grade ].min_model = min_model;
	gr[ grade ].max_model = max_model;
	gr[ grade ].setting = setting;
	gr[ grade ].train = train;
	gr[ grade ].min_jewels = min_jewels;
	gr[ grade ].max_jewels = max_jewels;
	gr[ grade ].min_adj = min_adj;
	gr[ grade ].max_adj = max_adj;
	gr[ grade ].wi_option = wi_option;
	gr[ grade ].wind_hrs = wind_hrs;
	gr[ grade ].regulator = regulator;
	gr[ grade ].barrel = barrel;
    }
}



void analyze_info( sn_range_type *snr, int num_snr, grade_type *grades,
		   first_runs_type *fr )
{
    int		i, j;
    sn_range_type *s;
    grade_type	*g;

    /* used to keep track of the differ "letter" serial number runs	*/
    int		prev_snr[ 256 ], psnr;
    first_runs_type	pr;
    int		size_idx;

    int		last_grade_in_size[ NUM_SIZES ];


    /* calculate the quantity in each run */
    for( i = 0; i < array_elem( prev_snr ); i++ )
	prev_snr[ i ] = -1;

    for( i = 1; i <= num_snr; i++ )
    {
	psnr = prev_snr[ snr[ i ].letter ];
	if( psnr != -1 )
	    snr[ psnr ].cnt = snr[ i ].sn - snr[ psnr ].sn;
	prev_snr[ snr[ i ].letter ] = i;
    }
	

    /*
     * initialized the "first run" info
     */
    for( i = 0; i < MAX_GRADE; i++ )
	fr->first.grade[ i ] = -1;
    fr->first.bridge = -1;
    for( i = 0; i < NUM_STYLES; i++ )
	fr->first.style[ i ] = -1;
    for( i = 0; i < NUM_FINISHES; i++ )
	fr->first.finish[ i ] = -1;
    for( i = 0; i < NUM_SETTINGS; i++ )
	fr->first.setting[ i ] = -1;
    for( i = 0; i < NUM_JEWELS; i++ )
	fr->first.jewel[ i ] = -1;
    fr->first.wi_option = -1;
    fr->first.wind_hrs = -1;
    for( i = 0; i < NUM_REGS; i++ )
	fr->first.regulator[ i ] = -1;
    for( i = 0; i < NUM_BARRELS; i++ )
	fr->first.barrel[ i ] = -1;

    for( i = 0; i < NUM_SIZES; i++ )
    {
	fr->first.size[ i ] = -1;
	fr->first.bridge_in_size[ i ] = -1;
	for( j = 0; j < NUM_STYLES; j++ )
	    fr->first.style_in_size[ i ][ j ] = -1;
	for( j = 0; j < NUM_FINISHES; j++ )
	    fr->first.finish_in_size[ i ][ j ] = -1;
	for( j = 0; j < NUM_MODELS; j++ )
	    fr->first.model_in_size[ i ][ j ] = -1;
	for( j = 0; j < NUM_SETTINGS; j++ )
	    fr->first.setting_in_size[ i ][ j ] = -1;
	for( j = 0; j < NUM_JEWELS; j++ )
	    fr->first.jewel_in_size[ i ][ j ] = -1;
	fr->first.wi_option_in_size[ i ] = -1;
	for( j = 0; j < NUM_REGS; j++ )
	    fr->first.regulator_in_size[ i ][ j ] = -1;
	for( j = 0; j < NUM_BARRELS; j++ )
	    fr->first.barrel_in_size[ i ][ j ] = -1;

	for( j = 0; j < MAX_GRADE_NAME; j++ )
	    fr->first.name_in_size[ i ][ j ] = -1;
    }
    
    for( j = 0; j < MAX_GRADE_NAME; j++ )
	fr->first.name[ j ] = -1;

    fr->prev_tot_in_gr = fr->first;
    fr->prev_snr_idx = fr->first;
    

    /*
     * calculate the grade's first_snr, last_snr, total_cnt and such
     * based off the sn data
     */
    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];
	g = &grades[ s->grade ];
	
	if( g->first_snr == -1 )
	{
	    fr->first.grade[ s->grade ] = i;
	    g->first_snr = i;
	    g->last_snr = i;
	    g->min_cnt = g->max_cnt = g->total_cnt = s->cnt;
	    g->num_runs = 1;
	} else {
	    if( g->last_snr == i-1 )
		g->num_consecutive++;
	    
	    snr[ g->last_snr ].next_grade = i;
	    g->last_snr = i;

	    if( g->min_cnt > s->cnt )
		g->min_cnt = s->cnt;

	    if( g->max_cnt < s->cnt )
		g->max_cnt = s->cnt;

	    g->total_cnt += s->cnt;

	    g->num_runs++;
	}

	if( s->comments )
	    g->sn_comments = TRUE;

	if( s->comment_lines )
	    g->sn_comment_lines = TRUE;


	size_idx = g->mvt_size - MIN_SIZE;

	SET_COUNT_VAR( size, [ size_idx ], TRUE );

	if( g->bridge )
	{
	    SET_COUNT_VAR( bridge, /* no subscript */, TRUE );
	    SET_COUNT_VAR_IN_SIZE( bridge, /* no subscript */, TRUE );
	}

	SET_COUNT_VAR( style, [ g->style ], TRUE );
	SET_COUNT_VAR_IN_SIZE( style, [ g->style ], TRUE );

	SET_COUNT_VAR( finish, [ g->finish ], g->finish != FINISH_GILDED );
	SET_COUNT_VAR_IN_SIZE( finish, [ g->finish ],
				   g->finish != FINISH_GILDED );

	SET_COUNT_VAR_IN_SIZE(
	    model, [ g->min_model ],
	    g->min_model != 1 || !g->first_in_size );

	SET_COUNT_VAR( setting, [ g->setting ], TRUE );
	SET_COUNT_VAR_IN_SIZE( setting, [ g->setting ], TRUE );

	SET_COUNT_VAR( jewel, [ g->max_jewels ],
		       (s->year < 1880 && g->max_jewels >= 15)
		       || (s->year < 1890 && g->max_jewels >= 17)
		       || g->max_jewels >= 19 );
	SET_COUNT_VAR_IN_SIZE( jewel, [ g->max_jewels ],
		       (s->year < 1880 && g->max_jewels >= 15)
		       || (s->year < 1890 && g->max_jewels >= 17)
		       || g->max_jewels >= 19 );

	if( g->wi_option )
	{
	    SET_COUNT_VAR( wi_option, /* no subscript */, TRUE );
	    SET_COUNT_VAR_IN_SIZE( wi_option, /* no subscript */, TRUE );
	}

	if( g->wind_hrs != DEFAULT_WIND_HRS )
	{
	    SET_COUNT_VAR( wind_hrs, /* no subscript */, TRUE );
	}

	SET_COUNT_VAR( regulator, [ g->regulator ], g->regulator != REG_FREE );
	SET_COUNT_VAR_IN_SIZE( regulator, [ g->regulator ],
			       g->regulator != REG_FREE );

	SET_COUNT_VAR( barrel, [ g->barrel ], g->barrel != BARREL_GOING );
	SET_COUNT_VAR_IN_SIZE( barrel, [ g->barrel ],
			       g->barrel != BARREL_GOING );
    }

    for( i = 0; i < NUM_SIZES; i++ )
	grades[ snr[ pr.first.size[ i ] ].grade ].last_produced_in_size = TRUE;
	

    /*
     * calculate info about the grades
     */
    for( i = 0; i < array_elem( last_grade_in_size ); i++ )
	last_grade_in_size[ i ] = -1;

    for( g = grades; g < &grades[MAX_GRADE]; g++ )
    {
	if( g->num_runs == 0 )
	    continue;

	g->avg_cnt = (double)g->total_cnt / g->num_runs;

	if( g->num_runs == 1 )
	    g->std_dev = 0;
	else
	{
	    for( i = g->first_snr; i != -1; i = snr[ i ].next_grade )
	    {
		if( snr[ i ].next_grade <= 0 && snr[ i ].next_grade != -1 )
		    fprintf( stderr, "%s:%d  grade %d has an invalid "
			     "next_grade of %d field on snr %d\n",
			     __FILE__, __LINE__, g->grade,
			     snr[ i ].next_grade, i );
		
		g->std_dev += sqr(snr[ i ].cnt - g->avg_cnt);
	    }
	    

	    g->std_dev = sqrt( g->std_dev / (g->num_runs - 1) );
	}

	if( g->dial_size != g->mvt_size )
	    g->flag_notes = TRUE;
	
	if( g->name[ 1 ] != N_DEFER )
	    g->flag_notes = TRUE;

	if( g->comments )
	    g->flag_notes = TRUE;

	if( g->style != STYLE_UNKNOWN
	    && g->style != STYLE_HC
	    && g->style != STYLE_OF
	    )
	    g->flag_notes = TRUE;

	if( g->finish != FINISH_UNKNOWN
	    && g->finish != FINISH_GILDED
	    && g->finish != FINISH_NICKEL )
	    g->flag_notes = TRUE;
	
	if( g->setting == SETTING_AUTO
	    || g->setting == SETTING_HACK
	    )
	    g->flag_notes = TRUE;

	if( g->train != TRAIN_QUICK )
	    g->flag_notes = TRUE;

	if( g->bridge )
	    g->flag_notes = TRUE;

	if( g->wind_hrs != DEFAULT_WIND_HRS )
	    g->flag_notes = TRUE;
	
	
	size_idx = g->mvt_size - MIN_SIZE;

	if( last_grade_in_size[ size_idx ] == -1
	    || grades[ last_grade_in_size[size_idx] ].first_snr < g->first_snr
	    )
	    last_grade_in_size[ size_idx ] = g->grade;
    }

    for( i = 0; i < array_elem( last_grade_in_size ); i++ )
	if( last_grade_in_size[ i ] != -1 )
	{
	    grades[ last_grade_in_size[ i ] ].last_grade_in_size = TRUE;
	    grades[ last_grade_in_size[ i ] ].flag_notes = TRUE;
	}
    
}



int gr_sort_mvt_size_model(const void *p_gr1, const void *p_gr2 )
{
    grade_type	*gr1 = (grade_type *) p_gr1;
    grade_type	*gr2 = (grade_type *) p_gr2;

    if( gr1->mvt_size < gr2->mvt_size )
	return -1;
    if( gr1->mvt_size > gr2->mvt_size )
	return 1;

    if( gr1->min_model < gr2->min_model )
	return -1;
    if( gr1->min_model > gr2->min_model )
	return 1;

    if( gr1->max_model < gr2->max_model )
	return -1;
    if( gr1->max_model > gr2->max_model )
	return 1;

    return gr1->grade - gr2->grade;
}
    


void verify_grade_info( grade_type *gr )
{
    int		i, j, k;
    int		max_class;

    grade_type	gr_sorted[ MAX_GRADE ];
    

    /* verify that the grade is used */
    for( i = 1; i < MAX_GRADE; i++ )
    {
	if( gr[ i ].dial_size == SIZE_UNKNOWN
	    && gr[ i ].mvt_size == SIZE_UNKNOWN
	    && gr[ i ].plate == PLATE_UNKNOWN
	    && gr[ i ].style == STYLE_UNKNOWN
	    && gr[ i ].finish == FINISH_UNKNOWN
	    && gr[ i ].min_model == MODEL_UNKNOWN
	    && gr[ i ].setting == SETTING_UNKNOWN
	    && gr[ i ].train == TRAIN_UNKNOWN
	    && gr[ i ].min_jewels == JEWELS_UNKNOWN
	    && gr[ i ].max_jewels == JEWELS_UNKNOWN
	    && gr[ i ].min_adj == ADJ_UNKNOWN
	    && gr[ i ].max_adj == ADJ_UNKNOWN
	    && gr[ i ].regulator == REG_UNKNOWN
	    && gr[ i ].barrel == BARREL_UNKNOWN
	    )
	{
	    if( gr[ i ].grade != -1 )
	    {
		printf( "%s:%d  grade %d has no known info.\n",
			__FILE__, __LINE__, i );
	    }
	
	    continue;
	}

	if( gr[ i ].grade == -1 )
	{
	    printf( "%s:%d  grade structure not initialized correctly for "
		    "grade %d.\n", __FILE__, __LINE__, i );
	    continue;
	}
	
	if( gr[ i ].first_snr == -1 )
	    printf( "grade %d is defined, but never used.\n", i );
    }
	


    /* verify the basic mvt_size/model info */
    memcpy( gr_sorted, gr, sizeof( gr_sorted ) );
    qsort( gr_sorted, MAX_GRADE, sizeof( grade_type ),
	   gr_sort_mvt_size_model );


    for( i = 1; i < MAX_GRADE; i++ )
    {
	if( gr_sorted[ i-1 ].mvt_size != gr_sorted[ i ].mvt_size
	    || gr_sorted[ i-1 ].min_model != gr_sorted[ i ].min_model
	    )
	    continue;

	if( gr_sorted[ i-1 ].min_model == MODEL_UNKNOWN
	    || gr_sorted[ i ].min_model == MODEL_UNKNOWN
	    )
	    continue;

	if( gr_sorted[ i-1 ].plate != gr_sorted[ i ].plate )
	    printf( "grade %d (%ss model %d) has plate %s, "
		    "but grade %d (%ss model %d) has plate %s\n",
		    gr_sorted[ i-1 ].grade,
		    size_to_str( gr_sorted[ i-1 ].mvt_size ),
		    gr_sorted[ i-1 ].min_model,
		    plate_to_str( gr_sorted[ i-1 ].plate ),
		    gr_sorted[ i ].grade,
		    size_to_str( gr_sorted[ i ].mvt_size ),
		    gr_sorted[ i ].min_model,
		    plate_to_str( gr_sorted[ i ].plate )
		);
	    
	if( gr_sorted[ i-1 ].style != gr_sorted[ i ].style )
	    printf( "grade %d (%ss model %d) has style %s, "
		    "but grade %d (%ss model %d) has style %s\n",
		    gr_sorted[ i-1 ].grade,
		    size_to_str( gr_sorted[ i-1 ].mvt_size ),
		    gr_sorted[ i-1 ].min_model,
		    style_to_str( gr_sorted[ i-1 ].style ),
		    gr_sorted[ i ].grade,
		    size_to_str( gr_sorted[ i ].mvt_size ),
		    gr_sorted[ i ].min_model,
		    style_to_str( gr_sorted[ i ].style )
		);
	    
	if( gr_sorted[ i-1 ].setting != gr_sorted[ i ].setting )
	    printf( "grade %d (%ss model %d) has setting %s, "
		    "but grade %d (%ss model %d) has setting %s\n",
		    gr_sorted[ i-1 ].grade,
		    size_to_str( gr_sorted[ i-1 ].mvt_size ),
		    gr_sorted[ i-1 ].min_model,
		    setting_to_str( gr_sorted[ i-1 ].setting ),
		    gr_sorted[ i ].grade,
		    size_to_str( gr_sorted[ i ].mvt_size ),
		    gr_sorted[ i ].min_model,
		    setting_to_str( gr_sorted[ i ].setting )
		);
    }
    


    /* verify the classes */
    max_class = 0;
    for( i = 1; i < MAX_GRADE; i++ )
	if( max_class < gr[ i ].class )
	    max_class = gr[ i ].class;

    for( i = 1; i <= max_class; i++ )
    {
	for( j = 1; j < MAX_GRADE; j++ )
	    if( gr[j].class == i )
		break;

	if( j == MAX_GRADE )
	{
	    printf( "Could not find a reference to class %d\n", i );
	    continue;
	}

	for( k = j+1; k < MAX_GRADE; k++ )
	{
	    if( gr[j].class != gr[k].class )
		continue;

	    if( gr[j].mvt_size != gr[k].mvt_size )
		printf( "class %d  grade %d has size %s while "
			"grade %d has size %s\n",
			i, j, size_to_str( gr[j].mvt_size ),
			k, size_to_str( gr[k].mvt_size ) );

	    if( gr[j].plate != gr[k].plate )
		printf( "class %d  grade %d has plate %s while "
			"grade %d has plate %s\n",
			i, j, plate_to_str( gr[j].plate ),
			k, plate_to_str( gr[k].plate ) );

	    if( gr[j].bridge != gr[k].bridge )
		printf( "class %d  grade %d is %s while "
			"grade %d is %s\n",
			i, j, bridge_to_str( gr[j].bridge ),
			k, bridge_to_str( gr[k].bridge ) );

	    if( gr[j].style != gr[k].style )
		printf( "class %d  grade %d has style %s while "
			"grade %d has style %s\n",
			i, j, style_to_str( gr[j].style ),
			k, style_to_str( gr[k].style ) );

	    if( gr[j].min_model != gr[k].min_model )
		printf( "class %d  grade %d has min_model %d while "
			"grade %d has min_model %d\n",
			i, j, gr[j].min_model,
			k, gr[k].min_model );
	    
	    else if( gr[j].max_model != gr[k].max_model )
		printf( "class %d  grade %d has max_model %d while "
			"grade %d has max_model %d\n",
			i, j, gr[j].max_model,
			k, gr[k].max_model );
	}
    }

}



int round_cnt( int cnt )
{
    if( cnt < 0 )
    {
	fprintf( stderr, "%s:%d  invalid count of %d\n",
		__FILE__, __LINE__, cnt );
	return 0;
    }
    else if( (cnt % 1000) == 0 )
    {
	if( cnt < 15000 )
	    return cnt;
	else if( cnt < 25000 )
	    return 20000;
	else if( cnt < 75000 )
	    return 50000;
	else
	    return 100000;
    }

    else if( (cnt % 100) == 0 )
	return cnt;

    else if( cnt > 1000 )
	return cnt - cnt % 1000 + 1;

    else if( cnt > 100 )
	return cnt - cnt % 100 + 1;

    else
    {
	cnt = (cnt + 5) - (cnt + 5) % 10;

	if( cnt == 0 )
	    return 1;
	else
	    return cnt;
    }
    
}



#define CNT_WINDOW	11
#define CNT_JUDGE	5
typedef struct
{
    int		val;
    int		freq;
} mode_info_type;

	    

void verify_sn_info( sn_range_type *snr, int num_snr,
		     grade_type *grades)
{
    int		i, j;
    grade_type	*g;
    sn_range_type	*s;

    int		snr_buf[ CNT_WINDOW ];

    mode_info_type	mode[ CNT_WINDOW ];
    int		num_mode;
    int		num_fill;
    int		max_mode;
    int		cur_mode;

    int		cnt, rcnt;
    int		num_cnt;
    int		total_cnt;
    double	avg_cnt;
    

    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];
	g = &grades[ s->grade ];

	if( (s->grade == 0 && s->cnt < 0)
	    || (s->grade != 0 && s->cnt < 1)
	    )
	    printf( "%5d:  %8s %8s %3d  invalid count %3d\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade, s->cnt );

	else if( s->cnt > 0 && s->cnt < 1000
		 && snr[ i-1 ].cnt >= 1000
		 && snr[ i+1 ].cnt >= 1000 )
	    printf( "%5d:  %8s %8s %3d  suspicious break in sn.  count %3d\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade, s->cnt );

	if( s->sn % 10 != 1 && (s->sn + s->cnt) % 100 != 1
	    && s->cnt >= 10 )
	    printf( "%5d:  %8s %8s %3d  sn bottom digit != 1.  count %3d\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade, s->cnt );

	else if( s->sn % 100 != 1 && (s->sn + s->cnt) % 1000 != 1
		 && s->cnt >= 100 )
	    printf( "%5d:  %8s %8s %3d  sn bottom two digits != 01.  count %3d\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade, s->cnt );

	else if( s->sn % 1000 != 1 && (s->sn + s->cnt) % 1000 != 1
		 && s->cnt >= 1000 )
	    printf( "%5d:  %8s %8s %3d  sn bottom three digits != 001.  "
		    "count %3d\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade, s->cnt );

	else if( (s->sn > 750000 || s->letter != '_') && s->sn % 1000 != 1 )
	    printf( "%5d:  %8s %8s %3d  sn bottom three digits != 001.  "
		    "count %3d\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade, s->cnt );


	if( s->next_grade != -1 && snr[ s->next_grade ].year - s->year > 2 )
	    printf( "%5d:  %8s %8s %3d  %d year gap between runs for this grade "
		    "(next %s)\n",
		    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ), s->grade,
		    snr[ s->next_grade ].year - s->year,
		    sn_to_str( snr[ s->next_grade ].letter,
			       snr[ s->next_grade ].sn ) );
    }


    /* see if the counts in the grades are reasonable */

    printf( "\n" );

    for( g = grades; g < &grades[MAX_GRADE]; g++ )
    {
	if( g->num_runs < 2 )
	    continue;

	num_cnt = 0;
	total_cnt = 0;
	num_fill = 0;

	memset( mode, 0, sizeof( mode ) );
	num_mode = 0;
	
	
	i = g->first_snr;
	
	while( i != -1 || (num_cnt != 0 && num_fill < CNT_JUDGE-1) )
	{
	    snr_buf[ 0 ] = i;

	    if( i != -1 )
	    {
		cnt = snr[ i ].cnt;
	    
		total_cnt += cnt;

		rcnt = round_cnt( cnt );
		for( cur_mode = 0; cur_mode < num_mode; cur_mode++ )
		{
		    if( mode[ cur_mode ].val == rcnt )
		    {
			mode[ cur_mode ].freq++;
			break;
		    }
		}

		if( cur_mode == num_mode )
		{
		    mode[ cur_mode ].val = rcnt;
		    mode[ cur_mode ].freq = 1;
		    num_mode++;
		}
	    } else {
		num_fill++;
		cnt = 0;
	    }
	    
	    if( num_cnt < CNT_WINDOW )
		num_cnt++;

	    avg_cnt = (double)total_cnt / (num_cnt - num_fill);
	    

	    /* judge the current serial number range */
	    if( num_cnt >= CNT_JUDGE )
	    {
		s = &snr[ snr_buf[ CNT_JUDGE-1 ] ];
		cnt = s->cnt;

		rcnt = round_cnt( cnt );

/*		printf( "cnt = %d   rnd_cnt = %d\n", cnt, rcnt ); */

		for( cur_mode = 0; cur_mode < num_mode; cur_mode++ )
		{
		    if( mode[ cur_mode ].val == rcnt )
			break;
		}

		max_mode = 0;
		for( j = 0; j < num_mode; j++ )
		    if( mode[ j ].freq > mode[ max_mode ].freq )
			max_mode = j;




#if 0
		printf( "%5d:  %8s %8s %3d  checking grade: "
			"%5d (avg %6.0f)\n",
			s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ),
			s->grade, s->cnt, avg_cnt );
#endif

		/* this flags about 90 ranges */
		if( (double) mode[ cur_mode ].freq / num_cnt <= .25
		    && (double) mode[ max_mode ].freq / num_cnt >= .66
		    && ( cnt < avg_cnt * .75 - 500 || cnt > avg_cnt * 1.5 + 500 )
		    )
		{
		    printf( "%5d:  %8s %8s %3d  unusual count for grade: "
			    "%5d    prob: %.2f   avg: %.0f\n",
			    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ),
			    s->grade, s->cnt,
			    (double) mode[ cur_mode ].freq / num_cnt,
			    avg_cnt );
		}
		
		else if( (double) mode[ cur_mode ].freq == 1
			 && num_mode < 6
			 && ( cnt < avg_cnt * .5 - 500 || cnt > avg_cnt * 3 + 1000 )
		    )
		{
		    printf( "%5d:  %8s %8s %3d  unusual count for grade: "
			    "%5d    avg: %.0f\n",
			    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ),
			    s->grade, s->cnt, avg_cnt );
		}
		
		/* this flagss about 150 ranges */
		else if( (double) mode[ cur_mode ].freq / num_cnt <= .15
		    && (double) mode[ max_mode ].freq / num_cnt >= .80
		    )
		{
		    printf( "%5d:  %8s %8s %3d  unusual count for grade: "
			    "%5d    prob: %.2f\n",
			    s->line, sn_to_str( s->letter, s->sn ), sn_to_str( s->letter, s->sn + s->cnt - 1 ),
			    s->grade, s->cnt,
			    (double) mode[ cur_mode ].freq / num_cnt );
		}
	    }


	    /* deleted the oldest serial number range out */
	    if( num_cnt == CNT_WINDOW )
	    {
		cnt = snr[ snr_buf[ CNT_WINDOW - 1 ] ].cnt;
		total_cnt -= cnt;

		rcnt = round_cnt( cnt );
		for( cur_mode = 0; cur_mode < num_mode; cur_mode++ )
		{
		    if( mode[ cur_mode ].val == rcnt )
		    {
			mode[ cur_mode ].freq--;
			break;
		    }
		}

		if( cur_mode == num_mode )
		{
		    printf( "%s:%d could not find \"mode info\" to delete\n",
			    __FILE__, __LINE__ );
		}
		else if( mode[ cur_mode ].freq == 0 )
		{
		    memmove( &mode[ cur_mode ], &mode[ cur_mode + 1 ],
			     (num_mode - cur_mode) * sizeof( *mode ) );
		    num_mode--;
		}
	    }

	    memmove( &snr_buf[ 1 ], &snr_buf[ 0 ],
		     (CNT_WINDOW - 1) * sizeof( *snr_buf ) );
	    snr_buf[ 0 ] = -1;
	    

	    if( i != -1 )
		i = snr[ i ].next_grade;
	}
	
    }
    
}


void print_grade_info_as_input( sn_range_type *snr, int num_snr,
				     grade_type *grades)
{
    int		i;
    
    grade_type	*g;


    if( grades[ 0 ].comment_lines )
	printf( "%s", grades[ 0 ].comment_lines );

    for( g = &grades[ 1 ]; g < &grades[MAX_GRADE]; g++ )
    {
	if( g->grade == GRADE_UNKNOWN )
	    continue;
	
	printf( "%3d ", g->grade );

	if( g->class == CLASS_UNKNOWN )
	    printf( " ?  " );
	else
	    printf( "%3d ", g->class );

	printf( "%4s ", size_to_str( g->dial_size ) );
	
	if( g->dial_size != g->mvt_size )
	    printf( "%4s ", size_to_str( g->mvt_size ) );
	else
	    printf( "  -  " );
	
	printf( "%3s ", plate_to_str( g->plate ) );
	
	printf( style_to_instr( g->style ) );
	if( g->bridge )
	    printf( "b" );
	else
	    printf( " " );
	printf( " " );
	
	printf( "%s ", finish_to_instr( g->finish ) );

	if( g->min_model == MODEL_UNKNOWN )
	    printf( " ?  " );
	else
	    printf( "%3s ", range_to_str( g->min_model, g->max_model ) );
	
	printf( "%s ", setting_to_instr( g->setting ) );

	printf( "%s ", train_to_instr( g->train ) );

	if( g->min_jewels == JEWELS_UNKNOWN )
	    printf( "    ? " );
	else
	    printf( "%5s ", range_to_str( g->min_jewels, g->max_jewels ) );

	if( g->min_adj == 1 && g->max_adj == 1 )
	    printf( "%7s ", "A" );
	else
	    printf( "%7s ", adj_to_str( g->min_adj, g->max_adj ) );

	if( g->wi_option )
	    printf( "WI " );
	else
	    printf( " - " );
	
	if( g->wind_hrs == DEFAULT_WIND_HRS )
	    printf( "  - " );
	else
	    printf( "%3d ", g->wind_hrs );

	printf( "%s ", reg_to_instr( g->regulator ) );

	printf( "%2s", barrel_to_instr( g->barrel ) );


	if( g->name[ 0 ] != N_DEFER )
	    printf( " " );
	for( i = 0; i < array_elem( g->name ); i++ )
	    if( g->name[ i ] == N_NONE )
		printf( " -" );
	    else if( g->name[ i ] != N_DEFER )
		printf( " %s", mvt_names[ g->name[ i ] ] );

	
	if( g->comments )
	    printf( "\t# %s", g->comments );

	printf( "\n" );

	if( g->comment_lines )
	    printf( "%s", g->comment_lines );

    }
}


void print_grade_info( sn_range_type *snr, int num_snr,
		       grade_type *grades,
		       char *print_grade )
{
    int		i, j;
    
    grade_type	*g;
    int		last_sn;

    printf( "grade total runs   first sn  last sn class  size   code  "
	    "jewels  Adj/name\n" );
    

    for( i = 0; i < MAX_GRADE; i++ )
    {
	g = &grades[ i ];
	
	if( g->num_runs == 0 )
	    continue;

	if( print_grade != NULL && !print_grade[ i ] )
	    continue;

	
	printf( "%3d%8d %3d",
		g - grades, g->total_cnt, g->num_runs );
	if( g->num_consecutive != 0 )
	    printf( "-%-2d", g->num_consecutive );
	else
	    printf( "   " );
	
	last_sn = snr[ g->last_snr ].sn + snr[ g->last_snr ].cnt - 1;
	printf( "%8s  %8s",
		sn_to_str( snr[ g->first_snr ].letter,
			   snr[ g->first_snr ].sn ),
		sn_to_str( snr[ g->last_snr ].letter,
			   last_sn ) );
	
	if( g->class == CLASS_UNKNOWN )
	    printf( "   ? " );
	else
	    printf( "  %3d", g->class);
	
	printf( "  %4ss  %-7s",
		size_to_str( g->mvt_size ), grade_to_code_str( g ) );

	if( g->min_jewels == JEWELS_UNKNOWN )
	    printf( "    ?j" );
	else
	    printf( "%5sj", range_to_str( g->min_jewels, g->max_jewels ) );

	if( g->max_adj != 0 )
	{
	    printf( " %s", adj_to_str( g->min_adj, g->max_adj ) );
	    if( g->name[ 0 ] != N_DEFER )
		printf( " /" );
	}

	for( j = 0; j < array_elem( g->name ); j++ )
	    if( g->name[ j ] == N_NONE )
		printf( " -" );
	    else if( g->name[ j ] != N_DEFER )
		printf( " %s", mvt_names[ g->name[ j ] ] );

	printf( "\n" );
    }
}



void print_grade_stats_info( sn_range_type *snr, int num_snr,
			     grade_type *grades)
{
    grade_type	*g;
    int		last_sn;

    printf( "grade  total  runs    first sn    last sn  min run max run   "
	    "avg run   std dev\n" );

    for( g = grades; g < &grades[MAX_GRADE]; g++ )
    {
	if( g->num_runs == 0 )
	    continue;
	
	printf( "%3d  %7d  %3d",
		g - grades, g->total_cnt, g->num_runs );
	if( g->num_consecutive != 0 )
	    printf( "/%2d", g->num_consecutive );
	else
	    printf( "   " );
	
	last_sn = snr[ g->last_snr ].sn + snr[ g->last_snr ].cnt - 1;
	printf( "  %8s   %8s   %6d  %6d  %8.2f  %8.2f\n",
		sn_to_str( snr[ g->first_snr ].letter,
			   snr[ g->first_snr ].sn ),
		sn_to_str( snr[ g->last_snr ].letter,
			   last_sn ),
		g->min_cnt, g->max_cnt, g->avg_cnt, g->std_dev );
    }
}



void print_class_info( grade_type *gr, char *print_class )
{
    int		i, j, k, l;
    int		max_class;

    int		all_same_size;
    int		all_same_style;
    int		all_same_plate;
    int		all_same_setting;
    int		all_same_model;
    

    /* verify the classes */
    max_class = 0;
    for( i = 1; i < MAX_GRADE; i++ )
	if( max_class < gr[ i ].class )
	    max_class = gr[ i ].class;

    if( print_class == NULL )
	printf( "Maximum class number:  %d\n\n", max_class );
    

    for( i = 1; i <= max_class; i++ )
    {
	if( print_class != NULL && !print_class[ i ] )
	    continue;
	
	for( j = 1; j < MAX_GRADE; j++ )
	    if( gr[j].class == i )
		break;

	if( j == MAX_GRADE )
	{
	    printf( "Could not find a reference to class %d\n\n", i );
	    continue;
	}

	all_same_size = TRUE;
	all_same_style = TRUE;
	all_same_plate = TRUE;
	all_same_setting = TRUE;
	all_same_model = TRUE;
	for( k = j; k < MAX_GRADE; k++ )
	{
	    if( gr[j].class != gr[k].class )
		continue;
	    
	    if( gr[j].mvt_size != gr[k].mvt_size )
		all_same_size = FALSE;

	    if( gr[j].style != gr[k].style )
		all_same_style = FALSE;

	    if( gr[j].plate != gr[k].plate )
		all_same_plate = FALSE;

	    if( gr[j].setting != gr[k].setting )
		all_same_setting = FALSE;

	    if( gr[j].min_model != gr[k].min_model
		|| gr[j].max_model != gr[k].max_model )
		all_same_model = FALSE;
	}

	printf( "Class %d:        ", i );

	if( all_same_size )
	    printf( " %ss", size_to_str( gr[j].mvt_size ) );

	if( all_same_style )
	    printf( " %s", style_to_str( gr[j].style ) );

	if( all_same_plate )
	    printf( " %s", plate_to_str( gr[j].plate ) );
	
	if( all_same_setting )
	    printf( " %s", setting_to_str( gr[j].setting ) );

	if( all_same_model )
	    printf( " model %s",
		    range_to_str( gr[j].min_model, gr[j].max_model ) );

	printf( "\n" );


	for( k = j; k < MAX_GRADE; k++ )
	{
	    if( gr[j].class != gr[k].class )
		continue;
	    
	    printf( "    %3d   ", k );

	    printf( " %7d made", gr[k].total_cnt );

	    if( !all_same_size )
		printf( " %ss", size_to_str( gr[k].mvt_size ) );

	    if( !all_same_style )
		printf( " %s", style_to_str( gr[k].style ) );

	    if( !all_same_plate )
		printf( " %s", plate_to_str( gr[k].plate ) );
	
	    if( !all_same_setting )
		printf( " %s", setting_to_str( gr[k].setting ) );

	    if( !all_same_model )
		printf( " model %s",
			range_to_str( gr[k].min_model, gr[k].max_model ) );

	    if( gr[k].bridge )
		printf( " bridge" );

	    printf( " %2sj",
		    range_to_str( gr[k].min_jewels, gr[k].max_jewels ) );

	    if( gr[k].max_adj != 0 )
		printf( " %s", adj_to_str( gr[k].min_adj, gr[k].max_adj ) );

	    if( gr[k].name[ 0 ] != N_DEFER )
	    {
		printf( "  Marked" );
		
		for( l = 0; l < array_elem( gr[k].name ); l++ )
		{
		    if( gr[k].name[ l ] == N_DEFER )
			break;

		    if( l > 0 )
		    {
			if( l+1 == array_elem( gr[k].name )
			    || gr[k].name[ l+1 ] == N_DEFER
			    )
			    printf( " or" );
			else
			    printf( "," );
		    }
		    
		    if( gr[k].name[ l ] == N_NONE )
			printf( " %s", mvt_names_long[ gr[k].name[ l ] ] );
		    else
			printf( " %s", mvt_names[ gr[k].name[ l ] ] );
		}

		printf( "." );
	    }
	    
	    if( gr[k].finish != FINISH_NICKEL )
		printf( " %s", finish_to_str( gr[k].finish ) );

	    if( gr[k].train != TRAIN_QUICK )
		printf( " %s", train_to_str( gr[k].train ) );
		
	    if( gr[k].comments )
		printf( "  %s", gr[k].comments );

	    printf( "\n" );
	}

	printf( "\n" );
    }
}


	
  
void print_sn_info_as_input( sn_range_type *snr, int num_snr,
				     grade_type *grades)
{
    int		i;
    
    sn_range_type	*s;


    if( snr[ 0 ].comment_lines )
	printf( "%s", snr[ 0 ].comment_lines );

    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];

	printf( "%c %8d %3d ", s->letter, s->sn, s->grade );
	
	if( s->name == N_NONE )
	    printf( " -" );
	else if( s->name != N_DEFER )
	    printf( " %s", mvt_names[ s->name ] );
	else if( s->comments )
	    printf( "\t" );

	if( s->comments )
	    printf( "\t# %s", s->comments );

	printf( "\n" );

	if( s->comment_lines )
	    printf( "%s", s->comment_lines );
    }
}



void print_sn_info_1950MC( sn_range_type *snr, int num_snr,
			     grade_type *grades)
{
    int		i;
    sn_range_type	*s;
    

    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];

	if( s->grade == 0 )
	    continue;

	if( s->cnt == 0 )
	    continue;

	if( s->cnt != 1 )
	    printf( "%s %s %3d\n",
		    sn_to_str( s->letter, s->sn ),
		    sn_to_str( s->letter, s->sn + s->cnt - 1 ),
		    s->grade );
	else
	    printf( "%s %s %3d\n",
		    sn_to_str( s->letter, s->sn ),
		    "........",
		    s->grade );
    }
}



	
void print_sn_info_bufbooth( sn_range_type *snr, int num_snr,
			     grade_type *grades)
{
    int		i;
    sn_range_type	*s;
    

    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];

	if( s->letter == '_' )
	    printf( "%08d %03d\n", s->sn, s->grade );
    }
    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];

	if( s->letter != '_' )
	    printf( "%08d %c %03d\n", s->sn, s->letter, s->grade );
    }
}



typedef struct
{
    /* public */
    char	*buf;		/* malloced buffer		*/
    char	*first_left_pref;	/* what to start the first line with */
    char	*left_pref;	/* what to add at the start of each line */
    char	*seperator;	/* what to add between notes 	*/
    int		max_line_len;	/* start a new line if longer than this	*/

    /* private */
    char	*sol;		/* start of the current line	*/
    char	*eol;		/* end of the current line	*/
    int		buf_size;	/* 				*/
    int	        flp_len;
    int		lp_len;
    int		sep_len;
} note_buf_type;


void init_note( note_buf_type *note )
{
    note->buf = NULL;
}


void free_note( note_buf_type *note )
{
    if( !note->buf )
	return;
    
    free( note->buf );
    note->buf = NULL;

    free( note->first_left_pref );
    free( note->left_pref );
    free( note->seperator );
}


void setup_note( note_buf_type *note,
		char *first_left_pref, char *left_pref, char *seperator,
		int max_line_len )
{
    free_note( note );
    
    note->first_left_pref = strdup( first_left_pref );
    note->left_pref = strdup( left_pref );
    note->seperator = strdup( seperator );
    note->max_line_len = max_line_len;
}


char *note_text( note_buf_type *note )
{
    return note->buf;
}


void add_note( note_buf_type *note, char *fmt, ... )
{
    va_list	ap;
    char	*p;
    int		len, new_len;

    va_start( ap, fmt );
    len = vasprintf( &p, fmt, ap );
    va_end( ap );

    if( !note->buf )
    {
	new_len = len + 1 + 1024;

	note->flp_len = strlen( note->first_left_pref );
	new_len += note->flp_len;

	note->lp_len = strlen( note->left_pref );

	note->sep_len = strlen( note->seperator );
	
	    
	
	note->buf = malloc( new_len );
	if( !note->buf )
	{
	    fprintf( stderr, "%s:%d  could not malloc %d bytes of memory\n",
		     __FILE__, __LINE__, new_len );
	    exit( 1 );
	}

	note->sol = note->buf;
	note->buf_size = new_len;
	
	note->eol = note->buf;
	*note->eol = '\0';

	strcpy( note->eol, note->first_left_pref );
	note->eol += note->flp_len;

    } else {
	
	if( note->buf - note->eol + note->lp_len + note->sep_len + len + 2
	    >= note->buf_size )
	{
	    new_len = note->buf_size + note->lp_len + len + 1 + 1024;

	    note->buf = realloc( note->buf, new_len );
	    if( !note->buf )
	    {
		fprintf( stderr,
			 "%s:%d  could not malloc %d bytes of memory\n",
			 __FILE__, __LINE__, new_len );
		exit( 1 );
	    }
	}
    

	if( note->eol - note->sol + note->sep_len + len >= note->max_line_len )
	{
	    note->sol = note->eol;
	
	    strcat( note->eol, note->left_pref );
	    note->eol += note->lp_len;
	} else {
	    strcat( note->eol, note->seperator );
	    note->eol += note->sep_len;
	}
    }
    
    strcpy( note->eol, p );
    free( p );
    note->eol += len;
}




sn_range_type *search_sn_info( char *in_line, char *out_line,
			       sn_range_type *snr, int num_snr,
			       grade_type *grades, first_runs_type *fr,
			       int verbose, int print_full_sn_info,
			       int do_grade_search )
{
    int		i, j, k;
    int		total_listed;
    int		lines_listed, lines_not_listed;
    int		cnt;

    int		years_produced;
    int		print_years_produced;

    sn_range_type	*s, *s1;
    grade_type		*g, *g1;

    note_buf_type	note;
    
    uchar	letter;

    char	grade_found[ MAX_GRADE ];
    char	print_grade_notes[ MAX_GRADE ];
    char	grade_range_listed[ MAX_GRADE ];
    char	print_class[ MAX_CLASS ];
    int		print_class_notes;

    char	buf[ 81 ];

    char	*p, *p2;
    char	*orig_in_line;
    char	*num_pat;
    char	*var, *end_var, *save_var;

    char	key;
    char	op;
    int		val;
    int		fix_val;
    
    num_pat_type np, out_np;
    int		end_range;

    int		err;


    int		bridge_val;
    int		style_val;
    int		finish_val;
    int		setting_val;
    int		barrel_val;


    char	grade_op;
    int		grade_val;

    char	class_op;
    int		class_val;

    char	size_op;
    int		size_val;

    char	jewel_op;
    int		jewel_val;

    char	model_op;
    int		model_val;

    int		name_val;

    int		reg_val;

    char	tot_cnt_op;
    int		tot_cnt_val;

    char	year_op;
    int		year_val;

    char	*cur_name;
    char	name_buf[ 10 ];

    int		max_lines;


    /*
     * initialize variables
     */
	    
    bridge_val = -1; 
    style_val = STYLE_UNKNOWN;
    finish_val = FINISH_UNKNOWN;
    setting_val = SETTING_UNKNOWN;
    barrel_val = BARREL_UNKNOWN;


    grade_op = '?';
    grade_val = 0;

    class_op = '?';
    class_val = 0;

    size_op = '?';
    size_val = 0;

    jewel_op = '?';
    jewel_val = 0;

    model_op = '?';
    model_val = 0;

    name_val = N_DEFER;

    reg_val = REG_UNKNOWN;

    tot_cnt_op = '?';
    tot_cnt_val = 0;

    year_op = '?';
    year_val = 0;

    max_lines = -1;

    total_listed = 0;
    lines_listed = 0;
    lines_not_listed = 0;
    print_class_notes = FALSE;

    s1 = NULL;
    g1 = NULL;

    memset( grade_found, FALSE, sizeof( grade_found ) );
    memset( print_grade_notes, FALSE, sizeof( print_grade_notes ) );
    memset( grade_range_listed, 0, sizeof( grade_range_listed ) );
    memset( print_class, 0, sizeof( print_class ) );
    
    init_note( &note );


    /*
     * parse the serial number pattern
     */
    if( out_line != NULL )
	*out_line = '\0';
    p = in_line;
    orig_in_line = strdup( in_line );

    while( (num_pat = strsep( &p, " \t\n" )) != NULL && *num_pat == '\0' )
	;

    if( num_pat == NULL || *num_pat == '\0' )
    {
	free( orig_in_line );
	return NULL;
    }

    if( *num_pat == '#' )
    {
	if( out_line != NULL )
	{
	    if( num_pat[ 1 ] != '\0' && p != NULL )
		sprintf( out_line, "%s %s", num_pat + 1, p );
	    else if( num_pat[ 1 ] != '\0' )
		strcpy( out_line, num_pat + 1 );
	    else
		strcpy( out_line, p );
	}
	
	free( orig_in_line );
	return NULL;
    }
	
    letter = num_pat[ 0 ];
    if( isalpha( letter ) && islower( letter ) )
	letter = toupper( letter );
    
    if( cvt_sn_letter( letter, 0 ) >= 0 )
    {
	num_pat++;

	if( *num_pat == '\0' )
	{
	    /* allow an single optional space between the letter and
	     * the reset of the SN */
	    if( p != NULL && *p != '\0' )
	    {
		if( *p == ' ' )
		    num_pat = ++p;
		else
		    num_pat = p;

		while( !isspace( *p ) && *p != '\0' )
		    p++;
		
		if( *p != '\0' )
		    *p++ = '\0';
	    }
	}
    }
    else if( letter != '*' )
	letter = '_';	
	

    err = expand_num_pat( num_pat, &np );
    if( err == ENP_NOT_A_NUM )
    {
	/* assume that we tried to parse a keyword or something */
	letter = '*';
	num_pat = "*";
	
	if( expand_num_pat( num_pat, &np ) != ENP_OK )
	{
	    fprintf( stderr, "%s:%d invalid default serial number pattern.\n",
		     __FILE__, __LINE__ );
	    return NULL;
	}

	p = orig_in_line;
    }
    else if( err == ENP_INVALID_NUM )
    {
	if( letter == '_' )
	    printf( "invalid serial number pattern of %s\n", num_pat );
	else
	    printf( "invalid serial number pattern of %c%s", letter, num_pat );

	return NULL;
    }

    /*
     * remember what we are searching for later
     */
    if( out_line != NULL )
    {
	if( p != NULL )
	{
	    p2 = strchr( p, '\n' );
	    if( p2 != NULL )
		*p2 = '\0';
	}
	else
	    p = "";

	if( letter == '_' )
	    sprintf( out_line, "%s %s",
		     num_pat_to_str( '\0', np ), p );
	else
	    sprintf( out_line, "%s %s",
		     num_pat_to_str( letter, np ), p );
    }
    

    /*
     * delete any comments
     */
    if( p != NULL )
    {
	p2 = strchr( p, '#' );
	if( p2 != NULL )
	    *p2 = '\0';
    }


    /*
     * parse the search options
     */
    while( p != NULL && *p != '\0' )
    {
	while( (var = strsep( &p, " \t\n" )) != NULL && *var == '\0' )
	    ;

	if( var == NULL || *var == '\0' )
	    break;


	if( strcasecmp( "n=?", var ) == 0 )
	    name_val = N_UNKNOWN;

	else if( strcasecmp( "n=-", var ) == 0 )
	    name_val = N_NONE;

	else if( (var[ 0 ] == 'n' || var[ 0 ] == 'N' )
		 && var[ 1 ] == '='
		 && var[ 2 ] != '\0'
	    )
	{
	    name_val = N_NONE;
	    for( i = 1; i < MAX_NAME; i++ )
		if( strcasecmp( &var[ 2 ], mvt_names[i] ) == 0 )
		{
		    name_val = i;
		    break;
		}

	    if( name_val == N_NONE )
	    {
		printf( "invalid movement name of " "\"%s\"\n", &var[ 2 ] );
		p = "error";
		break;
	    }
	}

	else if( (var[ 0 ] == 'r' || var[ 0 ] == 'R' )
		 && var[ 1 ] == '='
		 && var[ 2 ] != '\0'
	    )
	{
	    if( strcasecmp( &var[ 2 ], "f" ) == 0 )
	    {
		reg_val = REG_FREE;
	    }
	    else if( strcasecmp( &var[ 2 ], "e" ) == 0 )
	    {
		reg_val = REG_ELGIN;
	    }
	    else if( strcasecmp( &var[ 2 ], "r" ) == 0 )
	    {
		reg_val = REG_RR_PAT;
	    }
	    else if( strcasecmp( &var[ 2 ], "o" ) == 0 )
	    {
		reg_val = REG_OLD_RR_PAT;
	    }
	    else if( strcasecmp( &var[ 2 ], "b" ) == 0 )
	    {
		reg_val = REG_BALL;
	    }
	    else if( strcasecmp( &var[ 2 ], "h" ) == 0 )
	    {
		reg_val = REG_HULBURD;
	    }
	    else if( strcasecmp( &var[ 2 ], "d" ) == 0 )
	    {
		reg_val = REG_DURABALANCE;
	    } else {
	    
		printf( "invalid regulator specification of \"%s\"\n",
			&var[ 2 ] );
		p = "error";
		break;
	    }
	}
	
	else if( strcasecmp( var, "grade" ) == 0 )
	    do_grade_search = TRUE;

	else if( strcasecmp( var, "sn" ) == 0 )
	    do_grade_search = FALSE;

	else if( strcasecmp( var, "b" ) == 0 )
	    bridge_val = TRUE;

	else if( strcasecmp( var, "nb" ) == 0 )
	    bridge_val = FALSE;
	    
	else if( strcasecmp( var, "hc" ) == 0 )
	    style_val = STYLE_HC;

	else if( strcasecmp( var, "of" ) == 0 )
	    style_val = STYLE_OF;

	else if( strcasecmp( var, "cvt" ) == 0 )
	    style_val = STYLE_CVT;

	else if( strcasecmp( var, "ss" ) == 0 )
	    style_val = STYLE_SS;

	else if( strcasecmp( var, "ns" ) == 0 )
	    style_val = STYLE_NONE;


	else if( strcasecmp( var, "g" ) == 0 )
	    finish_val = FINISH_GILDED;

	else if( strcasecmp( var, "n" ) == 0 )
	    finish_val = FINISH_NICKEL;

	else if( strcasecmp( var, "t" ) == 0 )
	    finish_val = FINISH_TWO_TONE;

	else if( strcasecmp( var, "f" ) == 0 )
	    finish_val = FINISH_FLAT;


	else if( strcasecmp( var, "kw" ) == 0 )
	    setting_val = SETTING_KEY;

	else if( strcasecmp( var, "l" ) == 0 )
	    setting_val = SETTING_LEVER;
	    
	else if( strcasecmp( var, "p" ) == 0 )
	    setting_val = SETTING_PEND;

	else if( strcasecmp( var, "aw" ) == 0 )
	    setting_val = SETTING_AUTO;

	else if( strcasecmp( var, "hs" ) == 0 )
	    setting_val = SETTING_HACK;
	    

	else if( strcasecmp( var, "gb" ) == 0 )
	    barrel_val = BARREL_GOING;

	else if( strcasecmp( var, "mb" ) == 0 )
	    barrel_val = BARREL_MOTOR;

	else if( strcasecmp( var, "jb" ) == 0 )
	    barrel_val = BARREL_JMOTOR;

	
	else
	{
	    key = *var++;

	    if( isupper( key ) )
		key = tolower( key );
	    

	    fix_val = FALSE;

	    if( *var == '=' )
		op = *var++;
	    else if( *var == '<' || *var == '>' )
	    {
		op = *var++;

		if( *var == '=' )
		{
		    fix_val = TRUE;
		    var++;
		}
	    }
	    else if( isalpha( *var ) )
	    {
		printf( "Keyword \"%s\" could not be parsed.\n", var - 1 );
		p = "error";
		break;
	    }
	    else
		op = '?';
	    
	    save_var = var;
	    
	    val = strtol( var, &end_var, 10 );

	    if( fix_val )
	    {
		if( op == '<' )
		    val++;
		else
		    val--;
	    }

	    if( key == 'g' )
	    {
		grade_op = op;
		grade_val = val;
	    }
		
	    else if( key == 'c' )
	    {
		class_op = op;
		class_val = val;
	    }
		
	    else if( key == 's' )
	    {
		size_op = op;

		if( strcmp( end_var, "/0" ) == 0 )
		{
		    end_var += 2;
		    size_val = 1 - val;

		    if( fix_val )
		    {
			if( op == '<' )
			    size_val += 2;
			else
			    size_val -= 2;
		    }
			
		}
		else
		    size_val = val;
	    }
		
	    else if( key == 'j' )
	    {
		jewel_op = op;
		jewel_val = val;
	    }
		
	    else if( key == 'm' )
	    {
		model_op = op;
		model_val = val;
	    }
		
	    else if( key == 't' )
	    {
		tot_cnt_op = op;
		tot_cnt_val = val;
	    }
		
	    else if( key == 'y' )
	    {
		year_op = op;
		year_val = val;
	    }
		
	    else if( key == 'l' )
	    {
		if( op != '=' )
		{
		    printf( "Only the '=' operator is valid for "
			    "max_lines option\n" );
		    p = "error";
		    break;
		}
		
		max_lines = val;
	    }

	    else if( key == 'v' )
	    {
		if( op != '=' )
		{
		    printf( "Only the '=' operator is valid for "
			    "verbose option\n" );
		    p = "error";
		    break;
		}
		
		verbose = val;
	    }
		
	    else 
	    {
		if( op != '?' )
		    printf( "Invalid keyword \"%c\"\n", key );
		else
		    printf( "Keyword \"%s\" could not be parsed.\n", var - 1 );
		p = "error";
		break;
	    }
	    
	    if( op == '?' )
	    {
		if( *var == '\0' )
		    printf( "The %c keyword requires a operator "
			    "(<, <=, =, >= or >).\n",
			    key );
		else
		    printf( "Invalid operator '%c' for keyword, "
			    "must be (<, <=, =, >= or >).\n",
			    *var );
		p = "error";
		break;
	    }
		

	    if( var == end_var )
	    {
		printf( "\"%s\" is not a number\n", var );
		p = "error";
		break;
	    }
	    
	    if( *end_var != '\0' )
	    {
		printf( "Invalid value \"%s\"\n", save_var );
		p = "error";
		break;
	    }
	}
    }

    if( p != NULL && *p != '\0' )
    {
	if( out_line != NULL )
	    *out_line = '\0';
	
	free( orig_in_line );
	return NULL;
    }
	    
    if( do_grade_search && out_line != NULL )
	*out_line = '\0';

    
    /*
     * search the database
     */

    for( i = 1; i <= num_snr; i++ )
    {
	s = &snr[ i ];
	g = &grades[ s->grade ];


	/*
	 * filter out all unwanted sn records
	 */

	if( s->grade == 0 && s->cnt == 0 )
	    continue;

	if( letter != '*' && letter != s->letter )
	    continue;
	
	if( bridge_val != -1 && bridge_val != g->bridge )
	    continue;

	if( style_val != STYLE_UNKNOWN && style_val != g->style )
	    continue;
	    
	if( finish_val != FINISH_UNKNOWN && finish_val != g->finish )
	    continue;
	    
	if( setting_val != SETTING_UNKNOWN && setting_val != g->setting )
	    continue;
	    
	if( barrel_val != BARREL_UNKNOWN && barrel_val != g->barrel )
	    continue;
	    
	if( reg_val != REG_UNKNOWN && reg_val != g->regulator )
	    continue;
	    

	if( (grade_op != '?' && g->grade == GRADE_UNKNOWN)
	    || (grade_op == '<' && g->grade >= grade_val)
	    || (grade_op == '=' && g->grade != grade_val)
	    || (grade_op == '>' && g->grade <= grade_val)
	    )
	    continue;

	if( (class_op != '?' && g->class == CLASS_UNKNOWN)
	    || (class_op == '<' && g->class >= class_val)
	    || (class_op == '=' && g->class != class_val)
	    || (class_op == '>' && g->class <= class_val)
	    )
	    continue;

	if( (size_op != '?' && g->mvt_size == SIZE_UNKNOWN)
	    || (size_op == '<' && g->mvt_size >= size_val)
	    || (size_op == '=' && g->mvt_size != size_val)
	    || (size_op == '>' && g->mvt_size <= size_val)
	    )
	    continue;

	if( (jewel_op != '?' && g->min_jewels == JEWELS_UNKNOWN)
	    || (jewel_op == '<' && g->min_jewels >= jewel_val)
	    || (jewel_op == '='
		&& (jewel_val < g->min_jewels
		    || jewel_val > g->max_jewels
		    )
		)
	    || (jewel_op == '>' && g->max_jewels <= jewel_val)
	    )
	    continue;


	if( (model_op != '?' && g->min_model == MODEL_UNKNOWN)
	    || (model_op == '<' && g->min_model >= model_val)
	    || (model_op == '='
		&& (model_val < g->min_model
		    || model_val > g->max_model
		    )
		)
	    || (model_op == '>' && g->max_model <= model_val)
	    )
	    continue;

	if( name_val != N_DEFER )
	{
	    if( name_val == N_UNKNOWN )
	    {
		if( s->name != N_DEFER )
		    continue;
	    }
	    
	    else if( s->name == N_DEFER )
	    {
		for( j = 0; j < array_elem( g->name ); j++ )
		    if( name_val == g->name[ j ] )
			break;

		if( j == array_elem( g->name ) )
		    continue;
	    }
	    else if( name_val != s->name )
		continue;
	}
	
	if( (tot_cnt_op == '<' && g->total_cnt >= tot_cnt_val)
	    || (tot_cnt_op == '=' && g->total_cnt != tot_cnt_val)
	    || (tot_cnt_op == '>' && g->total_cnt <= tot_cnt_val)
	    )
	    continue;

	if( (year_op != '?' && s->year == 0 )
	    || (year_op == '<' && s->year >= year_val)
	    || (year_op == '=' && s->year != year_val)
	    || (year_op == '>' && s->year <= year_val)
	    )
	    continue;


	end_range = s->sn + s->cnt - 1;
	if( !num_pat_in_range( np, &out_np, s->sn, s->sn )
	    && !num_pat_in_range( np, &out_np, s->sn, end_range )
	    )
	    continue;


	/*
	 * don't list too many lines
	 */
	if( !do_grade_search )
	{

	    if( s->grade != 0 )
		total_listed += s->cnt;

	    if( max_lines >= 0 && lines_listed >= max_lines )
	    {
		lines_not_listed++;
		continue;
	    }
	    else
		lines_listed++;
	}
	

	/*
	 * we have found a record of interested, print out the info
	 */

	grade_found[ s->grade ] = TRUE;

	if( g->flag_notes )
	    print_grade_notes[ s->grade ] = TRUE;


	/* for grade searches, just record that we have seen this grade */
	if( do_grade_search )
	    continue;



	if( note_text( &note ) )
	{
	    printf( "\n" );
	    free_note( &note );
	}

	if( s->name != N_DEFER )
	{
	    if( s->name == N_NONE )
		cur_name = mvt_names_long[ s->name ];
	    else
		cur_name = mvt_names[ s->name ];
	}
	
	else if( g->name[ 1 ] == N_DEFER )
	{
	    if( g->name[ 0 ] == N_DEFER )
		cur_name = "";
	    else
		cur_name = mvt_names[ g->name[ 0 ] ];
	}

	else
	{
	    sprintf( name_buf, "?" );
	    cur_name = name_buf;
	}


	if( print_full_sn_info )
	{
	    printf( "%8s %8s",
		    sn_to_str( s->letter, s->sn ),
		    sn_to_str( s->letter, s->sn + s->cnt - 1 ) );
	} else {
	    
	    if( s->letter == '_' )
		printf( "%14s  ", num_pat_to_str( '\0', out_np ) );
	    else
		printf( "%14s  ", num_pat_to_str( s->letter, out_np ) );
	
	    printf( "%8s", sn_to_str( s->letter, s->sn ) );
	}
	
	printf( " %6d %4s %4d %3d%c %4ss %-7s %5sj",
		s->cnt, cur_name, elgin_prod_year( s ),
		s->grade, g->flag_notes ? '*' : ' ',
		size_to_str( g->mvt_size ),
		grade_to_code_str( g ),
		range_to_str( g->min_jewels, g->max_jewels ) );

	if( g->max_adj != 0 )
	    printf( "  %s",
		    adj_to_str( g->min_adj, g->max_adj ) );

	if( (g->mvt_size <= 0 && g->regulator == REG_FREE)
	    || (g->max_adj == 0 && g->regulator == REG_FREE)
	    || (g->max_adj > 0 && g->max_adj != ADJ_UNKNOWN
		&& g->regulator == REG_ELGIN)
	    )
	    ;	/* don't print standard configurations */
	else
	    printf( " %s", reg_to_instr( g->regulator ) );

	if( g->barrel != BARREL_GOING )
	    printf( " %s", barrel_to_instr( g->barrel ) );


	printf( "\n" );

	setup_note( &note, "    ", ";\n    ", ";  ", 75 );
	
	print_years_produced = FALSE;
	if( g->first_snr == g->last_snr )
	{
	    add_note( &note, "only run of grade %d", s->grade );
	}
	else if( g->first_snr == i )
	{
	    add_note( &note, "first run of grade %d", s->grade );
	    print_years_produced = TRUE;
	}
	else if( g->last_snr == i )
	{
	    add_note( &note,"last run of grade %d", s->grade );

	    if( g->last_produced_in_size )
		add_note( &note,"last %ss produced",
			  size_to_str( g->mvt_size ) );
	    print_years_produced = TRUE;
	}

	if( print_years_produced )
	{
	    years_produced
		= snr[ g->last_snr ].year - snr[ g->first_snr ].year + 1;

	    if( years_produced <= 0 )
		add_note( &note,"  error: years produced is %d",
			  years_produced );
	    else if( years_produced == 1 )
	    {
		add_note( &note,"all produced in one year" );
	    }
	    else if( years_produced <= 2 )
		add_note( &note,"only produced for %d years",
			  years_produced );
	    else if( years_produced >= 15 )
		add_note( &note,"produced for %d years",
			  years_produced );
	}
	

	if( !grade_range_listed[ s->grade ]
	    && (g->total_cnt <= 20000
		|| note_text( &note ) 
		)
	    )
	{
	    grade_range_listed[ s->grade ] = TRUE;
	    add_note( &note,"%d of %d in grade",
		    find_count_in_grade( s->letter, out_np.min_val[ 0 ],
					 g, snr, fr ),
		    g->total_cnt );
	}

	CHECK_COUNT_VAR( size, g->mvt_size, "s", 10000 );

	if( g->bridge )
	{
	    cnt = find_count_in_bridge_in_size( s->letter, out_np.min_val[ 0 ],
						g, snr, fr );
	    if( cnt < 10000 )
	    {
		add_note( &note,"%d bridge model in %ss", cnt,
			size_to_str( g->mvt_size ) );
	    }
	}

	CHECK_COUNT_VAR( style, g->style, " style", 5000 );
	CHECK_COUNT_VAR_IN_SIZE( style, g->style, " style", 5000 );

	if( g->finish != FINISH_GILDED )
	{
	    CHECK_COUNT_VAR( finish, g->finish, " finish", 5000 );
	    CHECK_COUNT_VAR_IN_SIZE( finish, g->finish, " finish", 5000 );
	}

	if( g->min_model != 1 || !g->first_in_size )
	{
	    CHECK_COUNT_VAR_IN_SIZE( model, g->min_model, " model", 5000 );
	}

	CHECK_COUNT_VAR( setting, g->setting, " setting", 10000 );
	CHECK_COUNT_VAR_IN_SIZE( setting, g->setting, " setting", 5000 );

	if( (s->year < 1880 && g->max_jewels >= 15)
	    || (s->year < 1890 && g->max_jewels >= 17)
	    || g->max_jewels >= 19
	    )
	{
	    CHECK_COUNT_VAR( jewel, g->max_jewels, " jewels", 10000 );
	    CHECK_COUNT_VAR_IN_SIZE( jewel, g->max_jewels, " jewels", 5000 );
	}

	if( g->wi_option )
	{
	    cnt = find_count_in_wi_option( s->letter, out_np.min_val[ 0 ],
					g, snr, fr );
	    if( cnt < 5000 )
		add_note( &note,"%d w/WI option", cnt );


	    cnt = find_count_in_wi_option_in_size( s->letter,
						   out_np.min_val[ 0 ],
						   g, snr, fr );
	    if( cnt < 2000 )
		add_note( &note,"%d w/WI option in %ss", cnt,
			size_to_str( g->mvt_size ) );
	}

	if( g->wind_hrs != DEFAULT_WIND_HRS )
	{
	    cnt = find_count_in_wind_hrs( s->letter, out_np.min_val[ 0 ],
					  g, snr, fr );
	    if( cnt < 5000 )
		add_note( &note,"%d of non-standard wind hours", cnt );
	}

	if( g->regulator != REG_FREE )
	{
	    CHECK_COUNT_VAR( regulator, g->regulator, " regulator", 5000 );
	    CHECK_COUNT_VAR_IN_SIZE( regulator, g->regulator, " regulator",
				     2000 );
	}

	if( g->barrel != BARREL_GOING )
	{
	    CHECK_COUNT_VAR( barrel, g->barrel, " barrel", 5000 );
	    CHECK_COUNT_VAR_IN_SIZE( barrel, g->barrel, " barrel", 2000 );
	}


	if( s->comments )
	    add_note( &note,"# %s", s->comments );
	

	if( note_text( &note ) )
	    printf( "%s\n", note_text( &note ) );


	/* must be last */
	if( (print_full_sn_info || verbose >= 3) && s->comment_lines )
	{
	    printf( "%s", s->comment_lines );
	}
	
	s1 = s;
	g1 = g;
    }

    free_note( &note );

    if( lines_listed > 1 && lines_not_listed == 0 )
	printf( "total watches found:  %8d\n", total_listed );
    else if( lines_not_listed != 0 )
	printf( "total watches found:  %8d   (%d ranges not listed)\n",
		total_listed, lines_not_listed );
    else if( lines_listed == 0 && !do_grade_search )
    {
	printf( "No watches found.\n" );
	if( out_line )
	    *out_line = '\0';
    }

    if( lines_listed == 1 && lines_not_listed == 0 && s1 != NULL )
    {
	if( s1->year == 0 && s1->letter == '!' )
	    printf( "*** No serial number ranges for this grade have "
		    "been recorded. ***" );

	else if( s1->year < sn_year[ 0 ].year )
	    printf( "Error:  Invalid date of %d\n", s1->year );
	
	else if( g1->mvt_size <= 10 && s1->year <= 1878 )
	    printf( "*** dial should have an symbol made from 'E W Co' for "
		    "\"Elgin Watch Co\". ***\n" );
	else if( s1->year <= 1874 )
	    printf( "*** dial should be marked \"National Watch Co\" not "
		    "\"Elgin Nat'l Watch Co\". ***\n" );
	else if( s1->sn < 500000 && s1->letter == '_' ) /* I don't trust the prod years data */
	    printf( "*** dial may be marked \"National Watch Co\" or "
		    "\"Elgin Nat'l Watch Co\"  ***\n" );
    }


    for( i = 0, j=0; i < MAX_GRADE; i++ )
    {
	if( !grade_found[ i ] )
	    continue;
	    
	j++;
    }

    if( do_grade_search || verbose >= 2 )
    {
	if( j == 0 )
	{
	    if( do_grade_search )
		printf( "No grades found.\n" );
	} else {

	    if( !do_grade_search )
		printf( "\n\n" );
	    
	    print_grade_info( snr, num_snr, grades, grade_found );
	}
    }


    for( i = 0, j=0; i < MAX_GRADE; i++ )
    {
	if( !grade_found[ i ] )
	    continue;
	    
	j++;

	if( grades[ i ].class != CLASS_UNKNOWN )
	{
	    print_class_notes = TRUE;
	    print_class[ grades[ i ].class ] = TRUE;
	}
    }

    if( j > 0 )
    {
	if( j > 1 )
	    printf( "\nnotes on grades (*):\n" );
	
	for( i = 1; i < MAX_GRADE; i++ )
	{
	    g = &grades[ i ];
			   
	    if( !print_grade_notes[ i ]
		&& (!grade_found[ i ] || verbose < 3 || !g->comment_lines )
		&& (!grade_found[ i ] || verbose < 4 || !g->sn_comments )
		&& (!grade_found[ i ] || verbose < 5 || !g->sn_comment_lines )
		)
		continue;
	    
	    if( j == 1 )
	    {
		printf( "\n" );
		
		sprintf( buf, "(*) notes on grade %d:  ", i );
	    }
	    else
		sprintf( buf, "    grade %3d  ", i );
	    
	    setup_note( &note, buf, "\n               ", "  ", 75 );

	    if( g->dial_size != g->mvt_size )
		add_note( &note, "%ss dial", size_to_str( g->dial_size ) );

	    if( g->name[ 1 ] != N_DEFER )
	    {
		p = buf;
		p += sprintf( p, "Marked:" );

		for( j = 0; j < array_elem( g->name ); j++ )
		{
		    if( g->name[ j ] == N_DEFER )
			break;

		    if( j > 0 )
		    {
			if( j+1 == array_elem( g->name )
			    || g->name[ j+1 ] == N_DEFER
			    )
			    p += sprintf( p, " or" );
			else
			    p += sprintf( p, "," );
		    }
		    
		    if( g->name[ j ] == N_NONE )
			p += sprintf( p, " %s", mvt_names_long[ g->name[ j ] ] );
		    else
			p += sprintf( p, " %s", mvt_names[ g->name[ j ] ] );
		}

		p += sprintf( p, "." );

		add_note( &note, "%s", buf );
	    }

	    if( g->style != STYLE_UNKNOWN
		&& g->style != STYLE_HC
		&& g->style != STYLE_OF
		)
		add_note( &note, "%s", style_to_str( g->style ) );
	    
	    if( g->finish != FINISH_UNKNOWN
		&& g->finish != FINISH_GILDED
		&& g->finish != FINISH_NICKEL
		)
		add_note( &note, "%s", finish_to_str( g->finish ) );
	    
	    if( g->setting == SETTING_AUTO )
		add_note( &note, "Automatic wind" );
	    else if( g->setting == SETTING_HACK )
		add_note( &note, "Hack setting" );

	    if( g->train != TRAIN_QUICK )
		add_note( &note, "%s", train_to_str( g->train ) );

	    if( g->bridge )
		add_note( &note, "%s", bridge_to_str( g->bridge ) );

	    if( g->first_in_size )
		add_note( &note, "first %ss grade", size_to_str( g->mvt_size ) );
	    
	    if( g->last_grade_in_size )
		add_note( &note, "last %ss grade", size_to_str( g->mvt_size ) );
	    
	    if( g->wind_hrs != DEFAULT_WIND_HRS )
	    {
		if( (g->wind_hrs % 24) == 0 )
		    add_note( &note, "%d day wind", g->wind_hrs / 24 );
		else
		    add_note( &note, "%d hour wind", g->wind_hrs );
	    }
	    
	    if( g->first_in_bridge )
		add_note( &note, "first bridge model" );
	    else if( !g->first_in_size && g->first_in_bridge_in_size )
		add_note( &note, "first %ss bridge model",
			size_to_str( g->mvt_size ) );
	    
	    if( g->first_in_style )
		add_note( &note, "first %s style", style_to_str( g->style ) );
	    else if( !g->first_in_size && g->first_in_style_in_size )
		add_note( &note, "first %ss %s style",
			size_to_str( g->mvt_size ),
			style_to_str( g->style ) );
	    
	    if( g->first_in_finish )
		add_note( &note, "first %s finish", finish_to_str( g->finish ) );
	    else if( !g->first_in_size && g->first_in_finish_in_size )
		add_note( &note, "first %ss %s finish",
			size_to_str( g->mvt_size ),
			finish_to_str( g->finish ) );
	    
	    if( !g->first_in_size && g->first_in_model_in_size )
		add_note( &note, "first %ss model %s",
			size_to_str( g->mvt_size ),
			range_to_str( g->min_model, g->max_model ) );
	    
	    if( g->first_in_setting )
		add_note( &note, "first %s setting", setting_to_str( g->setting ) );
	    else if( !g->first_in_size && g->first_in_setting_in_size )
		add_note( &note, "first %ss %s setting",
			size_to_str( g->mvt_size ),
			setting_to_str( g->setting ) );
	    
	    if( g->first_in_jewel )
		add_note( &note, "first %s jewel", jewel_to_str( g->max_jewels ) );
	    else if( !g->first_in_size && g->first_in_jewel_in_size )
		add_note( &note, "first %ss %s jewel",
			size_to_str( g->mvt_size ),
			jewel_to_str( g->max_jewels ) );
	    
	    if( g->first_in_wi_option )
		add_note( &note, "first w/WI option" );
	    else if( !g->first_in_size && g->first_in_wi_option_in_size )
		add_note( &note, "first %ss w/WI option",
			size_to_str( g->mvt_size ) );
	    
	    if( g->first_in_wind_hrs )
		add_note( &note, "first in non standard wind hours" );

	    if( g->first_in_regulator )
		add_note( &note, "first %s regulator",
			regulator_to_str( g->regulator ) );
	    else if( !g->first_in_size && g->first_in_regulator_in_size )
		add_note( &note, "first %ss %s regulator",
			size_to_str( g->mvt_size ),
			regulator_to_str( g->regulator ) );
	    
	    if( g->first_in_barrel )
		add_note( &note, "first %s barrel", barrel_to_str( g->barrel ) );
	    else if( !g->first_in_size && g->first_in_barrel_in_size )
		add_note( &note, "first %ss %s barrel",
			size_to_str( g->mvt_size ),
			barrel_to_str( g->barrel ) );
	    

	    if( g->comments )
		add_note( &note, "# %s", g->comments );

	    if( note_text( &note ) )
		printf( "%s\n", note_text( &note ) );
	    

	    if( verbose >= 3 && g->comment_lines )
	    {
		printf( "\t%s", g->comment_lines );
	    }
	    
	    if( verbose >= 4 )
	    {
		for( k = 1; k <= num_snr; k++ )
		{
		    s = &snr[ k ];
		    
		    if( s->grade != i )
			continue;

		    if( s->comments )
			printf( "\t%8s %s\n", sn_to_str( s->letter, s->sn ),
				s->comments );
			
		    if( verbose >= 5 && s->comment_lines )
			printf( "\t%8s %s\n", sn_to_str( s->letter, s->sn ),
				s->comment_lines );
		}
	    }
	    
	}
    }


    if( print_class_notes && verbose >= 2 )
    {
	printf( "\n\n" );
	print_class_info( grades, print_class );
    }



    /*
     * remind me to update doc!
     */
    if( verbose >= 1
	&& lines_listed == 1
	&& lines_not_listed == 0
	&& g1 != NULL
	&& s1 != NULL
	&& ( g1->dial_size == SIZE_UNKNOWN
	     || g1->mvt_size == SIZE_UNKNOWN
	     || g1->plate == PLATE_UNKNOWN
	     || g1->style == STYLE_UNKNOWN
	     || g1->finish == FINISH_UNKNOWN
	     || g1->min_model == MODEL_UNKNOWN
	     || g1->setting == SETTING_UNKNOWN
	     || g1->train == TRAIN_UNKNOWN
	     || g1->min_jewels == JEWELS_UNKNOWN
	     || g1->min_jewels != g1->max_jewels
	     || g1->min_adj == ADJ_UNKNOWN
	     || g1->min_adj != g1->max_adj
	     || g1->regulator == REG_UNKNOWN
	     || g1->barrel == BARREL_UNKNOWN
	     || (g1->name[ 1 ] != N_DEFER && s1->name == N_DEFER)
	    )
	)
    {
	printf( "**** UNKNOWN DATA LISTED!   "
		"UPDATE DATABASE IF POSSIBLE ****\n" );
    }

    free( orig_in_line );

    if( lines_listed == 1 && lines_not_listed == 0 )
	return s1;
    else
    {
	if( out_line != NULL )
	    *out_line = '\0';
	
	return NULL;
    }
}



void parse_opt( int argc, char *argv[],
		char **prompt, char **log_file_name,
		FILE **sn_file, FILE **gr_file,
		int *verbose, int *do_verify, int *do_print_sn,
		int *do_print_grade, int *do_search, int *do_grade_search
    )
{
    int c;
    

    *sn_file = NULL;
    *gr_file = NULL;
    *verbose = 0;
    *do_verify = FALSE;
    *do_print_sn = FALSE;
    *do_print_grade = FALSE;
    *do_search = FALSE;
    *do_grade_search = FALSE;
    *prompt = strdup( "\nEnter SN: " );
    *log_file_name = NULL;


    while( 1 )
    {
	c = getopt( argc, argv, "vsSngVN:G:l:p:h" );

	if( c == EOF )
            break;

	switch( c )
	{
	case 'v':
	    ++*verbose;
	    break;
	    
	case 's':
	    *do_search = TRUE;
	    break;

	case 'S':
	    *do_grade_search = TRUE;
	    break;

	case 'n':
	    *do_print_sn = TRUE;
	    break;

	case 'g':
	    *do_print_grade = TRUE;
	    break;

	case 'V':
	    *do_verify = TRUE;
	    break;

	case 'N':
	    *sn_file = fopen( optarg, "r" );
	    if( *sn_file == NULL )
	    {
		perror( "Could not open serial number database" );
		exit( 1 );
	    }
	    break;

	case 'G':
	    *gr_file = fopen( optarg, "r" );
	    if( *gr_file == NULL )
	    {
		perror( "Could not open grade database" );
		exit( 1 );
	    }
	    break;

	case 'l':
	    *log_file_name = optarg;
	    break;

	case 'p':
	    *prompt = optarg;
	    if( **prompt == '\0' )
		*prompt = NULL;
	    
	    break;

	case '?':
	case 'h':
	    fprintf( stderr,
		     "pw_elgin v%s\n\n", versionInfo );

	    fprintf( stderr,
		     "Copyright (C) 1999, 2000 Wayne Schlitt (wayne@midwestcs.com)\n"
		     "This program is licensed under the GPL.  A copy of the\n"
		     "GPL should have been supplied with the program, but if it\n"
		     "wasn't, you can view it at www.fsf.org\n"
		     "\n"
		     "The most current version of pw_elgin can be obtained from:\n"
		     "    http://www.midwestcs.com/elgin/pw_elgin/\n"
		     "\n"
		     "\n"
		     "Usage:  pw_elgin [search options]\n"
		     "\n"
		     "    -s          Enter \"search\" mode.\n"
		     "    -S          Enter grade \"search\" "
		     "mode.\n"
		     "    -v          increase verboseness\n"
		     "    -n          Print serial number data\n"
		     "    -g          Print grade data\n"
		     "    -V          Verify serial number and "
		     "grade databases\n"
		     "    -N <file>   Use <file> for serial number "
		     "database\n"
		     "    -G <file>   Use <file> for grade "
		     "database\n"
		     "    -p <prompt> Prompt string to use in "
		     "search mode\n"
		     "    -l <file>   Log searches in <file>\n"
		     "\n"
		     "If no search options are given, pw_elgin will enter the search mode\n"
		     "\n"
		     "\n"
		     "Search Options:\n"
		     "\n"
		     "serial number\tThe serial number, if given, must be the first option\n"
		     "\t\ton the line.  Both '*' and '?' can be used as wild cards\n"
		     "\n"
		     "n=<name>\tlimit search to watches with a given name.  All \"names\"\n"
		     "\t\tare two or three letter abbreviations, or the special names\n"
		     "\t\tof '-' or '?'.\n"
		     "r=<reg type>\tlimit search to watches with a given regulator type.\n"
		     "\n"
		     "grade\t\tSearch the grade database instead of the serial number\n"
		     "\t\tdatabase.\n"
		     "sn\t\t\tSearch the serial number database instead of the grade\n"
		     "\t\tdatabase.\n"
		     "\n"
		     "b\t\tlimit search to watches that are bridge movements.\n"
		     "nb\t\tlimit search to watches that are not bridge movements.\n"
		     "\n"
		     "hc\t\tlimit search to watches with hunter case movements.\n"
		     "of\t\tlimit search to watches with open face movements.\n"
		     "cvt\t\tlimit search to watches with convertable movements.\n"
		     "ss\t\tlimit search to watches with a sweep second hand.\n"
		     "ns\t\tlimit search to watches with no second hand.\n"
		     "\n"
		     "g\t\tlimit search to watches with gilded finishes.\n"
		     "n\t\tlimit search to watches with nickel damaskeening.\n"
		     "t\t\tlimit search to watches with two-tone damaskeening.\n"
		     "f\t\tlimit search to watches with flat (matte) nickel finishes.\n"
		     "\n"
		     "kw\t\tlimit search to watches key wind/key set movements.\n"
		     "l\t\tlimit search to watches lever set movements.\n"
		     "p\t\tlimit search to watches pendant set movements.\n"
		     "aw\t\tlimit search to watches with auto-wind movements.\n"
		     "hs\t\tlimit search to watches with hack setting movements.\n"
		     "\n"
		     "gb\t\tlimit search to watches with going barrels.\n"
		     "mb\t\tlimit search to watches with motor barrels.\n"
		     "jb\t\tlimit search to watches with jewelled motor barrels.\n"
		     "\n"
		     "In the following options, the '=' can be replaced with '<', '<='\n"
		     "'>', or '>='.\n"
		     "\n"
		     "g=<grade>\tlimit search to watches of a given grade number.\n"
		     "c=<class>\tlimit search to watches in a given class.\n"
		     "s=<size>\tlimit search to watches of a given size.\n"
		     "j=<jewels>\tlimit search to watches with a given number of jewels.\n"
		     "m=<model>\tlimit search to a given model within a size.\n"
		     "t=<count>\tlimit search to watches with a total production count.\n"
		     "y=<year>\tlimit search to watches in a given year.\n"
		     "l=<lines>\tlimit output to a given number of lines.\n"
		     "v=<verbose>\tcontrol how verbose the output should be.\n"
		     "\n"
		     "\n"
		     "Examples:\n"
		     "\n"
		     "pw_elgin 12345"
		     "\n"
		     "This will tell you about serial number 12345.\n"
		     "\n"
		     "\n"
		     "pw_elgin H71????\n"
		     "\n"
		     "This will tell you about all watches in the ranges that begin\n"
		     "with the serial number \"H71\".\n"
		     "\n"
		     "\n"
		     "pw_elgin \"12???321 s<=12 j>21\"\n"
		     "\n"
		     "This will tell you about all watches that have serial numbers\n"
		     "starting with '12', followed by any three numbers and ending\n"
		     "in '321'.  The watches must also be 12s or smaller and have more\n"
		     "than 21 jewels.  This happens to show two different watches, one\n"
		     "a grade 194 and one a grade 190\n"
		     "\n"
		     "It is important to put quotes around the search options in this\n"
		     "case or the shell will interpret the \"j>21\" as redirecting\n"
		     "output to the file \"21\"\n"
		     "\n"
		     "\n"
		     "pw_elgin \"131????? n=bwr j=19 s=18\"\n"
		     "\n"
		     "This will end up finding a single run of grade 240 BWRaymonds.\n"
		     

		);

	    exit( 1 );
	    break;

	default:
	    fprintf( stderr, "?? getopt returned character code 0%o ??\n", c );
	}
    }

    if( *sn_file == NULL )
    {
	*sn_file = fopen( LIBDIR "elgin_sn_data", "r" );
	if( *sn_file == NULL )
	{
	    perror( "Could not open serial number database " );
	    exit( 1 );
	}
    }

    if( *gr_file == NULL )
    {
	*gr_file = fopen( LIBDIR "elgin_grade_data", "r" );
	if( *gr_file == NULL )
	{
	    perror( "Could not open grade database " );
	    exit( 1 );
	}
    }

    if( !*do_search && !*do_print_sn && !*do_print_grade && !*do_verify
	&& !*do_grade_search )
	*do_search = TRUE;
}


int valid_ebay_number( char *ebay_number )
{
    char	*p;

    /* verify if this is a valid ebay number so that we don't confuse
     * it grade numbers or serial numbers.
     *
     * Unfortunately, yahoo's numbers are still (as of 12/06/99) too
     * small and can be confused with serial numbers.  I haven't quite
     * figured out amazon's numbering scheme yet, and I don't know how
     * to use it.  (Sorry, I'm mostly searching ebay right now.)
     */

    for( p = ebay_number; *p != '\0'; p++ )
	if( !isdigit( *p ) )
	    return FALSE;

    return ( p - ebay_number >= 9 );
}



int search_log_file( char *log_file_name, char *out_line )
{
    FILE	*log_file_scan;

    char	scan_str[ 1024 ];
    int		scan_str_len;
    
    char	in_line[ 1024 ];
    char	*p;

    int		first_match;
    
    int		num_aka = 0;


    if( log_file_name == NULL )
	return num_aka;

    if( out_line == NULL || *out_line == '\0' )
	return num_aka;
    
    /* extract the search pattern out of the line */
    strcpy( scan_str, out_line );

    p = strchr( scan_str, '#' );
    if( p != NULL )
	*p = '\0';

    p = scan_str + strlen( scan_str );
    for( ;; )
    {
	if( p == scan_str )
	    return num_aka;

	p--;

	if( isspace( *p ) )
	    *p = '\0';
	else
	    break;
    }

    scan_str_len = strlen( scan_str );

    
    log_file_scan = fopen( log_file_name, "r" );
    if( log_file_scan == NULL )
    {
	perror( "Note: Could not open log file for history scanning" );
	return num_aka;
    }

    first_match = TRUE;
    while( 1 )
    {
	if( fgets( in_line, sizeof( in_line ), log_file_scan ) == NULL )
	    break;

	/* skip over ebay number */
	for( p = in_line; isdigit( *p ); p++ )
	    ;

	/* find start of search pattern */
	while( isspace( *p ) )
	    p++;

	if( *p == '#' )
	    continue;

	/* see if the pattern matches */
	if( strncmp( scan_str, p, scan_str_len ) != 0 )
	    continue;
	
	/* make sure that what's left of the line is just a comment */
	p += scan_str_len;

	while( *p == ' ' )
	    p++;

	/* skip over ebay time/price info */
	if( *p == '\t' )
	{
	    p++;
	    
	    while( *p != '\t' && *p != '#' && *p != '\0' )
		p++;
	}
	    
	while( isspace( *p ) )
	    p++;

	if( *p != '#' && *p != '\0' )
	    continue;

	if( first_match )
	{
	    first_match = FALSE;
	    printf( "\n" );
	}

	printf( "aka:  %s", in_line );
	num_aka++;
    }
	

    fclose( log_file_scan );
    return num_aka;
}



void search_mode( sn_range_type *snr, int num_snr, grade_type *grades,
		  first_runs_type *first_runs,
		  int verbose, char *prompt, char *log_file_name,
		  int do_grade_search
		  )
{
    FILE		*log_file_out;

    char		in_line[ 1024 ], out_line[ 1024 ];
    char		*p, *ebay_number;

    int			i;
    sn_range_type	*s;
    grade_type		*g;

    int			print_years_produced;
    int			years_produced;

    int			num_aka;
    


#ifdef HAVE_READLINE
    /* by default TAB does filename completion, which makes no sense here.
     * just insert the TAB instead */
    rl_bind_key ('\t', rl_insert);
#endif

    
    while( TRUE )
    {
#ifdef HAVE_READLINE
	if( prompt != NULL )
	{
	    p = readline( prompt );

	    if( p == NULL )
		break;

	    if( *p != '\0' )
		add_history( p );

	    if( strlen( p ) >= sizeof( in_line ) )
		p[ sizeof( in_line ) - 1 ] = '\0';
	    strcpy( in_line, p );
	    free( p );
	} else {
	    if( fgets( in_line, sizeof( in_line ), stdin ) == NULL )
		break;
	}
#else
	if( prompt != NULL )
	    fputs( prompt, stdout );
		
	if( fgets( in_line, sizeof( in_line ), stdin ) == NULL )
	    break;
#endif
    
	s = search_sn_info( in_line, out_line,
			    snr, num_snr, grades, first_runs,
			    verbose, FALSE, do_grade_search );

	num_aka = search_log_file( log_file_name, out_line );

	if( s != NULL && log_file_name != NULL )
	{
#ifdef HAVE_READLINE
	    if( prompt != NULL )
	    {
		p = readline( "      Comments: " );

		if( p == NULL )
		    break;

		if( *p != '\0' )
		    add_history( p );

		if( strlen( p ) >= sizeof( in_line ) )
		    p[ sizeof( in_line ) - 1 ] = '\0';
		strcpy( in_line, p );
		free( p );
	    } else {
		fputs( "      Comments: ", stdout );
		if( fgets( in_line, sizeof( in_line ), stdin ) == NULL )
		    break;
	    }
#else
	    fputs( "      Comments: ", stdout );
	    if( fgets( in_line, sizeof( in_line ), stdin ) == NULL )
		break;
#endif
    
	    p = strchr( in_line, '\n' );
	    if( p != NULL )
		*p = '\0';

	    p = in_line;
	    while( (ebay_number = strsep( &p, " \t\n" )) != NULL
		   && *ebay_number == '\0' )
		;

	    if( ebay_number == NULL
		|| (strcasecmp( ebay_number, "void" ) != 0
		    && strcasecmp( ebay_number, "skip" ) != 0
		    && strcasecmp( ebay_number, "nolog" ) != 0
		    )
		)
	    {
		log_file_out = fopen( log_file_name, "a" );
		if( log_file_out == NULL )
		{
		    perror( "Could not open log file" );
		    exit( 1 );
		}

		g = &grades[ s->grade ];
		    
		if( ebay_number != NULL && valid_ebay_number( ebay_number ) )
		    fprintf( log_file_out, "%-10s %s\t# ",
			     ebay_number, out_line );
		else
		{
		    fprintf( log_file_out, "           %s\t# ",
			     out_line );

		    printf( "\nWARNING:  no valid ebay number found.\n" );

		    /* undo the strsep() */
		    if( p == NULL )
			p = ebay_number;
		    
		    else if( ebay_number != NULL )
		    {
			p[ -1 ] = ' ';
			p = ebay_number;
		    }
		}
			    

		if( s->letter == '_' && s->sn < 1000000 )
		    fprintf( log_file_out, "old: %d ", s->year );
		
		if( s->name != N_DEFER )
		{
		    if( s->name == N_NONE )
			fprintf( log_file_out, "%s ",
				 mvt_names_long[ s->name ] );
		    else
			fprintf( log_file_out, "%s ",
				 mvt_names[ s->name ] );
		}
	
		else 
		{
		    for( i = 0; i < MAX_GRADE_NAME; i++ )
		    {
			if( g->name[ i ] == N_DEFER )
			    break;

			if( i != 0 )
			    fprintf( log_file_out, "/" );
			    
			if( g->name[ i ] == N_NONE )
			    fprintf( log_file_out, "%s",
				     mvt_names_long[ g->name[ i ] ] );
			else
			    fprintf( log_file_out, "%s",
				     mvt_names[ g->name[ i ] ] );
		    }

		    if( i > 0 )
			fprintf( log_file_out, " " );
		}

		if( g->bridge )
		{
		    if( g->mvt_size == 16 )
			fprintf( log_file_out, "3F " );
		    else
			fprintf( log_file_out, "Bridge " );
		}

		fprintf( log_file_out, "%ss %d ",
			 size_to_str( g->mvt_size ), g->grade );

		if( g->setting != SETTING_LEVER
		    && g->setting != SETTING_PEND )
		    fprintf( log_file_out, "%s ",
			     setting_to_str( g->setting ) );

		else if( (g->mvt_size > 6 && g->style != STYLE_OF)
		    || (g->mvt_size <= 6 && g->style != STYLE_HC) )
		    fprintf( log_file_out, "%s ", style_to_str( g->style ) );

		if( g->total_cnt <= 10000 )
		{
		    if( g->total_cnt % 1000 == 0 )
			fprintf( log_file_out, "%dk made ",
				 g->total_cnt / 1000 );
		    else
			fprintf( log_file_out, "%d made ", g->total_cnt );
		}

		if( g->max_jewels >= 19 )
		    fprintf( log_file_out, "%sj ",
			     range_to_str( g->min_jewels, g->max_jewels ) );

		if( g->barrel == BARREL_JMOTOR )
		    fprintf( log_file_out, "%s ", barrel_to_str( g->barrel ) );


		if( g->wind_hrs != DEFAULT_WIND_HRS )
		{
		    if( (g->wind_hrs % 24) == 0 )
			fprintf( log_file_out, "%d-day ", g->wind_hrs / 24 );
		    else
			fprintf( log_file_out, "%d-hr ", g->wind_hrs );
		}
	    
		print_years_produced = FALSE;
		if( g->first_snr == g->last_snr )
		    fprintf( log_file_out, "only run " );

		else if( g->first_snr == s - snr )
		{
		    fprintf( log_file_out, "first run" );
		    print_years_produced = TRUE;
		}
		else if( g->last_snr == s - snr )
		{
		    fprintf( log_file_out, "last run" );

		    if( g->last_produced_in_size )
			fprintf( log_file_out, ", last %ss produced", size_to_str( g->mvt_size ) );
		    print_years_produced = TRUE;
		}

		if( print_years_produced )
		{
		    years_produced
			= snr[ g->last_snr ].year - snr[ g->first_snr ].year + 1;

		    if( years_produced <= 0 )
			fprintf( log_file_out, " error: years produced is %d ", years_produced );
		    if( years_produced == 1 )
		    {
			fprintf( log_file_out, ", all produced in one year " );
		    }
		    else if( years_produced <= 2 )
			fprintf( log_file_out, ", only produced for %d years ", years_produced );
		    else if( years_produced >= 15 )
			fprintf( log_file_out, ", produced for %d years ", years_produced );
		    else
			fprintf( log_file_out, " " );
		}

		if( num_aka )
		    fprintf( log_file_out, "aka=%d ", num_aka );

		if( p != NULL && *p != '\0' )
		{
		    while( isspace( *p ) )
			p++;
	    		    
		    fprintf( log_file_out, "  %s\n", p );
		}
		else
		    fprintf( log_file_out, "\n" );

		fclose( log_file_out );
		log_file_out = NULL;
	    }
	}

	else if( *out_line != '\0' && log_file_name != NULL )
	{
	    log_file_out = fopen( log_file_name, "a" );
	    if( log_file_out == NULL )
	    {
		perror( "Could not open log file" );
		exit( 1 );
	    }

	    p = out_line;
	    while( isspace( *p ) )
		p++;

	    if( *p == '#' )
	    {
		fprintf( log_file_out, "%s\n", p );
	    } else {
	    
		while( (ebay_number = strsep( &p, " \t\n" )) != NULL
		       && *ebay_number == '\0' )
		    ;
		    
		if( p != NULL )
		{
		    while( isspace( *p ) )
			p++;
	    
		    if( valid_ebay_number( ebay_number ) )
			fprintf( log_file_out, "%-10s \t\t# %s\n",
				 ebay_number, p );
		    else
		    {
			fprintf( log_file_out, "%s %s\n", ebay_number, p );
			printf( "Note:  no valid ebay number found.\n" );
		    }
		} else {
		    fprintf( log_file_out, "%s\n", ebay_number );
		}
	    }
			
	    fclose( log_file_out );
	    log_file_out = NULL;
	}
		
    }
}



int main( int argc, char *argv[] )
{
    sn_range_type	snr[ MAX_SN_RANGES ];
    int			num_snr;
    
    grade_type		grades[ MAX_GRADE ];

    first_runs_type	first_runs;

    FILE		*sn_file, *gr_file;
    char		*prompt;
    char		*log_file_name;
    int			verbose;
    int			do_verify, do_print_sn, do_print_grade;
    int			do_search, do_grade_search;

    char		in_line[ 1024 ];
    int			line_len;



    if( array_elem( mvt_names ) != MAX_NAME )
	fprintf( stderr, "%s:%d  Movement name list not filled out correctly! "
		 " sizeof( mvt_names ) = %d\n",
		 __FILE__, __LINE__, sizeof( mvt_names ) );


    memset( snr, 0, sizeof( snr ) );
    memset( grades, 0, sizeof( grades ) );


    parse_opt( argc, argv, &prompt, &log_file_name,
	       &sn_file, &gr_file,
	       &verbose, &do_verify, &do_print_sn, &do_print_grade,
	       &do_search, &do_grade_search );

    read_grade_info( gr_file, grades, do_verify );

    read_sn_info( sn_file, snr, &num_snr, grades );

    analyze_info( snr, num_snr, grades, &first_runs );

    if( do_verify )
    {
	printf( "\n" );		/* verification is also done while reading */
	verify_grade_info( grades );
	printf( "\n" );
	verify_sn_info( snr, num_snr, grades );
    }

    if( do_print_sn )
    {
#if 1
	search_sn_info( "*", NULL, snr, num_snr, grades, &first_runs,
			verbose, TRUE, FALSE );

	if( verbose >= 2 )
	{
	    printf( "\n\n### START OF DATA ###\n" );
	    print_sn_info_as_input( snr, num_snr, grades );
	    printf( "### END OF DATA ###\n" );
	}
#elif 0
	print_sn_info_1950MC( snr, num_snr, grades );
#else
	print_sn_info_bufbooth( snr, num_snr, grades );
#endif
    }
    
    if( do_print_grade )
    {
	search_sn_info( "*", NULL, snr, num_snr, grades, &first_runs,
			verbose, FALSE, TRUE );

	if( verbose >= 3 )
	{
	    printf( "\n\n" );
	    print_grade_stats_info( snr, num_snr, grades );
	}

	if( verbose >= 2 )
	{
	    printf( "\n\n### START OF DATA ###\n" );
	    print_grade_info_as_input( snr, num_snr, grades );
	    printf( "### END OF DATA ###\n" );
	}
    }

    if( do_search || do_grade_search )
    {
	if( optind < argc )
	{
	    line_len = 0;
	    in_line[ 0 ] = '\0';
	    
	    while( optind < argc )
	    {
		line_len += strlen( argv[ optind ] );

		if( line_len >= sizeof( in_line ) )
		{
		    fprintf( stderr, "Search options are too long\n" );
		    exit( 1 );
		}

		strcat( in_line, " " );
		strcat( in_line, argv[ optind ] );
		optind++;
	    }

	    search_sn_info( in_line, NULL,
			    snr, num_snr, grades, &first_runs,
			    verbose, FALSE, do_grade_search );
	} else {
	    search_mode( snr, num_snr, grades, &first_runs,
			 verbose, prompt, log_file_name, do_grade_search );
	}
    }
    

    return 0;
}

    
	    
