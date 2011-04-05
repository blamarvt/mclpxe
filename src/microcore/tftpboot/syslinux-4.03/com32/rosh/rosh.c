/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2008-2010 Gene Cumm - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Boston MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * rosh.c
 *
 * Read-Only shell; Simple shell system designed for SYSLINUX-derivitives.
 * Provides minimal commands utilizing the console via stdout/stderr as the
 * sole output devices.  Designed to compile for Linux for testing/debugging.
 */

/*
 * ToDos:
 * rosh_ls(): sorted; then multiple columns
 */

/*#define DO_DEBUG 1
//*/
/* Uncomment the above line for debugging output; Comment to remove */
/*#define DO_DEBUG2 1
//*/
/* Uncomment the above line for super-debugging output; Must have regular
 * debugging enabled; Comment to remove.
 */
#include "rosh.h"
#include "../../version.h"

#define APP_LONGNAME	"Read-Only Shell"
#define APP_NAME	"rosh"
#define APP_AUTHOR	"Gene Cumm"
#define APP_YEAR	"2010"
#define APP_VER		"beta-b068"

void rosh_version(int vtype)
{
    char env[256];
    env[0] = 0;
    printf("%s v %s; (c) %s %s.\n\tFrom Syslinux %s, %s\n", APP_LONGNAME, APP_VER, APP_YEAR, APP_AUTHOR, VERSION_STR, DATE);
    switch (vtype) {
    case 1:
	rosh_get_env_ver(env, 256);
	printf("\tRunning on %s\n", env);
    }
}

void print_beta(void)
{
    puts(rosh_beta_str);
    ROSH_DEBUG("DO_DEBUG active\n");
    ROSH_DEBUG2("DO_DEBUG2 active\n");
}

/* Search a string for first non-space (' ') character, starting at ipos
 *	istr	input string to parse
 *	ipos	input position to start at
 */
int rosh_search_nonsp(const char *istr, const int ipos)
{
    int curpos;
    char c;

    curpos = ipos;
    c = istr[curpos];
    while (c && isspace(c))
	c = istr[++curpos];
    return curpos;
}

/* Search a string for space (' '), returning the position of the next space
 * or the '\0' at end of string
 *	istr	input string to parse
 *	ipos	input position to start at
 */
int rosh_search_sp(const char *istr, const int ipos)
{
    int curpos;
    char c;

    curpos = ipos;
    c = istr[curpos];
    while (c && !(isspace(c)))
	c = istr[++curpos];
    return curpos;
}

/* Parse a string for the first non-space string, returning the end position
 * from src
 *	dest	string to contain the first non-space string
 *	src	string to parse
 *	ipos	Position to start in src
 */
int rosh_parse_sp_1(char *dest, const char *src, const int ipos)
{
    int bpos, epos;		/* beginning and ending position of source string
				   to copy to destination string */

    bpos = 0;
    epos = 0;
/* //HERE-error condition checking */
    bpos = rosh_search_nonsp(src, ipos);
    epos = rosh_search_sp(src, bpos);
    if (epos > bpos) {
	memcpy(dest, src + bpos, epos - bpos);
	if (dest[epos - bpos] != 0)
	    dest[epos - bpos] = 0;
    } else {
	epos = strlen(src);
	dest[0] = 0;
    }
    return epos;
}

/* Display help
 *	type	Help type
 *	cmdstr	Command string
 */
void rosh_help(int type, const char *cmdstr)
{
    const char *istr;
    istr = cmdstr;
    switch (type) {
    case 2:
	istr += rosh_search_nonsp(cmdstr, rosh_search_sp(cmdstr, 0));
	if ((cmdstr == NULL) || (strcmp(istr, "") == 0)) {
	    rosh_version(0);
	    puts(rosh_help_str2);
	} else {
	    switch (istr[0]) {
	    case 'l':
		puts(rosh_help_ls_str);
		break;
	    default:
		printf(rosh_help_str_adv, istr);
	    }
	}
	break;
    case 1:
    default:
	rosh_version(0);
	puts(rosh_help_str1);
    }
}

/* Handle most/all errors
 *	ierrno	Input Error number
 *	cmdstr	Command being executed to cause error
 *	filestr	File/parameter causing error
 */
