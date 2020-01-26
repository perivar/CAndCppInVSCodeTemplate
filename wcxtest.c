#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

#include "wcxhead.h"

#define PROG_LOGO                                                                                \
	"Total Commander WCX plugin Test ver. 0.24 (64 bit). Copyright (c) 2004-2006 Oleg Bondar.\n" \
	"Implements algorithm described in \"WCX Writer\'s Reference\" to test plugin's\n"           \
	"functionality.\n"

#define PROG_USAGE                                                                \
	"Usage:\n"                                                                    \
	"  wcxtest [-f | -l | | -t | -x] [-q] <wcx_path> [<arc_path>] [<dir_path>]\n" \
	"\n"                                                                          \
	"<wcx_path> - path to WCX plugin\n"                                           \
	"<arc_path> - path to archive file\n"                                         \
	"<dir_path> - directory to unpack files, default is current\n"                \
	"  -f - list WCX exported functions\n"                                        \
	"  -l - List archive contents (default)\n"                                    \
	"  -t - Test archive contents\n"                                              \
	"  -x - eXtract files from archive (overwrite existing)\n"                    \
	"  -?, -h - this topic\n"                                                     \
	"\n"                                                                          \
	"  -v - Verbose\n"                                                            \
	"\n"                                                                          \
	"ERRORLEVEL: 0 - success, non zero - some (m.b. unknown) error.\n"            \
	"\n"                                                                          \
	"Switches are NOT case sensitive. It\'s order - arbitrary.\n"                 \
	"Program NOT tested with file names that contains non-ASCII chars.\n"

#define TODO_FLIST 0
#define TODO_LIST 1
#define TODO_TEST 2
#define TODO_EXTRACT 3

#define ERR_NO_WCX 1
#define ERR_NO_PROC 2
#define ERR_OPEN_FAIL 3
#define ERR_CLOSE_FAIL 4

/* mandatory functions */
static HANDLE(__stdcall *pOpenArchiveW)(tOpenArchiveDataW *ArchiveData) = NULL;
static int(__stdcall *pReadHeaderExW)(HANDLE hArcData, tHeaderDataExW *HeaderData) = NULL;
static int(__stdcall *pProcessFileW)(HANDLE hArcData, int Operation, WCHAR *DestPath, WCHAR *DestName) = NULL;
static int(__stdcall *pCloseArchive)(HANDLE hArcData) = NULL;

/* optional functions */
static int(__stdcall *pPackFilesW)(WCHAR *PackedFile, WCHAR *SubPath, WCHAR *SrcPath, WCHAR *AddList, int Flags) = NULL;
static int(__stdcall *pDeleteFilesW)(WCHAR *PackedFile, WCHAR *DeleteList) = NULL;
static int(__stdcall *pGetPackerCaps)(void) = NULL;
static void(__stdcall *pConfigurePacker)(HWND Parent, HINSTANCE DllInstance);
static void(__stdcall *pSetChangeVolProcW)(HANDLE hArcData, tChangeVolProcW pChangeVolProc1) = NULL;	  /* NOT quite */
static void(__stdcall *pSetProcessDataProcW)(HANDLE hArcData, tProcessDataProcW pProcessDataProc) = NULL; /* NOT quite */

/* packing into memory */
static int(__stdcall *pStartMemPackW)(int Options, WCHAR *FileName) = NULL;
static int(__stdcall *pPackToMem)(int hMemPack, char *BufIn, int InLen, int *Taken,
								  char *BufOut, int OutLen, int *Written, int SeekBy) = NULL;
static int(__stdcall *pDoneMemPack)(int hMemPack) = NULL;

static BOOL(__stdcall *pCanYouHandleThisFileW)(WCHAR *FileName) = NULL;
static void(__stdcall *pPackSetDefaultParams)(PackDefaultParamStruct *dps) = NULL;

