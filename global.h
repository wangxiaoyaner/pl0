#include<stdio.h>
#include<algorithm>
#include<ctype.h>
#include<string.h>
#include<string>
#include<iostream>
#include<stdlib.h>
#include<map>
#include<stack>
#include<sstream>
#include<queue>
#include<vector>
#include<list>
using namespace std;
#define GLOBAL_MAX_ERROR_NUM 100
#define LEX_W_NUM  32
#define LEX_MAX_LINE 4096//每行最多的字符数

typedef struct symbItem{
	string name;
	string kind;//var,const,array,procedure,function?
	string type;//char integer
	int if_used;
	int value;//char 类型怎么办，存ASCII？
	int level;
	int adr;
	int size;//func,proc,array
	int para_ifvar;
	int exist;//经过DAG荼毒还剩余的东西
	struct symbItem *link;
}symbItem;
typedef struct quadruple{
	string opr;
	symbItem *src1;
	symbItem *src2;
	symbItem *ans;
	struct quadruple *link;
}quadruple;

typedef struct symbTable{
	string name;
	int level;
	int localsnum;
	symbItem *first_item;
	symbItem *last_item;
	struct symbTable *father;
	struct symbTable *firstchild;
	struct symbTable *lastchild;
	struct symbTable *brother;
}symbTable;


typedef struct quadblock{
	int num;
	int subsnum[3];
	int subsn;
	quadruple *firstcode;
	quadruple *lastcode;
	struct quadblock *link;
	//int ifloop;
}quadblock;


typedef struct quadfunc{
	quadruple *firstcode;
	quadruple *lastcode;
	symbTable *table;
	quadblock **blocks;
	int blocknum;
	struct quadfunc *link;
}quadfunc;

extern quadfunc* quadruple_codes;
extern quadfunc* quadruple_codes_now;
extern symbTable *symbtable_table;
extern symbTable *symbtable_now;
symbItem* symbtable_check(string name);
int symbtable_find_dup(string name);
void symbtable_up_level();
void symbtable_new_level(string name);
int symbtable_enter(string name,string kind,string type,int value,int para_ifvar);
void symbtable_display();
int symbtable_if_can_change_func(string name);
extern stack<symbItem*> global_const_pool;
extern int global_lex_line_num;
extern int global_lex_lie_num;
extern int global_error_num;
void global_error(string right,string wrong);
void global_error(string words);
void global_init();

void global_new_quadruple(string opr,symbItem *src1,symbItem *src2,symbItem *ans);
void global_quadruple_display();
extern const string lex_words[];//直接判断，类型以lex_token命名。


extern FILE *sourcefile,*out;
extern string lex_token,lex_sym;
extern int single_quote_mark;
extern int lex_value,lex_line_no,lex_char_i,lex_mark;
extern char lex_ch;
extern char lex_line[];
void lex_getch();
int lex_getsym();

extern map<symbItem*,symbItem*>parser_arrvar_arr;
extern map<string,string> global_anti_ralation;
extern map<string,string> global_ralation;
string numtostring(int num);
void parser_program();

void optimazation_adr_alloc();
void generate_main();
extern int my_writes_num;
extern queue<string> my_write_string;
void codes_to_codes();
void basicblock_func_to_block();
void basicblock_print_all_blocks();
void bb_display_quad();