void rosh_error(const int ierrno, const char *cmdstr, const char *filestr)
{
    printf("--ERROR: %s '%s': ", cmdstr, filestr);
    switch (ierrno) {
    case 0:
	puts("NO ERROR");
	break;
    case ENOENT:
	puts("not found");
	/* SYSLinux-3.72 COM32 API returns this for a
	   directory or empty file */
	ROSH_COM32("  (COM32) could be a directory or empty file\n");
	break;
    case EIO:
	puts("I/O Error");
	break;
    case EBADF:
	puts("Bad File Descriptor");
	break;
    case EACCES:
	puts("Access DENIED");
	break;
    case ENOTDIR:
	puts("not a directory");
	ROSH_COM32("  (COM32) could be directory\n");
	break;
    case EISDIR:
	puts("IS a directory");
	break;
    case ENOSYS:
	puts("not implemented");
	break;
    default:
	printf("returns error; errno=%d\n", ierrno);
    }
}				/* rosh_error */

/* Concatenate command line arguments into one string
 *	cmdstr	Output command string
 *	argc	Argument Count
 *	argv	Argument Values
 *	barg	Beginning Argument
 */
int rosh_argcat(char *cmdstr, const int argc, char *argv[], const int barg)
{
    int i, arglen, curpos;	/* index, argument length, current position
				   in cmdstr */
    curpos = 0;
    cmdstr[0] = '\0';		/* Nullify string just to be sure */
    for (i = barg; i < argc; i++) {
	arglen = strlen(argv[i]);
	/* Theoretically, this should never be met in SYSLINUX */
	if ((curpos + arglen) > (ROSH_CMD_SZ - 1))
	    arglen = (ROSH_CMD_SZ - 1) - curpos;
	memcpy(cmdstr + curpos, argv[i], arglen);
	curpos += arglen;
	if (curpos >= (ROSH_CMD_SZ - 1)) {
	    /* Hopefully, curpos should not be greater than
	       (ROSH_CMD_SZ - 1) */
	    /* Still need a '\0' at the last character */
	    cmdstr[(ROSH_CMD_SZ - 1)] = 0;
	    break;		/* Escape out of the for() loop;
				   We can no longer process anything more */
	} else {
	    cmdstr[curpos] = ' ';
	    curpos += 1;
	    cmdstr[curpos] = 0;
	}
    }
    /* If there's a ' ' at the end, remove it.  This is normal unless
       the maximum length is met/exceeded. */
    if (cmdstr[curpos - 1] == ' ')
	cmdstr[--curpos] = 0;
    return curpos;
}				/* rosh_argcat */

/*
 * Prints a lot of the data in a struct termios
 */
/*
void rosh_print_tc(struct termios *tio)
{
	printf("  -- termios: ");
	printf(".c_iflag=%04X ", tio->c_iflag);
	printf(".c_oflag=%04X ", tio->c_oflag);
	printf(".c_cflag=%04X ", tio->c_cflag);
	printf(".c_lflag=%04X ", tio->c_lflag);
	printf(".c_cc[VTIME]='%d' ", tio->c_cc[VTIME]);
	printf(".c_cc[VMIN]='%d'", tio->c_cc[VMIN]);
	printf("\n");
}
*/

/*
 * Attempts to get a single key from the console
 *	returns	key pressed
 */
int rosh_getkey(void)
{
    int inc;

    inc = KEY_NONE;
    while (inc == KEY_NONE)
	inc = get_key(stdin, 6000);
    return inc;
}				/* rosh_getkey */

/*
 * Qualifies a filename relative to the working directory
 *	filestr	Filename to qualify
 *	pwdstr	working directory
 *	returns	qualified file name string
 */
void rosh_qualify_filestr(char *filestr, const char *ifilstr,
			  const char *pwdstr)
{
    int filepos = 0;
    if ((filestr) && (pwdstr) && (ifilstr)) {
	if (ifilstr[0] != SEP) {
	    strcpy(filestr, pwdstr);
	    filepos = strlen(pwdstr);
	    if (filestr[filepos - 1] != SEP)
		filestr[filepos++] = SEP;
	}
	strcpy(filestr + filepos, ifilstr);
	ROSH_DEBUG("--'%s'\n", filestr);
    }
}

/* Concatenate multiple files to stdout
 *	cmdstr	command string to process
 */
void rosh_cat(const char *cmdstr)
{
    FILE *f;
    char filestr[ROSH_PATH_SZ];
    char buf[ROSH_BUF_SZ];
    int numrd;
    int cmdpos;

    ROSH_DEBUG("CMD: '%s'\n", cmdstr);
    /* Initialization */
    filestr[0] = 0;
    cmdpos = 0;
    /* skip the first word */
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    while (strlen(filestr) > 0) {
	printf("--File = '%s'\n", filestr);
	f = fopen(filestr, "r");
	if (f != NULL) {
	    numrd = fread(buf, 1, ROSH_BUF_SZ, f);
	    while (numrd > 0) {
		fwrite(buf, 1, numrd, stdout);
		numrd = fread(buf, 1, ROSH_BUF_SZ, f);
	    }
	    fclose(f);
	} else {
	    rosh_error(errno, "cat", filestr);
	    errno = 0;
	}
	cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    }
}				/* rosh_cat */

