#ifndef COMMON_UTIL_SINGLETON
#define COMMON_UTIL_SINGLETON

namespace cppnet {

template<typename T>
class Singleton {
public:
    static T& Instance() {
        static T instance;
        return instance;
    }

protected:
    Singleton(const Singleton&) {}
    Singleton& operator = (const Singleton&) {}
    Singleton() {}
    virtual ~Singleton() {}
};

}
#endif



/*
static CHAR s_szFile[MAX_PATH] = "\0";

static BOOL CALLBACK ListFileCallback(PVOID pContext,
	PCHAR pszOrigFile,
	PCHAR pszFile,
	PCHAR *ppszOutFile)
{
	std::cout << "pszOrigFile" << pszOrigFile << std::endl;
	std::cout << "pszFile" << pszFile << std::endl;
	std::cout << "ppszOutFile" << ppszOutFile << std::endl;
	return TRUE;
}

BOOL CALLBACK ListSymbolCallback(PVOID pContext,
	ULONG nOrigOrdinal,
	ULONG nOrdinal,
	ULONG *pnOutOrdinal,
	PCHAR pszOrigSymbol,
	PCHAR pszSymbol,
	PCHAR *ppszOutSymbol)
{
	(void)pContext;
	(void)nOrdinal;
	(void)pszSymbol;

	*ppszOutSymbol = NULL;
	*pnOutOrdinal = 0;

	if (nOrigOrdinal != 0) {
		printf("  %s::#%ld\n",
			s_szFile, nOrigOrdinal);
	}
	else {
		printf("  %s::%s\n",
			s_szFile, pszOrigSymbol);
	}

	return TRUE;
}

BOOL DimpFile(PCHAR pszPath)
{
	BOOL bGood = TRUE;
	HANDLE hOld = INVALID_HANDLE_VALUE;
	PDETOUR_BINARY pBinary = NULL;


	hOld = CreateFileA(pszPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hOld == INVALID_HANDLE_VALUE) {
		printf("%s: Failed to open input file with error: %ld\n",
			pszPath, GetLastError());
		bGood = FALSE;
		goto end;
	}

	if ((pBinary = DetourBinaryOpen(hOld)) == NULL) {
		printf("%s: DetourBinaryOpen failed: %ld\n", pszPath, GetLastError());
		goto end;
	}

	if (hOld != INVALID_HANDLE_VALUE) {
		CloseHandle(hOld);
		hOld = INVALID_HANDLE_VALUE;
	}

	printf("%s:\n", pszPath);
	if (!DetourBinaryEditImports(pBinary,
		NULL,
		NULL,
		ListFileCallback,
		ListSymbolCallback,
		NULL)) {

		printf("%s: DetourBinaryEditImports failed: %ld\n", pszPath, GetLastError());
	}

	DetourBinaryClose(pBinary);
	pBinary = NULL;

end:
	if (pBinary) {
		DetourBinaryClose(pBinary);
		pBinary = NULL;
	}
	if (hOld != INVALID_HANDLE_VALUE) {
		CloseHandle(hOld);
		hOld = INVALID_HANDLE_VALUE;
	}
	return bGood;
}
*/