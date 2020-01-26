#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

#include "wcxhead.h"

#define PROG_LOGO																\
"Total Commander WCX plugin Test ver. 0.23. Copyright (c) 2004-2006 Oleg Bondar.\n"	\
"Implements algorithm described in \"WCX Writer\'s Reference\" to test plugin's\n"	\
"functionality.\n"

#define PROG_USAGE																\
"Usage:\n"																   \
"  wcxtest [-f | -l | | -t | -x] [-q] <wcx_path> [<arc_path>] [<dir_path>]\n" \
"\n"																			\
"<wcx_path> - path to WCX plugin\n"												\
"<arc_path> - path to archive file\n"											\
"<dir_path> - directory to unpack files, default is current\n"					\
"  -f - list WCX exported functions\n"											\
"  -l - List archive contents (default)\n"										\
"  -t - Test archive contents\n"												\
"  -x - eXtract files from archive (overwrite existing)\n"						\
"  -?, -h - this topic\n"														\
"\n"																			\
"  -v - Verbose\n"																\
"\n"																			\
"ERRORLEVEL: 0 - success, non zero - some (m.b. unknown) error.\n"				\
"\n"																			\
"Switches are NOT case sensitive. It\'s order - arbitrary.\n"					\
"Program NOT tested with file names that contains non-ASCII chars.\n"


#define TODO_FLIST		0
#define TODO_LIST		1
#define TODO_TEST		2
#define TODO_EXTRACT	3

#define ERR_NO_WCX		1
#define ERR_NO_PROC		2
#define ERR_OPEN_FAIL	3
#define ERR_CLOSE_FAIL	4


/* mandatory functions */
static HANDLE	(__stdcall *pOpenArchive)(tOpenArchiveData *ArchiveData) = NULL;
static int		(__stdcall *pReadHeader)(HANDLE hArcData, tHeaderData *HeaderData) = NULL;
static int		(__stdcall *pProcessFile)(HANDLE hArcData, int Operation, char *DestPath, char *DestName) = NULL;
static int		(__stdcall *pCloseArchive)(HANDLE hArcData) = NULL;

/* optional functions */
static int		(__stdcall *pPackFiles)(char *PackedFile, char *SubPath, char *SrcPath, char *AddList, int Flags) = NULL;
static int		(__stdcall *pDeleteFiles)(char *PackedFile, char *DeleteList) = NULL;
static int		(__stdcall *pGetPackerCaps)(void) = NULL;
static void		(__stdcall *pConfigurePacker)(HWND Parent, HINSTANCE DllInstance);
static void		(__stdcall *pSetChangeVolProc)(HANDLE hArcData, tChangeVolProc pChangeVolProc1) = NULL;		 /* NOT quite */
static void		(__stdcall *pSetProcessDataProc)(HANDLE hArcData, tProcessDataProc pProcessDataProc) = NULL; /* NOT quite */

/* packing into memory */
static int		(__stdcall *pStartMemPack)(int Options, char *FileName) = NULL;
static int		(__stdcall *pPackToMem)(int hMemPack, char* BufIn, int InLen, int* Taken,
										char* BufOut, int OutLen, int* Written, int SeekBy) = NULL;
static int		(__stdcall *pDoneMemPack)(int hMemPack) = NULL;

static BOOL		(__stdcall *pCanYouHandleThisFile)(char *FileName) = NULL;
static void		(__stdcall *pPackSetDefaultParams)(PackDefaultParamStruct* dps) = NULL;

static char wcx_path[MAX_PATH] = "";
static char arc_path[MAX_PATH] = "";
static char dir_path[MAX_PATH] = ".\\";
static char prog_prefix[] = "WT: ";
static int	open_todo = -1;
static int	verbose = 0;


static char *
WCX_err_msg(int code)
{
	static char		buf[256];

	switch(code) {
		case E_END_ARCHIVE:		strcpy(buf, "No more files in archive");	break;
		case E_NO_MEMORY:		strcpy(buf, "Not enough memory");			break;
		case E_BAD_DATA:		strcpy(buf, "Data is bad");					break;
		case E_BAD_ARCHIVE:		strcpy(buf, "CRC error in archive data");	break;
		case E_UNKNOWN_FORMAT:	strcpy(buf, "Archive format unknown");		break;
		case E_EOPEN:			strcpy(buf, "Cannot open existing file");	break;
		case E_ECREATE:			strcpy(buf, "Cannot create file");			break;
		case E_ECLOSE:			strcpy(buf, "Error closing file");			break;
		case E_EREAD:			strcpy(buf, "Error reading from file");		break;
		case E_EWRITE:			strcpy(buf, "Error writing to file");		break;
		case E_SMALL_BUF:		strcpy(buf, "Buffer too small");			break;
		case E_EABORTED:		strcpy(buf, "Function aborted by user");	break;
		case E_NO_FILES:		strcpy(buf, "No files found");				break;
		case E_TOO_MANY_FILES:	strcpy(buf, "Too many files to pack");		break;
		case E_NOT_SUPPORTED:	strcpy(buf, "Function not supported");		break;

		default: sprintf(buf, "Unknown error code (%d)", code); break;
	}

	return buf;
}