/* Change PWD (Present Working Directory)
 *	cmdstr	command string to process
 *	ipwdstr	Initial PWD
 */
void rosh_cd(const char *cmdstr, const char *ipwdstr)
{
    int rv;
    char filestr[ROSH_PATH_SZ];
    int cmdpos;
    ROSH_DEBUG("CMD: '%s'\n", cmdstr);
    /* Initialization */
    filestr[0] = 0;
    cmdpos = 0;
    rv = 0;
    /* skip the first word */
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    if (strlen(filestr) != 0)
	rv = chdir(filestr);
    else
	rv = chdir(ipwdstr);
    if (rv != 0) {
	rosh_error(errno, "cd", filestr);
	errno = 0;
    } else {
#ifdef DO_DEBUG
	if (getcwd(filestr, ROSH_PATH_SZ))
	    ROSH_DEBUG("  %s\n", filestr);
#endif /* DO_DEBUG */
    }
}				/* rosh_cd */

/* Print the syslinux config file name
 */
void rosh_cfg(void)
{
    printf("CFG:     '%s'\n", syslinux_config_file());
}				/* rosh_cfg */

/* Process optstr to optarr
 *	optstr	option string to process
 *	optarr	option array to populate
 */
void rosh_ls_arg_opt(const char *optstr, int *optarr)
{
    char *cpos;
    cpos = strchr(optstr, 'l');
    if (cpos) {
	optarr[0] = cpos - optstr;
    } else {
	optarr[0] = -1;
    }
    cpos = strchr(optstr, 'F');
    if (cpos) {
	optarr[1] = cpos - optstr;
    } else {
	optarr[1] = -1;
    }
    cpos = strchr(optstr, 'i');
    if (cpos) {
	optarr[2] = cpos - optstr;
    } else {
	optarr[2] = -1;
    }
}				/* rosh_ls_arg_opt */

/* Retrieve the size of a file argument
 *	filestr	directory name of directory entry
 *	de	directory entry
 */
int rosh_ls_de_size(const char *filestr, struct dirent *de)
{
    int de_size;
    char filestr2[ROSH_PATH_SZ];
    int fd2, file2pos;
    struct stat fdstat;
    int status;

    filestr2[0] = 0;
    file2pos = -1;
    if (filestr) {
	file2pos = strlen(filestr);
	memcpy(filestr2, filestr, file2pos);
	filestr2[file2pos] = '/';
    }
    strcpy(filestr2 + file2pos + 1, de->d_name);
    fd2 = open(filestr2, O_RDONLY);
    status = fstat(fd2, &fdstat);
    fd2 = close(fd2);
    de_size = (int)fdstat.st_size;
    return de_size;
}				/* rosh_ls_de_size */

/* Retrieve the size and mode of a file
 *	filestr	directory name of directory entry
 *	de	directory entry
 */
int rosh_ls_de_size_mode(struct dirent *de, mode_t * st_mode)
{
    int de_size;
//    char filestr2[ROSH_PATH_SZ];
//     int file2pos;
    struct stat fdstat;
    int status;

/*    filestr2[0] = 0;
    file2pos = -1;*/
    fdstat.st_size = 0;
    fdstat.st_mode = 0;
/*    if (filestr) {
	file2pos = strlen(filestr);
	memcpy(filestr2, filestr, file2pos);
	filestr2[file2pos] = '/';
    }
    strcpy(filestr2 + file2pos + 1, de->d_name);*/
    status = stat(de->d_name, &fdstat);
    ROSH_DEBUG2("\t--stat()=%d\terr=%d\n", status, errno);
    if (errno) {
	rosh_error(errno, "ls:szmd.stat", de->d_name);
	errno = 0;
    }
    de_size = (int)fdstat.st_size;
    *st_mode = fdstat.st_mode;
    return de_size;
}				/* rosh_ls_de_size_mode */

/* Returns the Inode number if fdstat contains it
 *	fdstat	struct to extract inode from if not COM32, for now
 */
long rosh_ls_d_ino(struct stat *fdstat)
{
    long de_ino;
#ifdef __COM32__
    if (fdstat)
	de_ino = -1;
    else
	de_ino = 0;
#else /* __COM32__ */
    de_ino = fdstat->st_ino;
#endif /* __COM32__ */
    return de_ino;
}

