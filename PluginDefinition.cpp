//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <iostream>
#include "Hyperlinks.h"
#include "pcre.h"
#define OVECCOUNT 60

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

HANDLE g_hMod;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	g_hMod=hModule;
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

	//--------------------------------------------//
	//-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
	//--------------------------------------------//
	// with function :
	// setCommand(int index,                      // zero based number to indicate the order of command
	//            TCHAR *commandName,             // the command name that you want to see in plugin menu
	//            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
	//            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
	//            bool check0nInit                // optional. Make this menu item be checked visually
	//            );
	ShortcutKey *sk=new ShortcutKey();
	sk->_isAlt=TRUE;
	sk->_isCtrl=TRUE;
	sk->_isShift=TRUE;
	sk->_key='D';
	setCommand(0, TEXT("Insert Doc String"), insert_doc_string, sk, false);
	setCommand(1, TEXT("About"), show_about_dlg, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
	if (index >= nbFunc)
		return false;

	if (!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}


std::string get_spaces(int sp_count)
{
	std::string sp;
	for(int i=0;i<sp_count;i++)
	{
		sp.append(" ");
	}
	return sp;
}

//boiler plate section
std::string gen_c_doc_string(char *func_string,int indentation) 
{
	pcre *re;
	pcre *re2;
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];
	int ovector2[OVECCOUNT];
	int rc,rc1;
	std::string doc_string;

	std::string indent_spaces=get_spaces(indentation);

	char *regex="(?P<ret_type>\\w+)(?P<pointer>\\s+|\\s+[*]|[*]\\s+)(?P<func_name>\\w+)\\s*\\((?P<func_args>.*)\\)";
	re = pcre_compile(
		regex, /* the pattern */
		0, /* default options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character table */

	rc=0;
	int offset=0;
	rc = pcre_exec(
		re, /* the compiled pattern */
		NULL, /* no extra data - we didn't study the pattern */
		func_string, /* the subject string */
		strlen(func_string), /* the length of the subject */
		offset, /* start at offset 0 in the subject */
		0, /* default options */
		ovector, /* output vector for substring information */
		OVECCOUNT); /* number of elements in the output
					vector */

	if (rc < 0)
	{
		doc_string.append(indent_spaces);
		doc_string.append("/*\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" */\r\n");
		return doc_string;
	}

	const char *ret=NULL;
	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"func_name",&ret);
	doc_string.append(indent_spaces);
	doc_string.append("/*\r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" * Name         : ");
	doc_string.append(ret);
	doc_string.append("\r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" *\r\n");

	pcre_free_substring(ret);

	doc_string.append(indent_spaces);
	doc_string.append(" * Synopsis     : ");
	pcre_get_substring(func_string,ovector,OVECCOUNT,0,&ret);
	doc_string.append(ret);
	doc_string.append("\r\n");
	pcre_free_substring(ret);
	
	doc_string.append(indent_spaces);
	doc_string.append(" *\r\n");


	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"func_args",&ret);

	//parse func_args and insert in each line
	char *regex2="(?P<arg_type>struct\\s+\\w+|const\\s+\\w+|\\w+)(?P<pointer>\\s+|\\s+[*]|[*]\\s+)(?P<arg_name>\\w+|\\(\\*(?P<p_func_name>\\w+)\\)(\\((?P<p_func_args>.*)\\)))";
	re2 = pcre_compile(
		regex2, /* the pattern */
		0, /* default options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character table */

	rc1=0;
	int offset1=0;
	rc1 = pcre_exec(
			re2, /* the compiled pattern */
			NULL, /* no extra data - we didn't study the pattern */
			ret, /* the subject string */
			strlen(ret), /* the length of the subject */
			offset1, /* start at offset 0 in the subject */
			0, /* default options */
			ovector2, /* output vector for substring information */
			OVECCOUNT); /* number of elements in the output
						vector */
	if(rc1>0){
		doc_string.append(indent_spaces);	
		doc_string.append(" * Arguments    : ");
		while(rc1!=-1){

			const char *ret1=NULL;
			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"arg_type",&ret1);
			doc_string.append(ret1);
			doc_string.append(" ");
			pcre_free_substring(ret1);

			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"pointer",&ret1);
			doc_string.append(ret1);
			pcre_free_substring(ret1);

			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"arg_name",&ret1);
			doc_string.append(ret1);
			doc_string.append(" : ");
			pcre_free_substring(ret1);
			offset1=ovector2[1];

			rc1 = pcre_exec(
				re2, /* the compiled pattern */
				NULL, /* no extra data - we didn't study the pattern */
				ret, /* the subject string */
				strlen(ret), /* the length of the subject */
				offset1, /* start at offset 0 in the subject */
				0, /* default options */
				ovector2, /* output vector for substring information */
				OVECCOUNT); /* number of elements in the output
							vector */
		 
			if (rc1 < 0 || rc < 3)
			{
				break;
			}
			doc_string.append("\r\n");
			doc_string.append(indent_spaces);
			doc_string.append(" *                ");
		}
		doc_string.append("\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
	}

	pcre_free_substring(ret);

	doc_string.append(indent_spaces);
	doc_string.append(" * Description  : \r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" * \r\n");
	doc_string.append(indent_spaces);
	
	//get return type
	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"ret_type",&ret);
	
	if(strcmp(ret,"void")!=0)
	{
		doc_string.append(" * Returns      : ");
		doc_string.append(ret);
		pcre_free_substring(ret);
		pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"pointer",&ret);
		doc_string.append(ret);
	}

	pcre_free_substring(ret);

	doc_string.append("\r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" */\r\n");

	pcre_free(re);
	pcre_free(re2);

	return doc_string;
}

void removeCRLF(std::string *str)
{
	
	for(int i=0;i<((int)str->length());i++)
	{
		if(str->at(i)=='\r' || str->at(i)=='\n')
		{
			str->replace(i,1,"\0");
		}
	}
}

std::string gen_cpp_doc_string(char *func_string,int indentation)
{
	pcre *re;
	pcre *re2;
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];
	int ovector2[OVECCOUNT];
	int rc,rc1;
	std::string doc_string;

	std::string indent_spaces=get_spaces(indentation);

	char *regex="(((?P<ret_type>\\w+)(?P<pointer>\\s+|\\s+[*]|[*]\\s+))|\\s*)((?P<func_name>\\w+)|(?P<cpp_method>(?P<namespace>\\w+)::(?P<method_name>\\w+)))\\s*\\((?P<func_args>[^\\)]*)\\)(\\s*:\\s*(?P<parent_const>.*)|\\s*)";
	re = pcre_compile(
		regex, /* the pattern */
		0, /* default options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character table */

	rc=0;
	int offset=0;
	rc = pcre_exec(
		re, /* the compiled pattern */
		NULL, /* no extra data - we didn't study the pattern */
		func_string, /* the subject string */
		strlen(func_string), /* the length of the subject */
		offset, /* start at offset 0 in the subject */
		0, /* default options */
		ovector, /* output vector for substring information */
		OVECCOUNT); /* number of elements in the output
					vector */

	if (rc < 0)
	{
		doc_string.append(indent_spaces);
		doc_string.append("/*\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" */\r\n");
		return doc_string;
	}

	const char *ret=NULL;
	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"func_name",&ret);
	if(strlen(ret)>0)
	{
		doc_string.append(indent_spaces);
		doc_string.append("/*\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" * Name         : ");
		doc_string.append(ret);
		doc_string.append("\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
	}
	pcre_free_substring(ret);

	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"method_name",&ret);
	if(strlen(ret)>0)
	{
		doc_string.append(indent_spaces);
		doc_string.append("/*\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" * Method Name  : ");
		doc_string.append(ret);
		doc_string.append("\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
	}
	pcre_free_substring(ret);



	doc_string.append(indent_spaces);
	doc_string.append(" * Synopsis     : ");
	pcre_get_substring(func_string,ovector,OVECCOUNT,0,&ret);
	std::string tmpStr(ret);
	removeCRLF(&tmpStr);
	doc_string.append(tmpStr);
	//doc_string.append("\n");
	pcre_free_substring(ret);
	
	doc_string.append(indent_spaces);
	doc_string.append(" *\r\n");


	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"func_args",&ret);

	//parse func_args and insert in each line
	char *regex2="(?P<arg_type>struct\\s+\\w+|const\\s+\\w+|\\w+)(?P<pointer>\\s+|\\s+[*]|[*]\\s+)(?P<arg_name>\\w+|\\(\\*(?P<p_func_name>\\w+)\\)(\\((?P<p_func_args>.*)\\)))";
	re2 = pcre_compile(
		regex2, /* the pattern */
		0, /* default options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character table */

	rc1=0;
	int offset1=0;
	rc1 = pcre_exec(
			re2, /* the compiled pattern */
			NULL, /* no extra data - we didn't study the pattern */
			ret, /* the subject string */
			strlen(ret), /* the length of the subject */
			offset1, /* start at offset 0 in the subject */
			0, /* default options */
			ovector2, /* output vector for substring information */
			OVECCOUNT); /* number of elements in the output
						vector */
	if(rc1>0){
		doc_string.append(indent_spaces);	
		doc_string.append(" * Arguments    : ");
		while(rc1!=-1){

			const char *ret1=NULL;
			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"arg_type",&ret1);
			doc_string.append(ret1);
			doc_string.append(" ");
			pcre_free_substring(ret1);

			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"pointer",&ret1);
			doc_string.append(ret1);
			pcre_free_substring(ret1);

			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"arg_name",&ret1);
			doc_string.append(ret1);
			doc_string.append(" : ");
			pcre_free_substring(ret1);
			offset1=ovector2[1];

			rc1 = pcre_exec(
				re2, /* the compiled pattern */
				NULL, /* no extra data - we didn't study the pattern */
				ret, /* the subject string */
				strlen(ret), /* the length of the subject */
				offset1, /* start at offset 0 in the subject */
				0, /* default options */
				ovector2, /* output vector for substring information */
				OVECCOUNT); /* number of elements in the output
							vector */
		 
			if (rc1 < 0 || rc < 3)
			{
				break;
			}
			doc_string.append("\r\n");
			doc_string.append(indent_spaces);
			doc_string.append(" *                ");
		}
		doc_string.append("\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
	}

	pcre_free_substring(ret);

	doc_string.append(indent_spaces);
	doc_string.append(" * Description  : \r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" * \r\n");
	
	//get return type
	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"ret_type",&ret);
	
	if(strcmp(ret,"void")!=0)
	{
		doc_string.append(indent_spaces);
		doc_string.append(" * Returns      : ");
		doc_string.append(ret);
		pcre_free_substring(ret);
		pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"pointer",&ret);
		doc_string.append(ret);
		doc_string.append("\r\n");
	}

	pcre_free_substring(ret);
	doc_string.append(indent_spaces);
	doc_string.append(" */\r\n");

	pcre_free(re);
	pcre_free(re2);

	return doc_string;
}


std::string gen_java_doc_string(char *func_string,int indentation)
{
	pcre *re;
	pcre *re2;
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];
	int ovector2[OVECCOUNT];
	int rc,rc1;
	std::string doc_string;

	std::string indent_spaces=get_spaces(indentation);

	char *regex="(?P<ret_type>\\w+)\\s+(?P<func_name>\\w+)\\s*\\((?P<func_args>.*)\\)(\\s+(?P<expts_block>throws\\s+(?P<exceptions>.*))|\\s*)";
	re = pcre_compile(
		regex, /* the pattern */
		NULL, /* default options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character table */

	rc=0;
	int offset=0;
	rc = pcre_exec(
		re, /* the compiled pattern */
		NULL, /* no extra data - we didn't study the pattern */
		func_string, /* the subject string */
		strlen(func_string), /* the length of the subject */
		offset, /* start at offset 0 in the subject */
		0, /* default options */
		ovector, /* output vector for substring information */
		OVECCOUNT); /* number of elements in the output
					vector */

	if (rc < 0)
	{
		doc_string.append(indent_spaces);
		doc_string.append("/**\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" *\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" */\r\n");
		return doc_string;
	}

	const char *ret=NULL;
	doc_string.append(indent_spaces);
	doc_string.append("/**\r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" * \r\n");
	doc_string.append(indent_spaces);
	doc_string.append(" * \r\n");
	

	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"func_args",&ret);

	//parse func_args and insert in each line
	char *regex2="(?P<arg_type>\\w+)\\s+(?P<arg_name>\\w+)";
	re2 = pcre_compile(
		regex2, /* the pattern */
		0, /* default options */
		&error, /* for error message */
		&erroffset, /* for error offset */
		NULL); /* use default character table */

	rc1=0;
	int offset1=0;
	rc1 = pcre_exec(
			re2, /* the compiled pattern */
			NULL, /* no extra data - we didn't study the pattern */
			ret, /* the subject string */
			strlen(ret), /* the length of the subject */
			offset1, /* start at offset 0 in the subject */
			0, /* default options */
			ovector2, /* output vector for substring information */
			OVECCOUNT); /* number of elements in the output
						vector */
	if(rc1>0){	
		while(rc1!=-1){

			const char *ret1=NULL;
			
			pcre_get_named_substring(re2,ret,ovector2,OVECCOUNT,"arg_name",&ret1);
			doc_string.append(indent_spaces);
			doc_string.append(" * @param ");
			doc_string.append(ret1);
			doc_string.append(" \r\n");
			pcre_free_substring(ret1);
			
			offset1=ovector2[1];

			rc1 = pcre_exec(
				re2, /* the compiled pattern */
				NULL, /* no extra data - we didn't study the pattern */
				ret, /* the subject string */
				strlen(ret), /* the length of the subject */
				offset1, /* start at offset 0 in the subject */
				0, /* default options */
				ovector2, /* output vector for substring information */
				OVECCOUNT); /* number of elements in the output
							vector */
		 
			if (rc1 < 0 || rc < 3)
			{
				break;
			}
		}
	}

	pcre_free_substring(ret);

	
	//get return type
	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"ret_type",&ret);
	
	if(strcmp(ret,"void")!=0)
	{
		doc_string.append(indent_spaces);
		doc_string.append(" * @return ");
		doc_string.append(ret);
		doc_string.append("\r\n");
	}
	pcre_free_substring(ret);

	//process exceptions block
	pcre_get_named_substring(re,func_string,ovector,OVECCOUNT,"exceptions",&ret);
	if(strlen(ret)>0)
	{
		//function throws exceptions, include in doc string
		char *tmp=new char[strlen(ret)];
		strcpy(tmp,ret);
		char *exception_name=strtok(tmp,",");
		while(exception_name!=NULL)
		{
			doc_string.append(indent_spaces);
			doc_string.append(" * @throws ");
			doc_string.append(exception_name);
			doc_string.append("\r\n");
			exception_name=strtok(NULL,",");
		}
	}

	doc_string.append(indent_spaces);
	doc_string.append(" */\r\n");

	pcre_free(re);
	pcre_free(re2);

	return doc_string;
}

