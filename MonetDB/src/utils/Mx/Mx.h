/* Debug control
 */

#include <config.h>

#ifdef WIN32
# include <io.h>
# include <direct.h>
# define mkdir(path,op) _mkdir(path)
#endif

#ifndef DIR_SEP
#define DIR_SEP '/'
#endif

#define	DB_DEF	0x10
#define	DB_TEXT	0x20
#define	DB_CODE	0x40
#define	DB_TOK	0x41

#define	assert(e, m)	if( !(e) ) Fatal(m, "Assertion failed"); else

extern	int	db_flag;
extern	int	archived;

/* MX control indicators
 */
#define MARK		'@' 


typedef enum { 
    Nop =0,
    Index0,Index1,Index2,Index3,Index4,Index5,Index6,Index7,Index8,Index9,
    Bfile, Efile, Ofile, Mxmacro, Ifdef, Ifndef, Endif,
    Title, Author, Version, Date, InHide, OutHide, 
    Module, Section, Subsection, Paragraph, Qtex, Qcode, Continue,
    Pspec, Pimpl, Cdef, Csrc, CCsrc, ODLspec, SQL,
    OQLspec, Cyacc, Clex, Prolog, Monet, MILcode, Qnap, HTML, Java,
    Tcl, ProC, Shell, fGrammar, Macro, XML, DTD, XSL, Config, Swig,
    CCyacc, CClex } CmdCode;


typedef struct {
    char *cmd;
    CmdCode code;
    char *ext;
} Directive;

extern char *outputdir;
extern Directive str2dir[];
 
/* CtlDir (can be any where)
 */

#define CTLSTR		"$#%!`[:{}|()"
#define MACROSTR        "[:?!"
#define T_NONE		'\0'
#define T_TEX		'$'
#define T_CODE		'#'
#define T_ITALIC	'%'
#define T_BOLD		'!'
#define T_INDEX		'`'
#define T_SGML		'['
#define T_REFERENCE	':'
#define T_POSCOND	'?'
#define T_NEGCOND	'!'


#define HIDESTR		"{|}()"
#define T_BEGHIDE	'{'
#define T_HIDETEXT	'|'
#define T_ENDHIDE	'}'
#define T_BEGARCHIVE	'('
#define T_ENDARCHIVE	')'

/*
extern	char *		strchr();
*/
#define	CmdDir(c)	(strchr(CMDSTR, c) != 0)
#define	CtlDir(c)	(strchr(CTLSTR, c) != 0)
#define CallDir(c)      (strchr(MACROSTR, c)!= 0)
#define	HideDir(c)	(strchr(HIDESTR, c) != 0)

/* 
 * MX layout types
 */

#define M_CODE	0x00ff

#define M_TEXT	0xf000
#define	M_DRAFT	0x1000
#define	M_BOOK	0x2000

#define M_WWW 	0x00f0
#define	M_MS	0x0f00
#define	M_TEX	0xf000

extern	int	mode;
/* 
 * Environments.
 */

#define	E_NONE	0x10
#define	E_TEXT	0x20
#define	E_CODE	0x40
#define	E_DEF	0x41
#define	E_SRC	0x42
#define	E_CMD	0x80


/* Mx Currents
 */
extern	int	mx_err;
extern	char *	mx_file;
extern	int	mx_line;
extern	int	mx_chp;
extern	int	mx_mod;
extern	int	mx_sec;
extern	int	mx_out;

/* Mx Options
 */
extern	int	opt_column;
extern	int	opt_hide;
extern	char	*opt_code;	/* extract code of interest only*/
extern	int	textmode;	/* either T_TEX or T_MS */
extern  int	noline;
extern  int	notouch;

#define	NO_HIDE	-1

/* MX Files
 */


extern	FILE *	ofile;
extern	FILE *	ofile_body;
extern	FILE *	ofile_index;
extern	FILE *	ifile;
#define	M_FILES	128
struct file_s {
	char *	f_name;
	char *  f_tmp;
	char *  f_str;
	int	f_mode;
};
typedef struct file_s File;

extern char filename[200];	/* assume that no errors will occur */
extern char filename_index[200];	
extern char filename_body[200];	

/* MX Def
 */
#define M_DEFS	16384
#define	M_BLK	655360
#define	M_CMD	2048
#define	M_STR	1024

typedef
 struct{
	CmdCode	d_dir;
	char *	d_cmd;
	char *	d_blk;
	int	d_mod;
	int	d_sec;
	char *	d_cms;
	char *	d_file;
	int	d_line;
} Def;

extern	Def *	defs;
extern	int	ndef;

/* MX Tok
 */
typedef struct{
	char	t_dir;
	char *	t_str;
	char	t_ext;
	char *	t_nxt;
	char	t_chr;
} Tok;

/* MX Index
 */
#define  M_ITABLE	 12
#define  M_IENTRY	1000

#define ICONT		 0
#define IFRAG		10
#define IMACRO		11

typedef struct {
	char *	ie_name;
	int	ie_sec;
	int	ie_mod;
} Ientry;

typedef struct {
	char *	it_name;
	int	it_nentry;
	Ientry *it_entrys;
} Itable;

extern	Itable *itable;
extern	int itable_done;

#define link_color "#6666ff"
#define vlnk_color "#871F78"
#define text_color "#000000"
#define code_color "#505050"
#define line_color "#A0A0A0"

/* Footnotes
 * They contain however tex commands ??
 */
#define PNOTE "\
The work reported in this document was conducted as part of the PRISMA project,\n\
a joint effort with Philips Research Eindhoven,\n\
partially supported by the Dutch Stimuleringsprojectteam Informaticaonderzoek (SPIN).\n"
#define MNOTE "\
This document was produced with the {\\bf mx} programmer's tool.\n"