/* Convert a d_type to a single char in human readable format
 *	d_type	d_type to convert
 *	returns human readable single character; a space if other
 */
char rosh_d_type2char_human(unsigned char d_type)
{
    char ret;
    switch (d_type) {
    case DT_UNKNOWN:
	ret = 'U';
	break;			/* Unknown */
    case DT_FIFO:
	ret = 'F';
	break;			/* FIFO */
    case DT_CHR:
	ret = 'C';
	break;			/* Char Dev */
    case DT_DIR:
	ret = 'D';
	break;			/* Directory */
    case DT_BLK:
	ret = 'B';
	break;			/* Block Dev */
    case DT_REG:
	ret = 'R';
	break;			/* Regular File */
    case DT_LNK:
	ret = 'L';
	break;			/* Link, Symbolic */
    case DT_SOCK:
	ret = 'S';
	break;			/* Socket */
    case DT_WHT:
	ret = 'W';
	break;			/* UnionFS Whiteout */
    default:
	ret = ' ';
    }
    return ret;
}				/* rosh_d_type2char_human */

/* Convert a d_type to a single char by ls's prefix standards for -l
 *	d_type	d_type to convert
 *	returns ls style single character; a space if other
 */
char rosh_d_type2char_lspre(unsigned char d_type)
{
    char ret;
    switch (d_type) {
    case DT_FIFO:
	ret = 'p';
	break;
    case DT_CHR:
	ret = 'c';
	break;
    case DT_DIR:
	ret = 'd';
	break;
    case DT_BLK:
	ret = 'b';
	break;
    case DT_REG:
	ret = '-';
	break;
    case DT_LNK:
	ret = 'l';
	break;
    case DT_SOCK:
	ret = 's';
	break;
    default:
	ret = '?';
    }
    return ret;
}				/* rosh_d_type2char_lspre */

/* Convert a d_type to a single char by ls's classify (-F) suffix standards
 *	d_type	d_type to convert
 *	returns ls style single character; a space if other
 */
char rosh_d_type2char_lssuf(unsigned char d_type)
{
    char ret;
    switch (d_type) {
    case DT_FIFO:
	ret = '|';
	break;
    case DT_DIR:
	ret = '/';
	break;
    case DT_LNK:
	ret = '@';
	break;
    case DT_SOCK:
	ret = '=';
	break;
    default:
	ret = ' ';
    }
    return ret;
}				/* rosh_d_type2char_lssuf */

/* Converts data in the "other" place of st_mode to a ls-style string
 *	st_mode	Mode in other to analyze
 *	st_mode_str	string to hold converted string
 */
void rosh_st_mode_am2str(mode_t st_mode, char *st_mode_str)
{
    st_mode_str[0] = ((st_mode & S_IROTH) ? 'r' : '-');
    st_mode_str[1] = ((st_mode & S_IWOTH) ? 'w' : '-');
    st_mode_str[2] = ((st_mode & S_IXOTH) ? 'x' : '-');
}

/* Converts st_mode to an ls-style string
 *	st_mode	mode to convert
 *	st_mode_str	string to hold converted string
 */
void rosh_st_mode2str(mode_t st_mode, char *st_mode_str)
{
    st_mode_str[0] = rosh_d_type2char_lspre(IFTODT(st_mode));
    rosh_st_mode_am2str((st_mode & S_IRWXU) >> 6, st_mode_str + 1);
    rosh_st_mode_am2str((st_mode & S_IRWXG) >> 3, st_mode_str + 4);
    rosh_st_mode_am2str(st_mode & S_IRWXO, st_mode_str + 7);
    st_mode_str[10] = 0;
}				/* rosh_st_mode2str */

/* Output a single entry
 *	filestr	directory name to list
 *	de	directory entry
 *	optarr	Array of options
 */
void rosh_ls_arg_dir_de(struct dirent *de, const int *optarr)
{
    int de_size;
    mode_t st_mode;
    char st_mode_str[11];
    st_mode = 0;
    if (optarr[2] > -1)
	printf("%10d ", (int)de->d_ino);
    if (optarr[0] > -1) {
	de_size = rosh_ls_de_size_mode(de, &st_mode);
	rosh_st_mode2str(st_mode, st_mode_str);
	ROSH_DEBUG2("%04X ", st_mode);
	printf("%s %10d ", st_mode_str, de_size);
    }
    ROSH_DEBUG("'");
    printf("%s", de->d_name);
    ROSH_DEBUG("'");
    if (optarr[1] > -1)
	printf("%c", rosh_d_type2char_lssuf(de->d_type));
    printf("\n");
}				/* rosh_ls_arg_dir_de */