static WCHAR wcx_path[MAX_PATH] = L"";
static WCHAR arc_path[MAX_PATH] = L"";
static WCHAR dir_path[MAX_PATH] = L".\\";
static char prog_prefix[] = "WT: ";
static int open_todo = -1;
static int verbose = 0;

static char *
WCX_err_msg(int code)
{
	static char buf[256];

	switch (code)
	{
	case E_END_ARCHIVE:
		strcpy(buf, "No more files in archive");
		break;
	case E_NO_MEMORY:
		strcpy(buf, "Not enough memory");
		break;
	case E_BAD_DATA:
		strcpy(buf, "Data is bad");
		break;
	case E_BAD_ARCHIVE:
		strcpy(buf, "CRC error in archive data");
		break;
	case E_UNKNOWN_FORMAT:
		strcpy(buf, "Archive format unknown");
		break;
	case E_EOPEN:
		strcpy(buf, "Cannot open existing file");
		break;
	case E_ECREATE:
		strcpy(buf, "Cannot create file");
		break;
	case E_ECLOSE:
		strcpy(buf, "Error closing file");
		break;
	case E_EREAD:
		strcpy(buf, "Error reading from file");
		break;
	case E_EWRITE:
		strcpy(buf, "Error writing to file");
		break;
	case E_SMALL_BUF:
		strcpy(buf, "Buffer too small");
		break;
	case E_EABORTED:
		strcpy(buf, "Function aborted by user");
		break;
	case E_NO_FILES:
		strcpy(buf, "No files found");
		break;
	case E_TOO_MANY_FILES:
		strcpy(buf, "Too many files to pack");
		break;
	case E_NOT_SUPPORTED:
		strcpy(buf, "Function not supported");
		break;

	default:
		sprintf(buf, "Unknown error code (%d)", code);
		break;
	}

	return buf;
}

