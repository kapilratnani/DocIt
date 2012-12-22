//Copyright (C)2011 Kapil Ratnani <kapil.ratnani@iiitb.net>
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

//Based on phpdoc's format <http://phpdoc.org>


#define PCRE_STATIC
#include"include\pcre.h"
#define OVECCOUNT 60
#include<string>


enum LangType {L_TXT, L_PHP , L_C, L_CPP, L_CS, L_OBJC, L_JAVA, L_RC,\
	L_HTML, L_XML, L_MAKEFILE, L_PASCAL, L_BATCH, L_INI, L_NFO, L_USER,\
	L_ASP, L_SQL, L_VB, L_JS, L_CSS, L_PERL, L_PYTHON, L_LUA,\
	L_TEX, L_FORTRAN, L_BASH, L_FLASH, L_NSIS, L_TCL, L_LISP, L_SCHEME,\
	L_ASM, L_DIFF, L_PROPS, L_PS, L_RUBY, L_SMALLTALK, L_VHDL, L_KIX, L_AU3,\
	L_CAML, L_ADA, L_VERILOG, L_MATLAB, L_HASKELL, L_INNO, L_SEARCHRESULT,\
	L_CMAKE, L_YAML,\
	// The end of enumated language type, so it should be always at the end
	L_EXTERNAL};

std::string get_spaces(int sp_count)
{
	std::string sp;
	for(int i=0;i<sp_count;i++)
	{
		sp.append(" ");
	}
	return sp;
}

void trim(std::string *str)
{
	for(int i=0;i<(int)str->length();i++)
	{
		if(str->at(i)==' ')
			str->erase(i,1);
		else
			break;
	}

	for(int i=str->length()-1;i>=0;i--)
	{
		if(str->at(i)==' ')
			str->erase(i,1);
		else
			break;
	}
}

extern "C"
{
	__declspec(dllexport) void init()
	{
		//TODO: write some code here :P
		//compile regex or do any other initialization 
	}

	__declspec(dllexport) void cleanup()
	{
		//TODO: write some code here :P
		//free memory
	}

	__declspec(dllexport) void gen_doc_string(char *func_string,int indentation,char** out)
	{
		pcre *re_function;
		pcre *re_function_args;

		const char *error;
		int erroffset;
		int ovector[OVECCOUNT];
		int ovector2[OVECCOUNT];
		int rc,rc1;
		std::string doc_string;

		std::string indent_spaces=get_spaces(indentation);

		char *regex="function\\s+(?P<func_name>\\w+)\\s*\\((?P<func_args>.*)\\)";


		re_function = pcre_compile(
			regex, /* the pattern */
			NULL, /* default options */
			&error, /* for error message */
			&erroffset, /* for error offset */
			NULL); /* use default character table */

		rc=0;
		int offset=0;
		rc = pcre_exec(
			re_function, /* the compiled pattern */
			NULL, /* no extra data - we didn't study the pattern */
			func_string, /* the subject string */
			strlen(func_string), /* the length of the subject */
			offset, /* start at offset 0 in the subject */
			0, /* default options */
			ovector, /* output vector for substring information */
			OVECCOUNT); /* number of elements in the output
						vector */

		//return empty comment block if nothing matches
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

			*out=new char[doc_string.size()+1];
			strncpy_s(*out,doc_string.size()+1,doc_string.c_str(),doc_string.size()+1);
			return;
		}

		const char *ret=NULL;
		doc_string.append(indent_spaces);
		doc_string.append("/**\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" * \r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" * \r\n");


		pcre_get_named_substring(re_function,func_string,ovector,OVECCOUNT,"func_args",&ret);

		//parse func_args and insert in each line
		char *regex2="\\$(?P<arg_name>[^,=]*)";
		re_function_args = pcre_compile(
			regex2, /* the pattern */
			0, /* default options */
			&error, /* for error message */
			&erroffset, /* for error offset */
			NULL); /* use default character table */

		rc1=0;
		int offset1=0;
		rc1 = pcre_exec(
			re_function_args, /* the compiled pattern */
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

				pcre_get_named_substring(re_function_args,ret,ovector2,OVECCOUNT,"arg_name",&ret1);
				doc_string.append(indent_spaces);
				doc_string.append(" * @param <type> ");
				//append $ before parameter
				doc_string.append("$");
				doc_string.append(ret1);
				doc_string.append(" \r\n");
				pcre_free_substring(ret1);

				offset1=ovector2[1];

				rc1 = pcre_exec(
					re_function_args, /* the compiled pattern */
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

		doc_string.append(indent_spaces);
		doc_string.append(" * \r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" * @return <type>\r\n");
		doc_string.append(indent_spaces);
		doc_string.append(" */\r\n");

		pcre_free(re_function);
		pcre_free(re_function_args);
		*out=new char[doc_string.size()+1];
		strncpy_s(*out,doc_string.size()+1,doc_string.c_str(),doc_string.size()+1);
		return;
	}

	__declspec(dllexport) const char* get_terminating_character()
	{
		return "[;{]";
	}

	__declspec(dllexport) enum LangType get_language_type()
	{
		return L_PHP;
	}
}