/* Output listing of a regular directory
 *	filestr	directory name to list
 *	d	the open DIR
 *	optarr	Array of options
	NOTE:This is where I could use qsort
 */
void rosh_ls_arg_dir(const char *filestr, DIR * d, const int *optarr)
{
    struct dirent *de;
    int filepos;

    filepos = 0;
    while ((de = readdir(d))) {
	filepos++;
	rosh_ls_arg_dir_de(de, optarr);
    }
    if (errno)
	rosh_error(errno, "ls:arg_dir", filestr);
    else if (filepos == 0)
	ROSH_DEBUG("0 files found");
}				/* rosh_ls_arg_dir */

/* Simple directory listing for one argument (file/directory) based on
 * filestr and pwdstr
 *	ifilstr	input filename/directory name to list
 *	pwdstr	Present Working Directory string
 *	optarr	Option Array
 */
void rosh_ls_arg(const char *filestr, const int *optarr)
{
    struct stat fdstat;
    int status;
//     char filestr[ROSH_PATH_SZ];
//     int filepos;
    DIR *d;
    struct dirent de;

    /* Initialization; make filestr based on leading character of ifilstr
       and pwdstr */
//     rosh_qualify_filestr(filestr, ifilstr, pwdstr);
    fdstat.st_mode = 0;
    fdstat.st_size = 0;
    ROSH_DEBUG("\topt[0]=%d\topt[1]=%d\topt[2]=%d\n", optarr[0], optarr[1],
	       optarr[2]);

    /* Now, the real work */
    errno = 0;
    status = stat(filestr, &fdstat);
    if (status == 0) {
	if (S_ISDIR(fdstat.st_mode)) {
	    ROSH_DEBUG("PATH '%s' is a directory\n", filestr);
	    d = opendir(filestr);
	    rosh_ls_arg_dir(filestr, d, optarr);
	    closedir(d);
	} else {
	    de.d_ino = rosh_ls_d_ino(&fdstat);
	    de.d_type = (IFTODT(fdstat.st_mode));
	    strcpy(de.d_name, filestr);
	    if (S_ISREG(fdstat.st_mode)) {
		ROSH_DEBUG("PATH '%s' is a regular file\n", filestr);
	    } else {
		ROSH_DEBUG("PATH '%s' is some other file\n", filestr);
	    }
	    rosh_ls_arg_dir_de(&de, optarr);
/*	    if (ifilstr[0] == SEP)
		rosh_ls_arg_dir_de(NULL, &de, optarr);
	    else
		rosh_ls_arg_dir_de(pwdstr, &de, optarr);*/
	}
    } else {
	rosh_error(errno, "ls", filestr);
	errno = 0;
    }
    return;
}				/* rosh_ls_arg */

/* Parse options that may be present in the cmdstr
 *	filestr	Possible option string to parse
 *	optstr	Current options
 *	returns 1 if filestr does not begin with '-' else 0
 */
int rosh_ls_parse_opt(const char *filestr, char *optstr)
{
    int ret;
    if (filestr[0] == '-') {
	ret = 0;
	if (optstr)
	    strcat(optstr, filestr + 1);
    } else {
	ret = 1;
    }
    ROSH_DEBUG("ParseOpt: '%s'\n\topt: '%s'\n\tret: %d\n", filestr, optstr,
	       ret);
    return ret;
}				/* rosh_ls_parse_opt */

/* List Directory based on cmdstr and pwdstr
 *	cmdstr	command string to process
 *	pwdstr	Present Working Directory string
 */
void rosh_ls(const char *cmdstr)
{
    char filestr[ROSH_PATH_SZ];
    char optstr[ROSH_OPT_SZ];	/* Options string */
    int cmdpos, tpos;		/* Position within cmdstr, temp position */
    int numargs;		/* number of non-option arguments */
    int argpos;			/* number of non-option arguments processed */
    int optarr[3];

    ROSH_DEBUG("CMD: '%s'\n", cmdstr);
    /* Initialization */
    filestr[0] = 0;
    optstr[0] = 0;
    cmdpos = 0;
    numargs = 0;
    argpos = 0;
    /* skip the first word */
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    tpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    /* If there are no real arguments, substitute PWD */
    if (strlen(filestr) == 0) {
	strcpy(filestr, ".");
	cmdpos = tpos;
    } else {			/* Parse for command line options */
	while (strlen(filestr) > 0) {
	    numargs += rosh_ls_parse_opt(filestr, optstr);
	    tpos = rosh_parse_sp_1(filestr, cmdstr, tpos);
	}
	if (numargs == 0) {
	    strcpy(filestr, ".");
	    cmdpos = tpos;
	} else {
	    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
	}
    }
#ifdef DO_DEBUG
    if (!strchr(optstr, 'l'))
	strcat(optstr, "l");
#endif /* DO_DEBUG */
    rosh_ls_arg_opt(optstr, optarr);
    ROSH_DEBUG("\tfopt: '%s'\n", optstr);
    while (strlen(filestr) > 0) {
	if (rosh_ls_parse_opt(filestr, NULL)) {
	    rosh_ls_arg(filestr, optarr);
	    argpos++;
	}
	if (argpos < numargs)
	    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
	else
	    break;
    }
}				/* rosh_ls */