static int __stdcall ChangeVolW(WCHAR *ArcName, int Mode)
{
	char buf[32];
	int rc = 0;

	switch (Mode)
	{
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

static int __stdcall ProcessDataW(WCHAR *FileName, int Size)
{
	WCHAR buf[MAX_PATH];

	// CharToOem(FileName, buf);
	wcscpy(buf, FileName);

	printf("%sProcessed %ls (%d). Ok.\n", prog_prefix, buf, Size);
	fflush(stdout);

	// printf(".");
	return 1;
}

#define DIR_SEPARATOR L'\\'
#define DRV_SEPARATOR L':'

void check_fndir(WCHAR *fname)
{
	struct _stat sb;
	WCHAR *s;
	WCHAR buf[MAX_PATH];

	/* check if dir exists; if not create */
	for (s = fname; *s;)
	{
		if (*s == DIR_SEPARATOR)
			++s;
		while (*s && *s != DIR_SEPARATOR)
			++s;
		if (*s == DIR_SEPARATOR)
		{
			// *s = 0;
			*s = L'\0';

			// there is no difference in speed: check if exists directory
			// A return value of -1 indicates an error
			if (_wstat(fname, &sb) == -1)
			{
				// CharToOem(fname, buf);
				wcscpy(buf, fname);

				// don't create drives, e.g. C: or D:
				if (buf[2] != L'\0')
				{
					if (verbose)
						printf("%s-- Making dir %ls\n", prog_prefix, buf);

					_wmkdir(fname);
				}
			}

			*s = DIR_SEPARATOR;
		}
	}
}

extern int
main(int argc, char *argv[])
{
	int i, j, rc = 0;
	char *s;
	WCHAR buf[MAX_PATH];
	HINSTANCE hwcx = NULL;
	HANDLE arch = NULL;
	tOpenArchiveDataW arcd;
	tHeaderDataExW hdrd;

	if (argc < 2)
	{
		printf(PROG_LOGO);
		printf(PROG_USAGE);
		return 0;
	}

	if (argc == 2)
		open_todo = TODO_FLIST;

	/* switches */
	for (i = 1; i < argc; ++i)
	{
		s = argv[i];
		if (*s != '-' && *s != '/')
			continue;
		++s;
		switch (*s)
		{
		case 'f':
		case 'F': /* list of functions mode */
			if (open_todo < 0)
				open_todo = TODO_FLIST;
			else
			{
				printf("Syntax error: too many switches.\n");
				printf(PROG_USAGE);
				return 0;
			}
			break;

		case 'l':
		case 'L': /* list mode */
			if (open_todo < 0)
				open_todo = TODO_LIST;
			else
			{
				printf("Syntax error: too many switches.\n");
				printf(PROG_USAGE);
				return 0;
			}
			break;

		case 't':
		case 'T': /* test mode */
			if (open_todo < 0)
				open_todo = TODO_TEST;
			else
			{
				printf("Syntax error: too many switches.\n");
				printf(PROG_USAGE);
				return 0;
			}
			break;

		case 'x':
		case 'X': /* extract mode */
			if (open_todo < 0)
				open_todo = TODO_EXTRACT;
			else
			{
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
	if (open_todo < 0)
		open_todo = TODO_LIST;
	if (!verbose)
		*prog_prefix = 0;

	/* parameters */
	for (i = 1, j = 0; i < argc; ++i)
	{
		s = argv[i];
		if (*s == '-' || *s == '/')
			continue;
		switch (j)
		{
		case 0:
			// strcpy(wcx_path, argv[i]);
			mbstowcs(wcx_path, argv[i], strlen(argv[i]) + 1);
			break;

		case 1:
			// strcpy(arc_path, argv[i]);
			mbstowcs(arc_path, argv[i], strlen(argv[i]) + 1);
			break;

		case 2:
			// strcpy(dir_path, argv[i]);
			mbstowcs(dir_path, argv[i], strlen(argv[i]) + 1);
			break;

		default:
			printf("Syntax error: too many arguments.\n");
			printf(PROG_USAGE);
			return 0;
		}
		++j;
	}

	if (!*wcx_path)
	{
		printf("Syntax error: no WCX name.\n");
		printf(PROG_USAGE);
		return 0;
	}
	if (!*arc_path && (open_todo == TODO_LIST || open_todo == TODO_TEST || open_todo == TODO_EXTRACT))
	{
		printf("Syntax error: no archive name.\n");
		printf(PROG_USAGE);
		return 0;
	}
	if (*dir_path && dir_path[wcslen(dir_path) - 1] != L'\\')
		wcscat(dir_path, L"\\");

	if (verbose)
	{
		switch (open_todo)
		{
		case TODO_FLIST:
			// printf("%sExported functions in \"%ls\":\n", prog_prefix, wcx_path);
			break;

		case TODO_LIST:
			printf("%sUsing \"%ls\" for list files in \"%ls\".\n", prog_prefix, wcx_path, arc_path);
			break;

		case TODO_TEST:
			printf("%sUsing \"%ls\" for test files in \"%ls\".\n", prog_prefix, wcx_path, arc_path);
			break;

		case TODO_EXTRACT:
			printf("%sUsing \"%ls\" for extract files from \"%ls\" to \"%ls\".\n", prog_prefix, wcx_path, arc_path, dir_path);
			break;

		default:
			printf("unknown to do with");
			break;
		}
	}

	if (verbose)
		printf("%sLoading plugin %ls... ", prog_prefix, wcx_path);

	if (!(hwcx = LoadLibraryW(wcx_path)))
	{
		if (verbose)
			printf("Failed.\n");
		else
			printf("Failed loading plugin.\n");
		return ERR_NO_WCX;
	}
	if (verbose)
		printf("Ok.\n");

	/* mandatory */
	pOpenArchiveW = (void *)GetProcAddress(hwcx, "OpenArchiveW");
	pReadHeaderExW = (void *)GetProcAddress(hwcx, "ReadHeaderExW");
	pProcessFileW = (void *)GetProcAddress(hwcx, "ProcessFileW");
	pCloseArchive = (void *)GetProcAddress(hwcx, "CloseArchive");
	/* optional */
	pPackFilesW = (void *)GetProcAddress(hwcx, "PackFilesW");
	pDeleteFilesW = (void *)GetProcAddress(hwcx, "DeleteFilesW");
	pGetPackerCaps = (void *)GetProcAddress(hwcx, "GetPackerCaps");
	pConfigurePacker = (void *)GetProcAddress(hwcx, "ConfigurePacker");
	/* NOT optional */
	pSetChangeVolProcW = (void *)GetProcAddress(hwcx, "SetChangeVolProcW");
	pSetProcessDataProcW = (void *)GetProcAddress(hwcx, "SetProcessDataProcW");
	/* optional */
	pStartMemPackW = (void *)GetProcAddress(hwcx, "StartMemPackW");
	pPackToMem = (void *)GetProcAddress(hwcx, "PackToMem");
	pDoneMemPack = (void *)GetProcAddress(hwcx, "DoneMemPack");
	pCanYouHandleThisFileW = (void *)GetProcAddress(hwcx, "CanYouHandleThisFileW");
	pPackSetDefaultParams = (void *)GetProcAddress(hwcx, "PackSetDefaultParams");

	if (open_todo == TODO_FLIST)
	{
		printf("%sExported WCX functions in \"%ls\":\n", prog_prefix, wcx_path);
		if (pOpenArchiveW)
			printf("%s  OpenArchiveW\n", prog_prefix);
		if (pReadHeaderExW)
			printf("%s  ReadHeaderExW\n", prog_prefix);
		if (pProcessFileW)
			printf("%s  ProcessFileW\n", prog_prefix);
		if (pCloseArchive)
			printf("%s  CloseArchive\n", prog_prefix);
		if (pPackFilesW)
			printf("%s  PackFilesW\n", prog_prefix);
		if (pDeleteFilesW)
			printf("%s  DeleteFilesW\n", prog_prefix);
		if (pGetPackerCaps)
			printf("%s  GetPackerCaps\n", prog_prefix);
		if (pConfigurePacker)
			printf("%s  ConfigurePacker\n", prog_prefix);
		if (pSetChangeVolProcW)
			printf("%s  SetChangeVolProcW\n", prog_prefix);
		if (pSetProcessDataProcW)
			printf("%s  SetProcessDataProcW\n", prog_prefix);
		if (pStartMemPackW)
			printf("%s  StartMemPackW\n", prog_prefix);
		if (pPackToMem)
			printf("%s  PackToMem\n", prog_prefix);
		if (pDoneMemPack)
			printf("%s  DoneMemPack\n", prog_prefix);
		if (pCanYouHandleThisFileW)
			printf("%s  CanYouHandleThisFileW\n", prog_prefix);
		if (pPackSetDefaultParams)
			printf("%s  PackSetDefaultParams\n", prog_prefix);

		if (pGetPackerCaps)
		{
			int pc = pGetPackerCaps(), f = 0;

			printf("%sPackerCaps: %u =", prog_prefix, pc);
			if (pc & PK_CAPS_NEW)
			{
				printf("%s PK_CAPS_NEW", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_MODIFY)
			{
				printf("%s PK_CAPS_MODIFY", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_MULTIPLE)
			{
				printf("%s PK_CAPS_MULTIPLE", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_DELETE)
			{
				printf("%s PK_CAPS_DELETE", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_OPTIONS)
			{
				printf("%s PK_CAPS_OPTIONS", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_MEMPACK)
			{
				printf("%s PK_CAPS_MEMPACK", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_BY_CONTENT)
			{
				printf("%s PK_CAPS_BY_CONTENT", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_SEARCHTEXT)
			{
				printf("%s PK_CAPS_SEARCHTEXT", f ? " |" : "");
				f = 1;
			}
			if (pc & PK_CAPS_HIDE)
			{
				printf("%s PK_CAPS_HIDE", f ? " |" : "");
				f = 1;
			}
			printf("\n");
		}
		goto stop;
	}

	if (!pOpenArchiveW || !pReadHeaderExW || !pProcessFileW || !pCloseArchive)
	{
		printf("%sError: There IS NOT mandatory function(s):", prog_prefix);
		if (!pOpenArchiveW)
			printf(" OpenArchiveW");
		if (!pReadHeaderExW)
			printf(" ReadHeaderExW");
		if (!pProcessFileW)
			printf(" ProcessFileW");
		if (!pCloseArchive)
			printf(" CloseArchive");
		printf("\n");
		rc = ERR_NO_PROC;
		goto stop;
	}
	if (!pSetChangeVolProcW || !pSetProcessDataProcW)
	{
		printf("%sError: There IS NOT NOT-optional function(s):", prog_prefix);
		if (!pSetChangeVolProcW)
			printf(" SetChangeVolProcW");
		if (!pSetProcessDataProcW)
			printf(" SetProcessDataProcW");
		printf("\n");
		rc = ERR_NO_PROC;
		goto stop;
	}

	if (verbose)
		printf("%sOpening archive %ls...\n", prog_prefix, arc_path);
	memset(&arcd, 0, sizeof(arcd));
	arcd.ArcName = arc_path;
	switch (open_todo)
	{
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
	if (!(arch = pOpenArchiveW(&arcd)))
	{
		if (verbose)
			printf("%sFailed: %s\n", prog_prefix, WCX_err_msg(arcd.OpenResult));
		else
			printf("%sFailed opening archive: %s\n", prog_prefix, WCX_err_msg(arcd.OpenResult));
		rc = ERR_OPEN_FAIL;
		goto stop;
	}
	if (verbose)
		printf("%sHandle returned by WCX: %X\n", prog_prefix, arch);
	fflush(stdout);

	if (pSetChangeVolProcW)
		pSetChangeVolProcW(arch, ChangeVolW);
	if (pSetProcessDataProcW)
		pSetProcessDataProcW(arch, ProcessDataW);

	switch (open_todo)
	{
	case TODO_LIST:
		if (verbose)
			printf("%sList of files in %ls\n", prog_prefix, arc_path);
		printf("%s Length    YYYY/MM/DD HH:MM:SS   Attr   Name\n", prog_prefix);
		printf("%s---------  ---------- --------  ------  ------------\n", prog_prefix);
		break;

	case TODO_TEST:
		if (verbose)
			printf("%sTesting files in %ls\n", prog_prefix, arc_path);
		if (verbose)
			printf("%s--------\n", prog_prefix);
		break;

	case TODO_EXTRACT:
		if (verbose)
			printf("%sExtracting files from %ls\n", prog_prefix, arc_path);
		if (verbose)
			printf("%s--------\n", prog_prefix);
		break;

	default:
		printf("%sUnknown TODO\n", prog_prefix);
		rc = ERR_OPEN_FAIL;
		goto stop;
	}

	/* main loop */
	while (!(rc = pReadHeaderExW(arch, &hdrd)))
	{
		int pfrc;

		// CharToOem(hdrd.FileName, buf);
		wcscpy(buf, hdrd.FileName);

		switch (open_todo)
		{
		case TODO_LIST:
			printf("%s%9u  %04u/%02u/%02u %02u:%02u:%02u  %c%c%c%c%c%c	%ls", prog_prefix, hdrd.UnpSize,
				   ((hdrd.FileTime >> 25 & 0x7f) + 1980), hdrd.FileTime >> 21 & 0x0f, hdrd.FileTime >> 16 & 0x1f,
				   hdrd.FileTime >> 11 & 0x1f, hdrd.FileTime >> 5 & 0x3f, (hdrd.FileTime & 0x1F) * 2,
				   hdrd.FileAttr & 0x01 ? 'r' : '-',
				   hdrd.FileAttr & 0x02 ? 'h' : '-',
				   hdrd.FileAttr & 0x04 ? 's' : '-',
				   hdrd.FileAttr & 0x08 ? 'v' : '-',
				   hdrd.FileAttr & 0x10 ? 'd' : '-',
				   hdrd.FileAttr & 0x20 ? 'a' : '-',
				   buf);
			fflush(stdout);
			pfrc = pProcessFileW(arch, PK_SKIP, NULL, NULL);
			if (pfrc)
			{
				printf(" - Error! %s\n", WCX_err_msg(pfrc));
				goto stop;
			}
			else
				printf("\n");
			fflush(stdout);
			break;

		case TODO_TEST:
			// if not directory
			if (!(hdrd.FileAttr & 0x10))
			{
				printf("%s%ls ", prog_prefix, buf);
				pfrc = pProcessFileW(arch, PK_TEST, NULL, NULL);
				if (pfrc)
				{
					printf("Error! %s\n", WCX_err_msg(pfrc));
					goto stop;
				}
				else
					printf("Ok.\n");
				fflush(stdout);
			}
			else
			{
				pfrc = pProcessFileW(arch, PK_SKIP, NULL, NULL);
			}
			break;

		case TODO_EXTRACT:
			// if not directory
			if (!(hdrd.FileAttr & 0x10))
			{
				WCHAR outpath[MAX_PATH];
				swprintf(outpath, MAX_PATH, L"%ls%ls", dir_path, hdrd.FileName);
				check_fndir(outpath);
				printf("%s%ls ", prog_prefix, buf);
				pfrc = pProcessFileW(arch, PK_EXTRACT, NULL, outpath);
				if (pfrc)
				{
					printf("\nError! %s (%ls)\n", WCX_err_msg(pfrc), outpath);
					goto stop;
				}
				else
					printf("Ok.\n");
				fflush(stdout);
			}
			else
			{
				pfrc = pProcessFileW(arch, PK_SKIP, NULL, NULL);
			}
			break;

		default:
			printf("%sUnknown TODO\n", prog_prefix);
			fflush(stdout);
			rc = ERR_OPEN_FAIL;
			goto stop;
		}
	}
	if (verbose)
		printf("%s--------\n", prog_prefix);
	if (verbose)
		printf("%s%s\n", prog_prefix, WCX_err_msg(rc));
	fflush(stdout);
	if (rc == E_END_ARCHIVE)
		rc = 0;

	/* cleanup */
stop:
	if (arch)
	{
		if (verbose)
		{
			printf("%sClosing archive... ", prog_prefix);
			fflush(stdout);
		}
		if (pCloseArchive(arch))
		{
			if (verbose)
				printf("Failed: %x\n", rc);
			else
				printf("Failed closing archive: %x\n", rc);
			fflush(stdout);
			rc = ERR_CLOSE_FAIL;
		}
		else
		{
			if (verbose)
			{
				printf("Ok.\n");
				fflush(stdout);
			}
			arch = NULL;
		}
	}

	if (hwcx)
	{
		if (verbose)
		{
			printf("%sUnloading plugin... ", prog_prefix);
			fflush(stdout);
		}

		// close plugin window?
		// PostMessage(hwcx, WM_CLOSE, 0, 0);

		// if (!FreeLibrary(hwcx))
		// {
		// 	if (verbose)
		// 		printf("Failed.\n");
		// 	else
		// 		printf("Failed unloading plugin.\n");

		// 	fflush(stdout);
		// 	rc = ERR_NO_WCX;
		// }
		// else
		// {
		// 	if (verbose)
		// 	{
		// 		printf("Ok.\n");
		// 		fflush(stdout);
		// 	}
		// 	hwcx = NULL;
		// }
	}

	printf("Press Enter. ");
	static char readBuf[256];
	gets(readBuf);

	if (verbose)
	{
		printf("%sERRORLEVEL: %d\n", prog_prefix, rc);
		fflush(stdout);
	}
	return rc;
}
