// A simple program that uses LoadLibrary and 
// GetProcAddress to access myPuts from Myputs.dll. 
 
#include <windows.h> 
#include <stdio.h> 
#include <string>
#include <iostream>
#include <map>
#include <iterator>

enum LangType {L_TXT, L_PHP , L_C, L_CPP, L_CS, L_OBJC, L_JAVA, L_RC,\
	L_HTML, L_XML, L_MAKEFILE, L_PASCAL, L_BATCH, L_INI, L_NFO, L_USER,\
	L_ASP, L_SQL, L_VB, L_JS, L_CSS, L_PERL, L_PYTHON, L_LUA,\
	L_TEX, L_FORTRAN, L_BASH, L_FLASH, L_NSIS, L_TCL, L_LISP, L_SCHEME,\
	L_ASM, L_DIFF, L_PROPS, L_PS, L_RUBY, L_SMALLTALK, L_VHDL, L_KIX, L_AU3,\
	L_CAML, L_ADA, L_VERILOG, L_MATLAB, L_HASKELL, L_INNO, L_SEARCHRESULT,\
	L_CMAKE, L_YAML,\
	// The end of enumated language type, so it should be always at the end
	L_EXTERNAL};

typedef void (__cdecl *GEN_DOC_STRING)(char*,int,char **);
typedef char* (__cdecl *GET_TERMINATING_CHARACTER)();
typedef enum LangType (__cdecl *GET_LANG_TYPE)();

void testPlugin(LPCWSTR pluginFile)
{
	HINSTANCE hinstLib; 
    GEN_DOC_STRING ProcGenDocString; 
	GET_TERMINATING_CHARACTER ProcTermChar;
	GET_LANG_TYPE ProcLangType;
    BOOL fFreeResult, fRunTimeLinkSuccess = FALSE; 
	// Get a handle to the DLL module.
    hinstLib = LoadLibrary(pluginFile); 
	
 
    // If the handle is valid, try to get the function address.
 
    if (hinstLib != NULL) 
    { 
        ProcGenDocString = (GEN_DOC_STRING) GetProcAddress(hinstLib, "gen_doc_string"); 
		ProcTermChar=(GET_TERMINATING_CHARACTER) GetProcAddress(hinstLib,"get_terminating_character");
		ProcLangType=(GET_LANG_TYPE)GetProcAddress(hinstLib,"get_language_type");
        // If the function address is valid, call the function.
 
        if (NULL != ProcGenDocString) 
        {
            fRunTimeLinkSuccess = TRUE;
			char *temp=NULL;
			(ProcGenDocString)("void temp(int a,int b)",5,&temp); 
			printf("%s",temp);
			printf("%s",(ProcTermChar)());
			printf("%d",(ProcLangType)());
			delete temp;
        }
        // Free the DLL module.
 
        fFreeResult = FreeLibrary(hinstLib); 
    }
}

struct test
{
	int d;
};

int main( void ) 
{ 
	WIN32_FIND_DATA ffd;
	HANDLE hFind=INVALID_HANDLE_VALUE;
	hFind=FindFirstFile(TEXT(".\\*.dll"),&ffd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{

		do
		{
			testPlugin(ffd.cFileName);
		}while(FindNextFile(hFind,&ffd)!=0);
		
		FindClose(hFind);
	}
    
	getchar();
    return 0;
}