/* Simple directory listing; calls rosh_ls()
 *	cmdstr	command string to process
 *	pwdstr	Present Working Directory string
 */
void rosh_dir(const char *cmdstr)
{
    ROSH_DEBUG("  dir implemented as ls\n");
    rosh_ls(cmdstr);
}				/* rosh_dir */

/* Page through a buffer string
 *	buf	Buffer to page through
 */
void rosh_more_buf(char *buf, int buflen, int rows, int cols, char *scrbuf)
{
    char *bufp, *bufeol, *bufeol2;	/* Pointer to current and next
					   end-of-line position in buffer */
    int bufpos, bufcnt;		/* current position, count characters */
    int inc;
    int i, numln;		/* Index, Number of lines */
    int elpl;		/* Extra lines per line read */

    (void)cols;

    bufpos = 0;
    bufp = buf + bufpos;
    bufeol = bufp;
    numln = rows - 1;
    ROSH_DEBUG("--(%d)\n", buflen);
    while (bufpos < buflen) {
	for (i = 0; i < numln; i++) {
	    bufeol2 = strchr(bufeol, '\n');
	    if (bufeol2 == NULL) {
		bufeol = buf + buflen;
		i = numln;
	    } else {
		elpl = ((bufeol2 - bufeol - 1) / cols);
		if (elpl < 0)
		    elpl = 0;
		i += elpl;
		ROSH_DEBUG2("  %d/%d  ", elpl, i+1);
		/* If this will not push too much, use it */
		/* but if it's the first line, use it */
		/* //HERE: We should probably snip the line off */
		if ((i < numln) || (i == elpl))
		    bufeol = bufeol2 + 1;
	    }
	}
	ROSH_DEBUG2("\n");
	bufcnt = bufeol - bufp;
	printf("--(%d/%d @%d)\n", bufcnt, buflen, bufpos);
	memcpy(scrbuf, bufp, bufcnt);
	scrbuf[bufcnt] = 0;
	printf("%s", scrbuf);
	bufp = bufeol;
	bufpos += bufcnt;
	if (bufpos == buflen)
	    break;
	inc = rosh_getkey();
	numln = 1;
	switch (inc) {
	case KEY_CTRL('c'):
	case 'q':
	case 'Q':
	    bufpos = buflen;
	    break;
	case ' ':
	    numln = rows - 1;
	}
    }
}				/* rosh_more_buf */

/* Page through a single file using the open file stream
 *	fd	File Descriptor
 */
void rosh_more_fd(int fd, int rows, int cols, char *scrbuf)
{
    struct stat fdstat;
    int status;
    char *buf;
    int bufpos;
    int numrd;
    FILE *f;

    status = fstat(fd, &fdstat);
    if (S_ISREG(fdstat.st_mode)) {
	buf = malloc((int)fdstat.st_size);
	if (buf != NULL) {
	    f = fdopen(fd, "r");
	    bufpos = 0;
	    numrd = fread(buf, 1, (int)fdstat.st_size, f);
	    while (numrd > 0) {
		bufpos += numrd;
		numrd = fread(buf + bufpos, 1,
			      ((int)fdstat.st_size - bufpos), f);
	    }
	    fclose(f);
	    rosh_more_buf(buf, bufpos, rows, cols, scrbuf);
	}
    } else {
    }

}				/* rosh_more_fd */

/* Page through a file like the more command
 *	cmdstr	command string to process
 *	ipwdstr	Initial PWD
 */
