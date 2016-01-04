#include"global.h"

quadfunc *quadruple_codes=NULL;
quadfunc *quadruple_codes_now=NULL;

stack<symbItem*> global_const_pool;
int global_lex_line_num=0;
int global_lex_lie_num=0;
int global_error_num=0;

void global_error(string right,string wrong)
{	
	cout << "error "<< ++global_error_num <<" : line[" << global_lex_line_num<<"," <<global_lex_lie_num<< "]   \"" << right <<"\" expected, but \""<<wrong<<"\" found\t\n";
}
void global_error(string words)
{
		cout << "error "<< ++global_error_num <<" : line[" << global_lex_line_num <<","<<global_lex_lie_num<< "]   " << words<<"\n";
}

void global_new_quadruple(string opr,symbItem *src1,symbItem *src2,symbItem *ans)
{	
	quadruple *newitem;
	newitem=new quadruple();
	newitem->opr=opr;
	newitem->src1=src1;
	newitem->src2=src2;
	newitem->ans=ans;
	newitem->link=NULL;
	//检查在后来被使用的局部变量.
	if(src1!=NULL)
	{
		src1->exist=0;
		if(src1->kind=="const"||src1->kind=="var")
			if(src1->level<symbtable_now->level)
				src1->if_used=1;
	}
	if(src2!=NULL)
	{
		src2->exist=0;
		if(src2->kind=="const"||src2->kind=="var")
			if(src2->level<symbtable_now->level)
				src2->if_used=1;
	}
	if(ans!=NULL)
	{
		ans->exist=0;
		if(ans->kind=="const"||ans->kind=="var")
			if(ans->level<symbtable_now->level)
				ans->if_used=1;
	}
	if(newitem->opr=="func")
	{
		quadfunc *newcodes=new quadfunc();
		newcodes->firstcode=newitem;
		newcodes->link=NULL;
		newcodes->lastcode=newitem;
		newcodes->table=symbtable_now;
		if(quadruple_codes==NULL)
		{
			quadruple_codes=newcodes;
			quadruple_codes_now=newcodes;
		}
		else
		{
			quadruple_codes_now->link=newcodes;
			quadruple_codes_now=quadruple_codes_now->link;
		}
	}
	else
	{
		quadruple_codes_now->lastcode->link=newitem;
		quadruple_codes_now->lastcode=quadruple_codes_now->lastcode->link;
	}
}
void global_quadruple_display()
{
	quadfunc *tmpfunc=quadruple_codes;
	while(tmpfunc!=NULL)
	{
		cout << "函数的名字是："<<tmpfunc->table->name<<"whh\t"<<tmpfunc->table->level<<" 函数内容如下:"<<endl;
		quadruple *tmp=tmpfunc->firstcode;
		while(tmp!=NULL)
		{
			cout << tmp->opr <<'\t';
			if(tmp->src1)
				cout<<tmp->src1->name<<'\t';
			if(tmp->src2)
				cout<<tmp->src2->name<<'\t';
			if(tmp->ans)
				cout<<tmp->ans->name;
			cout << endl;
			tmp=tmp->link;
		}
		tmpfunc=tmpfunc->link;
	}
}
void global_const_pool_del()
{
	symbItem *tmp;
	while(!global_const_pool.empty())
	{
		tmp=global_const_pool.top();
		global_const_pool.pop();
		delete tmp;
	}
}
map<string,string> global_anti_ralation;
map<string,string> global_ralation;
void global_init(){
	global_anti_ralation[">"]="jle";
	global_anti_ralation[">="]="jl";
	global_anti_ralation["<="]="jg";
	global_anti_ralation["<"]="jge";
	global_anti_ralation["="]="jne";
	global_anti_ralation["<>"]="je";

	global_ralation[">"]="jg";
	global_ralation[">="]="jge";
	global_ralation["<="]="jle";
	global_ralation["<"]="jl";
	global_ralation["="]="je";
	global_ralation["<>"]="jne";
}
