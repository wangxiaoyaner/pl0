//语法分析程序不关心对错。
#include"global.h"
const string lex_words[]={"(", ")", "*", "+", ",", "-", ".", "/", ";", "=", "[", "]", "array", "begin", "char", "const", "do", "downto", "else", "end", "for", "function", "if", "integer", "of", "procedure", "read", "then", "to", "var", "while", "write"};//直接判断，类型以lex_token命名。
FILE *sourcefile,*out;
string lex_token="",lex_sym="";
int single_quote_mark=0;
int lex_value=0,lex_line_no=0,lex_char_i=0,lex_mark=0;
char lex_ch;
char lex_line[LEX_MAX_LINE]="\n";
void lex_bye()
{
	lex_token="";
	lex_sym="";
}
int lex_find_words(string goal)
{
	int low=0,high=LEX_W_NUM,mid;
	while(low<=high)
	{
		mid=(high+low)/2;
		if(goal==lex_words[mid])
			return 1;
		else if(goal<lex_words[mid])
			high=mid-1;
		else
			low=mid+1;
	}
	return 0;
}

void lex_getch()
{
	if(lex_char_i!=-1&&lex_line[lex_char_i]=='\n'/*||lex_line[++lex_char_i]=='\n'*/)
	{
		if(fgets(lex_line,LEX_MAX_LINE,sourcefile)==NULL)
		{
			lex_ch=0;//end
			lex_mark=1;//file end mark
			fclose(sourcefile);
			return;
		}
		lex_line_no++;//
		lex_char_i=0;
	}
	else
		lex_char_i++;
	lex_ch=lex_line[lex_char_i];
}

int lex_getsym()//lex_getch first
{
	while(1)
	{
		if(!lex_ch)
		{
			lex_bye();
			return 0;
		}
		while(isspace(lex_ch))
		{
			if(lex_ch=='\n')
			{
				if(fgets(lex_line,LEX_MAX_LINE,sourcefile)==NULL)
				{
					lex_ch=0;//end
					lex_mark=1;//file end mark
				}
				else
				{
					lex_line_no++;//
					lex_char_i=-1;
				}
			}
			lex_getch();
		}
		if(isalpha(lex_ch))
		{
			lex_token="";
			lex_token+=lex_ch;
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			while(isalpha(lex_ch)||isdigit(lex_ch))
			{
				lex_token+=lex_ch;
				lex_getch();
				if(!lex_char_i)
					break;
			}
			if(lex_find_words(lex_token))
			{
				lex_sym=lex_token;
			}
			else
				lex_sym="ident";
			return 1;
		}
		else if(isdigit(lex_ch))
		{
			lex_token="";
			lex_token+=lex_ch;
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			while(isdigit(lex_ch))
			{
				lex_token+=lex_ch;
				lex_getch();
			}
			lex_sym="digit";
			sscanf(lex_token.data(),"%d",&lex_value);
			return 1;
		}	
		else if(lex_ch=='<')//<,<>,<=
		{
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			if(lex_ch=='>')
			{
				lex_sym="<>";
				lex_getch();
			}
			else if(lex_ch=='=')
			{
				lex_sym="<=";
				lex_getch();
			}
			else
				lex_sym="<";
			return 1;
		}
		else if(lex_ch==':')
		{
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			if(lex_ch=='=')
			{
				lex_sym=":=";
				lex_getch();
			}
			else
				lex_sym=":";
			return 1;
		}
		else if(lex_ch=='>')
		{
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			if(lex_ch=='=')
			{
				lex_sym=">=";
				lex_getch();
			}
			else
				lex_sym=">";
			return 1;
		}
		else if(lex_ch=='\'')
		{
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			lex_token="";
			lex_value=lex_ch;
			while(lex_ch!='\''&& lex_char_i)
			{
				if(lex_mark)
				{
					lex_bye();
					global_error("\'",""+lex_ch);//缺引号
					return 0;
				}
				if(!isalpha(lex_ch)&&!isdigit(lex_ch))
				{
					global_error("a~zA_Z0-9",(""+lex_ch));//非法字符
				}	
				lex_token+=lex_ch;
				lex_getch();
			}
			if(!lex_char_i&&lex_ch!='\'')
			{
				global_error("\'",""+lex_ch);//字符常量定义错误
			}
			if(lex_char_i&&lex_token.size()!=1)
			{
				global_error("single char",lex_token);//字符常量定义错误
			}
			lex_sym="char";
			if(lex_ch=='\'')
				lex_getch();
			else
				{
					global_error("legal charactor","illegal charactor");
				}
			return 1;
		}
		else if(lex_ch=='"')
		{
			global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
			lex_getch();
			lex_token="";
			lex_sym="string";
			while(lex_ch==32||lex_ch==33||(lex_ch>=35&&lex_ch<=126))
			{
				lex_token+=lex_ch;
				lex_getch();
				if(!lex_char_i)
					break;
			}
			if(lex_ch=='"')
				lex_getch();
			else
			{
				string ttt="";
				if(isspace(lex_ch))
					ttt="space";
				else ttt+=lex_ch;
				global_error("\"",ttt);
			}
			return 1;
		}
		else if(lex_ch=='{')
		{
			lex_getch();
			while(lex_ch!='}'&&!lex_mark)
				lex_getch();
			if(lex_ch=='}')
				lex_getch();
			if(lex_mark)
				return 0;
		}
		else
		{
			if(lex_mark)
			{
				lex_bye();
				return 0;
			}
			lex_sym="";
			lex_sym+=lex_ch;
			if(lex_find_words(lex_sym))
			{
				global_lex_line_num=lex_line_no;global_lex_lie_num=lex_char_i;
				lex_getch();
				return 1;//一眼能判别的分解符
			}
			else
			{
				global_error("legal charater","illegal character");
				lex_getch();
			}
		}
	}
	lex_bye();
	return 0;
}