void rosh_more(const char *cmdstr)
{
    int fd;
    char filestr[ROSH_PATH_SZ];
    int cmdpos;
    int rows, cols;
    char *scrbuf;
    int ret;

    ROSH_DEBUG("CMD: '%s'\n", cmdstr);
    /* Initialization */
    filestr[0] = 0;
    cmdpos = 0;
    ret = getscreensize(1, &rows, &cols);
    if (ret) {
	ROSH_DEBUG("getscreensize() fail(%d); fall back\n", ret);
	ROSH_DEBUG("\tROWS='%d'\tCOLS='%d'\n", rows, cols);
	/* If either fail, go under normal size, just in case */
	if (!rows)
	    rows = 20;
	if (!cols)
	    cols = 75;
    }
    ROSH_DEBUG("\tUSE ROWS='%d'\tCOLS='%d'\n", rows, cols);
    /* 32 bit align beginning of row and over allocate */
    scrbuf = malloc(rows * ((cols+3)&(INT_MAX - 3)));
    if (!scrbuf)
	return;

    /* skip the first word */
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
    if (strlen(filestr) > 0) {
	/* There is no need to mess up the console if we don't have a
	   file */
	rosh_console_raw();
	while (strlen(filestr) > 0) {
	    printf("--File = '%s'\n", filestr);
	    fd = open(filestr, O_RDONLY);
	    if (fd != -1) {
		rosh_more_fd(fd, rows, cols, scrbuf);
		close(fd);
	    } else {
		rosh_error(errno, "more", filestr);
		errno = 0;
	    }
	    cmdpos = rosh_parse_sp_1(filestr, cmdstr, cmdpos);
	}
	rosh_console_std();
    }
    free(scrbuf);
}				/* rosh_more */

/* Page a file with rewind
 *	cmdstr	command string to process
 *	pwdstr	Present Working Directory string
 *	ipwdstr	Initial PWD
 */
void rosh_less(const char *cmdstr)
{
    printf("  less implemented as more (for now)\n");
    rosh_more(cmdstr);
}				/* rosh_less */

/* Show PWD
 *	cmdstr	command string to process
 */
void rosh_pwd(const char *cmdstr)
{
    int istr;
    char pwdstr[ROSH_PATH_SZ];
    if (cmdstr)
	istr = 0;
    ROSH_DEBUG("CMD: '%s'\n", cmdstr);
    errno = 0;
    if (getcwd(pwdstr, ROSH_PATH_SZ)) {
	printf("%s\n", pwdstr);
    } else {
	rosh_error(errno, "pwd", "");
	errno = 0;
    }
    istr = htonl(*(int *)pwdstr);
    ROSH_DEBUG2("  --%08X\n", istr);
}				/* rosh_pwd */

/* Reboot
 */
void rosh_reboot(void)
{
//     char cmdstr[ROSH_CMD_SZ];
//     printf
    syslinux_reboot(0);
}				/* rosh_reboot */

/* Run a boot string, calling syslinux_run_command
 *	cmdstr	command string to process
 */
void rosh_run(const char *cmdstr)
{
    int cmdpos;
    char *cmdptr;

    cmdpos = 0;
    ROSH_DEBUG("CMD: '%s'\n", cmdstr);
    /* skip the first word */
    cmdpos = rosh_search_sp(cmdstr, cmdpos);
    /* skip spaces */
    cmdpos = rosh_search_nonsp(cmdstr, cmdpos);
    cmdptr = (char *)(cmdstr + cmdpos);
    printf("--run: '%s'\n", cmdptr);
    syslinux_run_command(cmdptr);
}				/* rosh_run */

/* Process a single command string and call handling function
 *	cmdstr	command string to process
 *	ipwdstr	Initial Present Working Directory string
 *	returns	Whether to exit prompt
 */