std::string gen_doc_string_proxy(char *func_string,int indentation)
{
	enum LangType lang_type;
	::SendMessage(nppData._nppHandle,NPPM_GETCURRENTLANGTYPE,0,(LPARAM)&lang_type);

	switch(lang_type)
	{
		case L_C:
			return gen_c_doc_string(func_string,indentation);
		case L_CPP:
			return gen_cpp_doc_string(func_string,indentation);
		case L_JAVA:
			return gen_java_doc_string(func_string,indentation);
		default:
			return "";
	}
}



//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void insert_doc_string()
{
	// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
		return;
	HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	size_t len=::SendMessage(curScintilla,SCI_GETCURLINE,0,0);
	char *buf=new char[len];
	::SendMessage(curScintilla,SCI_GETCURLINE,len,(LPARAM)buf);

	int curPos=::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
	int curLine=::SendMessage(curScintilla,SCI_LINEFROMPOSITION,curPos,0);
	int indentation=::SendMessage(curScintilla,SCI_GETLINEINDENTATION,curLine,0);
	
	//goto first line
	::SendMessage(curScintilla,SCI_HOME,0,0);
	//parse the string and if a valid function generate a doc string.
	::SendMessage(curScintilla,SCI_INSERTTEXT,-1,(LPARAM)gen_doc_string_proxy(buf,indentation).c_str());	
	delete buf;
}



INT_PTR CALLBACK abtDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		ConvertStaticToHyperlink(hwndDlg,IDC_WEB);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg,wParam);
			return TRUE;
		case IDC_WEB:
			ShellExecute(hwndDlg, TEXT("open"),TEXT("http://nppdocit.sourceforge.net/"),NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
		}
	}
	return FALSE;
}

void show_about_dlg()
{
	::CreateDialog((HINSTANCE)g_hMod,MAKEINTRESOURCE(IDD_ABOUTDLG),nppData._nppHandle,abtDlgProc);
}