static int __stdcall
ChangeVol(char *ArcName, int Mode)
{
	char	buf[32];
	int		rc = 0;

	switch(Mode) {
		case PK_VOL_ASK:
		printf("%sPlease change disk and enter Y or N to stop: ", prog_prefix);
		gets(buf);
		rc = *buf == 'y' || *buf == 'Y';
		break;

		case PK_VOL_NOTIFY:
		printf("%sProcessing next volume/diskette\n", prog_prefix);
		rc = 1;
		break;

		default:
		printf("%sUnknown ChangeVolProc mode\n", prog_prefix);
		rc = 0;
		break;
	}

	return rc;
}

static int __stdcall
ProcessData(char *FileName, int Size)
{
//	  char	  buf[MAX_PATH];

//	  CharToOem(FileName, buf);
//	  printf("%sProcessed %s (%d). Ok.\n", prog_prefix, buf, Size); fflush(stdout);
	printf(".");
	return 1;
}


#define DIR_SEPARATOR '\\'
#define DRV_SEPARATOR ':'

void
check_fndir(char *fname)
{
	struct _stat	sb;
	char			*s, buf[MAX_PATH];

	/* check if dir exists; if not create */
	for(s = fname; *s; ) {
		if(*s == DIR_SEPARATOR) ++s;
		while (*s && *s != DIR_SEPARATOR) ++s;
		if(*s == DIR_SEPARATOR) {
			*s = 0;
			/* there is no difference in speed: check if exists directory */
			if(_stat(fname, & sb) == -1) {
				CharToOem(fname, buf);
				if(verbose) printf("%s-- Making dir %s\n", prog_prefix, buf);
				mkdir(fname);
			}
			*s = DIR_SEPARATOR;
		}
	}
}