char rosh_command(const char *cmdstr, const char *ipwdstr)
{
    char do_exit;
    char tstr[ROSH_CMD_SZ];
    int tlen;
    do_exit = false;
    ROSH_DEBUG("--cmd:'%s'\n", cmdstr);
    tlen = rosh_parse_sp_1(tstr, cmdstr, 0);
    switch (cmdstr[0]) {
    case 'e':
    case 'E':
    case 'q':
    case 'Q':
	if ((strncasecmp("exit", tstr, tlen) == 0) ||
	    (strncasecmp("quit", tstr, tlen) == 0))
	    do_exit = true;
	else
	    rosh_help(1, NULL);
	break;
    case 'c':
    case 'C':			/* run 'cd' 'cat' 'cfg' */
	switch (cmdstr[1]) {
	case 'a':
	case 'A':
	    if (strncasecmp("cat", tstr, tlen) == 0)
		rosh_cat(cmdstr);
	    else
		rosh_help(1, NULL);
	    break;
	case 'd':
	case 'D':
	    if (strncasecmp("cd", tstr, tlen) == 0)
		rosh_cd(cmdstr, ipwdstr);
	    else
		rosh_help(1, NULL);
	    break;
	case 'f':
	case 'F':
	    if (strncasecmp("cfg", tstr, tlen) == 0)
		rosh_cfg();
	    else
		rosh_help(1, NULL);
	    break;
	default:
	    rosh_help(1, NULL);
	}
	break;
    case 'd':
    case 'D':			/* run 'dir' */
	if (strncasecmp("dir", tstr, tlen) == 0)
	    rosh_dir(cmdstr);
	else
	    rosh_help(1, NULL);
	break;
    case 'h':
    case 'H':
    case '?':
	if ((strncasecmp("help", tstr, tlen) == 0) || (tlen == 1))
	    rosh_help(2, cmdstr);
	else
	    rosh_help(1, NULL);
	break;
    case 'l':
    case 'L':			/* run 'ls' 'less' */
	switch (cmdstr[1]) {
	case 0:
	case ' ':
	case 's':
	case 'S':
	    if (strncasecmp("ls", tstr, tlen) == 0)
		rosh_ls(cmdstr);
	    else
		rosh_help(1, NULL);
	    break;
	case 'e':
	case 'E':
	    if (strncasecmp("less", tstr, tlen) == 0)
		rosh_less(cmdstr);
	    else
		rosh_help(1, NULL);
	    break;
	default:
	    rosh_help(1, NULL);
	}
	break;
    case 'm':
    case 'M':
	switch (cmdstr[1]) {
	case 'a':
	case 'A':
	    if (strncasecmp("man", tstr, tlen) == 0)
		rosh_help(2, cmdstr);
	    else
		rosh_help(1, NULL);
	    break;
	case 'o':
	case 'O':
	    if (strncasecmp("more", tstr, tlen) == 0)
		rosh_more(cmdstr);
	    else
		rosh_help(1, NULL);
	    break;
	default:
	    rosh_help(1, NULL);
	}
	break;
    case 'p':
    case 'P':			/* run 'pwd' */
	if (strncasecmp("pwd", tstr, tlen) == 0)
	    rosh_pwd(cmdstr);
	else
	    rosh_help(1, NULL);
	break;
    case 'r':
    case 'R':			/* run 'run' */
	switch (cmdstr[1]) {
	case 0:
	case ' ':
	case 'e':
	case 'E':
	    if (strncasecmp("reboot", tstr, tlen) == 0)
		rosh_reboot();
	    else
		rosh_help(1, NULL);
	    break;
	case 'u':
	case 'U':
	    if (strncasecmp("run", tstr, tlen) == 0)
		rosh_run(cmdstr);
	    else
		rosh_help(1, NULL);
	    break;
	}
	break;
    case 'v':
    case 'V':
	if (strncasecmp("version", tstr, tlen) == 0)
	    rosh_version(1);
	else
	    rosh_help(1, NULL);
	break;
    case 0:
    case '\n':
	break;
    default:
	rosh_help(1, NULL);
    }				/* switch(cmdstr[0]) */
    return do_exit;
}				/* rosh_command */

/* Process the prompt for commands as read from stdin and call rosh_command
 * to process command line string
 *	icmdstr	Initial command line string
 *	returns	Exit status
 */
int rosh_prompt(const char *icmdstr)
{
    int rv;
    char cmdstr[ROSH_CMD_SZ];
    char ipwdstr[ROSH_PATH_SZ];
    char do_exit;
    char *c;

    rv = 0;
    do_exit = false;
    if (!getcwd(ipwdstr, ROSH_PATH_SZ))
	strcpy(ipwdstr, "./");
    if (icmdstr[0] != '\0')
	do_exit = rosh_command(icmdstr, ipwdstr);
    while (!(do_exit)) {
	/* Extra preceeding newline */
	printf("\nrosh: ");
	/* Read a line from console */
	if (fgets(cmdstr, ROSH_CMD_SZ, stdin)) {
	    /* remove newline from input string */
	    c = strchr(cmdstr, '\n');
	    *c = 0;
	    do_exit = rosh_command(cmdstr, ipwdstr);
	} else {
	    do_exit = false;
	}
    }
    return rv;
}

int main(int argc, char *argv[])
{
    int rv;
    char cmdstr[ROSH_CMD_SZ];

    /* Initialization */
    rv = 0;
    rosh_console_std();
    if (argc != 1) {
	rv = rosh_argcat(cmdstr, argc, argv, 1);
    } else {
	rosh_version(0);
	print_beta();
	cmdstr[0] = '\0';
    }
    rv = rosh_prompt(cmdstr);
    printf("--Exiting '%s'\n", APP_NAME);
    return rv;
}
