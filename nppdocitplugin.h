
typedef void (__cdecl *GEN_DOC_STRING)(char*,int,char **);
typedef char* (__cdecl *GET_TERMINATING_CHARACTER)();
typedef enum LangType (__cdecl *GET_LANG_TYPE)();

struct nppDocItPlugin
{
	HINSTANCE hinstLib;
	GEN_DOC_STRING gen_doc_string;
	GET_TERMINATING_CHARACTER get_terminating_character;
	GET_LANG_TYPE get_lang_type;
};