extern int
main(int argc, char *argv[])
{
	int					i, j, rc = 0;
	char				*s, buf[1024];
	HINSTANCE			hwcx = NULL;
	HANDLE				arch = NULL;
	tOpenArchiveData	arcd;
	tHeaderData			hdrd;

	if (argc < 2) {
		printf(PROG_LOGO);
		printf(PROG_USAGE);
		return 0;
	}

	if (argc == 2) open_todo = TODO_FLIST;

	/* switches */
	for(i = 1; i < argc; ++i) {
		s = argv[i];
		if(*s != '-' && *s != '/') continue;
		++s;
		switch(*s) {
			case 'f':
			case 'F': /* list of functions mode */
			if(open_todo < 0) open_todo = TODO_FLIST;
			else {
				printf("Syntax error: too many switches.\n");
				printf(PROG_USAGE);
				return 0;
			}
			break;

			case 'l':
			case 'L': /* list mode */
			if(open_todo < 0) open_todo = TODO_LIST;
			else {
				printf("Syntax error: too many switches.\n");
				printf(PROG_USAGE);
				return 0;
			}
			break;

			case 't':
			case 'T': /* test mode */
			if(open_todo < 0) open_todo = TODO_TEST;
			else {
				printf("Syntax error: too many switches.\n");
				printf(PROG_USAGE);
				return 0;
			}
			break;

			case 'x':
			case 'X': /* extract mode */
			if(open_todo < 0) open_todo = TODO_EXTRACT;
			else {
				printf("Syntax error: too many switches.\n");
				printf(TODO_FLIST);
				return 0;
			}
			break;

			case 'v':
			case 'V':
			verbose = 1;
			break;

			case '?':
			case 'h':
			case 'H':
			printf(PROG_LOGO);
			printf(TODO_FLIST);
			return 0;

			default:
			printf("Syntax error: invalid switch.\n");
			printf(PROG_USAGE);
			return 0;
		}
	}
	if(open_todo < 0) open_todo = TODO_LIST;
	if(!verbose) *prog_prefix = 0;

	/* parameters */
	for(i = 1, j = 0; i < argc; ++i) {
		s = argv[i];
		if(*s == '-' || *s == '/') continue;
		switch(j) {
			case 0:
			strcpy(wcx_path, argv[i]);
			break;

			case 1:
			strcpy(arc_path, argv[i]);
			break;

			case 2:
			strcpy(dir_path, argv[i]);
			break;

			default:
			printf("Syntax error: too many arguments.\n");
			printf(PROG_USAGE);
			return 0;
		}
		++j;
	}

	if(!*wcx_path) {
		printf("Syntax error: no WCX name.\n");
		printf(PROG_USAGE);
		return 0;
	}
	if(!*arc_path && (open_todo == TODO_LIST || open_todo == TODO_TEST || open_todo == TODO_EXTRACT)) {
		printf("Syntax error: no archive name.\n");
		printf(PROG_USAGE);
		return 0;
	}
	if(*dir_path && dir_path[strlen(dir_path)-1] != '\\') strcat(dir_path,"\\");

	if(verbose) {
		switch(open_todo) {
			case TODO_FLIST:
			//printf("%sExported functions in \"%s\":\n", prog_prefix, wcx_path);
			break;

			case TODO_LIST:
			printf("%sUsing \"%s\" for list files in \"%s\".\n", prog_prefix, wcx_path, arc_path);
			break;

			case TODO_TEST:
			printf("%sUsing \"%s\" for test files in \"%s\".\n", prog_prefix, wcx_path, arc_path);
			break;

			case TODO_EXTRACT:
			printf("%sUsing \"%s\" for extract files from \"%s\" to \"%s\".\n", prog_prefix, wcx_path, arc_path, dir_path);
			break;

			default:
			printf("unknown to do with");
			break;
		}
	}

	if(verbose) printf("%sLoading plugin %s... ", prog_prefix, wcx_path);
	if(!(hwcx = LoadLibrary(wcx_path))) {
		if(verbose) printf("Failed.\n");
		else printf("Failed loading plugin.\n");
		return ERR_NO_WCX;
	}
	if(verbose) printf("Ok.\n");

	/* mandatory */
	pOpenArchive =			(void *) GetProcAddress(hwcx, "OpenArchive");
	pReadHeader =			(void *) GetProcAddress(hwcx, "ReadHeader");
	pProcessFile =			(void *) GetProcAddress(hwcx, "ProcessFile");
	pCloseArchive =			(void *) GetProcAddress(hwcx, "CloseArchive");
	/* optional */
	pPackFiles =			(void *) GetProcAddress(hwcx, "PackFiles");
	pDeleteFiles =			(void *) GetProcAddress(hwcx, "DeleteFiles");
	pGetPackerCaps =		(void *) GetProcAddress(hwcx, "GetPackerCaps");
	pConfigurePacker =		(void *) GetProcAddress(hwcx, "ConfigurePacker");
	/* NOT optional */
	pSetChangeVolProc =		(void *) GetProcAddress(hwcx, "SetChangeVolProc");
	pSetProcessDataProc =	(void *) GetProcAddress(hwcx, "SetProcessDataProc");
	/* optional */
	pStartMemPack =			(void *) GetProcAddress(hwcx, "StartMemPack");
	pPackToMem =			(void *) GetProcAddress(hwcx, "PackToMem");
	pDoneMemPack =			(void *) GetProcAddress(hwcx, "DoneMemPack");
	pCanYouHandleThisFile = (void *) GetProcAddress(hwcx, "CanYouHandleThisFile");
	pPackSetDefaultParams = (void *) GetProcAddress(hwcx, "PackSetDefaultParams");

	if(open_todo == TODO_FLIST) {
		printf("%sExported WCX functions in \"%s\":\n", prog_prefix, wcx_path);
		if(pOpenArchive			) printf("%s  OpenArchive\n", prog_prefix);
		if(pReadHeader			) printf("%s  ReadHeader\n", prog_prefix);
		if(pProcessFile			) printf("%s  ProcessFile\n", prog_prefix);
		if(pCloseArchive		) printf("%s  CloseArchive\n", prog_prefix);
		if(pPackFiles			) printf("%s  PackFiles\n", prog_prefix);
		if(pDeleteFiles			) printf("%s  DeleteFiles\n", prog_prefix);
		if(pGetPackerCaps		) printf("%s  GetPackerCaps\n", prog_prefix);
		if(pConfigurePacker		) printf("%s  ConfigurePacker\n", prog_prefix);
		if(pSetChangeVolProc	) printf("%s  SetChangeVolProc\n", prog_prefix);
		if(pSetProcessDataProc	) printf("%s  SetProcessDataProc\n", prog_prefix);
		if(pStartMemPack		) printf("%s  StartMemPack\n", prog_prefix);
		if(pPackToMem			) printf("%s  PackToMem\n", prog_prefix);
		if(pDoneMemPack			) printf("%s  DoneMemPack\n", prog_prefix);
		if(pCanYouHandleThisFile) printf("%s  CanYouHandleThisFile\n", prog_prefix);
		if(pPackSetDefaultParams) printf("%s  PackSetDefaultParams\n", prog_prefix);

		if(pGetPackerCaps) {
			int		pc = pGetPackerCaps(), f = 0;

			printf("%sPackerCaps: %u =", prog_prefix, pc);
			if(pc & PK_CAPS_NEW			) { printf("%s PK_CAPS_NEW", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_MODIFY		) { printf("%s PK_CAPS_MODIFY", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_MULTIPLE	) { printf("%s PK_CAPS_MULTIPLE", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_DELETE		) { printf("%s PK_CAPS_DELETE", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_OPTIONS		) { printf("%s PK_CAPS_OPTIONS", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_MEMPACK		) { printf("%s PK_CAPS_MEMPACK", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_BY_CONTENT	) { printf("%s PK_CAPS_BY_CONTENT", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_SEARCHTEXT	) { printf("%s PK_CAPS_SEARCHTEXT", f ? " |" : ""); f = 1; }
			if(pc & PK_CAPS_HIDE		) { printf("%s PK_CAPS_HIDE", f ? " |" : ""); f = 1; }
			printf("\n");
		}
		goto stop;
	}

	if(!pOpenArchive || !pReadHeader || !pProcessFile || !pCloseArchive) {
		printf("%sError: There IS NOT mandatory function(s):", prog_prefix);
		if(!pOpenArchive ) printf(" OpenArchive");
		if(!pReadHeader	 ) printf(" ReadHeader");
		if(!pProcessFile ) printf(" ProcessFile");
		if(!pCloseArchive) printf(" CloseArchive");
		printf("\n");
		rc = ERR_NO_PROC;
		goto stop;
	}
	if(!pSetChangeVolProc || !pSetProcessDataProc) {
		printf("%sError: There IS NOT NOT-optional function(s):", prog_prefix);
		if(!pSetChangeVolProc  ) printf(" SetChangeVolProc");
		if(!pSetProcessDataProc) printf(" SetProcessDataProc");
		printf("\n");
		rc = ERR_NO_PROC;
		goto stop;
	}

	if(verbose) printf("%sOpening archive %s...\n", prog_prefix, arc_path);
	memset(&arcd, 0, sizeof(arcd));
	arcd.ArcName = arc_path;
	switch(open_todo) {
		case TODO_LIST:
		arcd.OpenMode = PK_OM_LIST;
		break;

		case TODO_TEST:
		case TODO_EXTRACT:
		arcd.OpenMode = PK_OM_EXTRACT;
		break;

		default:
		printf("%sUnknown TODO\n", prog_prefix);
		rc = ERR_OPEN_FAIL;
		goto stop;
	}
	if(!(arch = pOpenArchive(&arcd))) {
		if(verbose) printf("%sFailed: %s\n", prog_prefix, WCX_err_msg(arcd.OpenResult));
		else printf("%sFailed opening archive: %s\n", prog_prefix, WCX_err_msg(arcd.OpenResult));
		rc = ERR_OPEN_FAIL;
		goto stop;
	}
	if(verbose) printf("%sHandle returned by WCX: %X\n", prog_prefix, arch); fflush(stdout);

	if(pSetChangeVolProc) pSetChangeVolProc(arch, ChangeVol);
	if(pSetProcessDataProc) pSetProcessDataProc(arch, ProcessData);

	switch(open_todo) {
		case TODO_LIST:
		if(verbose) printf("%sList of files in %s\n", prog_prefix, arc_path);
		printf("%s Length    YYYY/MM/DD HH:MM:SS   Attr   Name\n", prog_prefix);
		printf("%s---------  ---------- --------  ------  ------------\n", prog_prefix);
		break;

		case TODO_TEST:
		if(verbose) printf("%sTesting files in %s\n", prog_prefix, arc_path);
		if(verbose) printf("%s--------\n", prog_prefix);
		break;

		case TODO_EXTRACT:
		if(verbose) printf("%sExtracting files from %s\n", prog_prefix, arc_path);
		if(verbose) printf("%s--------\n", prog_prefix);
		break;

		default:
		printf("%sUnknown TODO\n", prog_prefix);
		rc = ERR_OPEN_FAIL;
		goto stop;
	}

	/* main loop */
	while(!(rc = pReadHeader(arch, &hdrd))) {
		int		pfrc;

		CharToOem(hdrd.FileName, buf);
		switch(open_todo) {
			case TODO_LIST:
			printf("%s%9u  %04u/%02u/%02u %02u:%02u:%02u  %c%c%c%c%c%c	%s", prog_prefix, hdrd.UnpSize,
				((hdrd.FileTime >> 25 & 0x7f) + 1980), hdrd.FileTime >> 21 & 0x0f, hdrd.FileTime >> 16 & 0x1f,
				hdrd.FileTime >> 11 & 0x1f, hdrd.FileTime >>  5 & 0x3f, (hdrd.FileTime & 0x1F) * 2,
				hdrd.FileAttr & 0x01 ? 'r' : '-',
				hdrd.FileAttr & 0x02 ? 'h' : '-',
				hdrd.FileAttr & 0x04 ? 's' : '-',
				hdrd.FileAttr & 0x08 ? 'v' : '-',
				hdrd.FileAttr & 0x10 ? 'd' : '-',
				hdrd.FileAttr & 0x20 ? 'a' : '-',
				buf); fflush(stdout);
			pfrc = pProcessFile(arch, PK_SKIP, NULL, NULL);
			if(pfrc) {
				printf(" - Error! %s\n", WCX_err_msg(pfrc));
				goto stop;
			}
			else printf("\n");
			fflush(stdout);
			break;

			case TODO_TEST:
			if(!(hdrd.FileAttr & 0x10)) {
				printf("%s%s ", prog_prefix, buf);
				pfrc = pProcessFile(arch, PK_TEST, NULL, NULL);
				if(pfrc) {
					printf("Error! %s\n", WCX_err_msg(pfrc));
					goto stop;
				}
				else printf("Ok.\n");
				fflush(stdout);
			}
			else {
				pfrc = pProcessFile(arch, PK_SKIP, NULL, NULL);
			}
			break;

			case TODO_EXTRACT:
			if(!(hdrd.FileAttr & 0x10)) {
				char	outpath[MAX_PATH];

				sprintf(outpath, "%s%s", dir_path, hdrd.FileName);
				check_fndir(outpath);
				printf("%s%s ", prog_prefix, buf);
				pfrc = pProcessFile(arch, PK_EXTRACT, NULL, outpath);
				if(pfrc) {
					printf("\nError! %s (%s)\n", WCX_err_msg(pfrc), outpath);
					goto stop;
				}
				else printf("Ok.\n");
				fflush(stdout);
			}
			else {
				pfrc = pProcessFile(arch, PK_SKIP, NULL, NULL);
			}
			break;

			default:
			printf("%sUnknown TODO\n", prog_prefix); fflush(stdout);
			rc = ERR_OPEN_FAIL;
			goto stop;
		}
	}
	if(verbose) printf("%s--------\n", prog_prefix);
	if(verbose) printf("%s%s\n", prog_prefix, WCX_err_msg(rc)); fflush(stdout);
	if(rc == E_END_ARCHIVE) rc = 0;

	/* cleanup */
stop:
	if(arch) {
		if(verbose) {
			printf("%sClosing archive... ", prog_prefix);
			fflush(stdout);
		}
		if(pCloseArchive(arch)) {
			if(verbose) printf("Failed: %s\n", rc);
			else printf("Failed closing archive: %s\n", rc);
			fflush(stdout);
			rc = ERR_CLOSE_FAIL;
		} else {
			if(verbose) {
				printf("Ok.\n");
				fflush(stdout);
			}
			arch = NULL;
		}
	}

	if(hwcx) {
		if(verbose) {
			printf("%sUnloading plugin... ", prog_prefix);
			fflush(stdout);
		}
		if(!FreeLibrary(hwcx)) {
			if(verbose) printf("Failed.\n");
			else printf("Failed unloading plugin.\n");
			fflush(stdout);
			rc = ERR_NO_WCX;
		}
		else {
			if(verbose) {
				printf("Ok.\n");
				fflush(stdout);
			}
			hwcx = NULL;
		}
	}

//	  printf("Press Enter. ");
//	  gets(buf);

	if(verbose) {
		printf("%sERRORLEVEL: %d\n", prog_prefix, rc);
		fflush(stdout);
	}
	return rc;
}
