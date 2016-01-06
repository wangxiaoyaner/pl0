#include"global.h"
//#define WIN_FORM
typedef struct tmpreginfo{
	int id;
	int ifuse;
	int write_back;
	symbItem* value;
	int time;
}tmpreginfo;
static int mark[5];
static int level;
static tmpreginfo tmpregpool[5];//临时寄存器池
static int count_num;
static map<int,string> adr_reg;
static map<int,string> tmpadr_reg;
static map<string,int> tmpreg_adr;
FILE *x86codes=fopen("x86codes.asm","w");
static int nolock[5];
static void mod_reg_pool(int id,int ifuse,symbItem *value,int write_back)
{
	tmpregpool[id].ifuse=ifuse;
	tmpregpool[id].value=value;
	tmpregpool[id].write_back=write_back;
	tmpregpool[id].time=count_num++;
}
static void print_pool_info()
{
	fprintf(x86codes,";");
	for(int i=1;i<=3;i++)
	{
		if(tmpregpool[i].write_back)
			fprintf(x86codes,"%s back ",tmpadr_reg[i].data());
		else if(tmpregpool[i].ifuse)
			fprintf(x86codes,"%s use ",tmpadr_reg[i].data());
	}
	fprintf(x86codes,"\n");
}
static void init_pool()
{
	adr_reg[1]="ebx";
	adr_reg[2]="edi";
	tmpadr_reg[1]="esi";
	tmpadr_reg[2]="ecx";
	tmpadr_reg[3]="edx";
	tmpreg_adr["esi"]=1;
	tmpreg_adr["ecx"]=2;
	tmpreg_adr["edx"]=3;
	count_num=0;
	for(int i=1;i<=3;i++)
	{
		mod_reg_pool(i,0,NULL,0);
		mark[i]=0;
		nolock[i]=1;
	}
}
static void write_backarrvar(symbItem *a,string reg)//将reg的内容写到a所在的内存中
{
	if(a->level==level)
	{
		if(a->adr%4)
		{
		//	fprintf(x86codes,"mov %s,%s\n",adr_reg[a->adr].data(),reg.data());
;
		}
		else
			fprintf(x86codes,"mov [ebp%d],%s\n",a->adr,reg.data());
	}
	else
	{
		if(a->adr%4)
		{
			fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax-%d],%s\n",8+a->level*4,4+a->adr*4,reg.data());
		}
		else
		{
			fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax%d],%s\n",8+a->level*4,a->adr,reg.data());

		}
	}
	mod_reg_pool(tmpreg_adr[reg],0,NULL,0);
}

static void hwrite_back(symbItem *a,string reg)
{//不会有arrvar
	if(a->kind=="function")
	{
		if(a->level==level-1)//在自己家地盘上
		{
			fprintf(x86codes,"mov [ebp-4],%s\n",reg.data());
		}
		else
		{
			fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax-4],%s\n",8+4*(a->level+1),reg.data());
		}
	}
	else if(a->level==level)
	{
		if(a->kind=="parameter")
		{
			if(a->para_ifvar)
			{
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax],%s\n",4+4*(level+a->adr),reg.data());
			}
			else
			{
				fprintf(x86codes,"mov [ebp+%d],%s\n",4+4*(level+a->adr),reg.data());
			}
		}
		else if(a->adr%4)
		{
			if(a->kind=="arrvar")
				fprintf(x86codes,"mov [%s],%s\n",adr_reg[a->adr].data(),reg.data());
			else
				fprintf(x86codes,"mov %s,%s\n",adr_reg[a->adr].data(),reg.data());
		}
		else
		{
			if(a->kind=="arrvar")
				fprintf(x86codes,"mov eax,[ebp%d]\nmov [eax],%s\n",a->adr,reg.data());
			else
				fprintf(x86codes,"mov [ebp%d],%s\n",a->adr,reg.data());
			

		}
	}
	else
	{
		if(a->kind=="parameter")
		{
			if(a->para_ifvar)
			{
				fprintf(x86codes,"mov eax,[ebp+%d]\nlea eax,[eax+%d]\nmov [eax],%s\n",8+a->level*4,	4+(a->level+a->adr)*4,reg.data());
			}
			else
			{
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax+%d],%s\n",8+a->level*4,4+(a->level+a->adr)*4,reg.data());
			}
		}else if(a->adr%4)
		{
			if(a->kind=="arrvar")
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov eax,[eax-%d]\nmov [eax],%s\n",8+a->level*4,4+a->adr*4,reg.data());
			else
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax-%d],%s\n",8+a->level*4,4+a->adr*4,reg.data());
		}
		else
		{
			if(a->kind=="arrvar")
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov eax,[eax%d]\nmov [eax],%s\n",8+a->level*4,a->adr,reg.data());
			else
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov [eax%d],%s\n",8+a->level*4,a->adr,reg.data());
		}
	}
}
static int if_in_tmp_rag(symbItem* a)
{
	for(int i=1;i<=3;i++)
	{
		if(tmpregpool[i].value==a)
			return i;
	}
	return 0;
}
static void clear_tmpregpool()
{
	for(int i=1;i<=3;i++)
	{
		if(tmpregpool[i].write_back)
		{
			hwrite_back(tmpregpool[i].value,tmpadr_reg[i]);
		}
		mod_reg_pool(i,0,NULL,0);
	}
}
static int checkifintmpreg(symbItem* a)
{
	for(int i=1;i<=3;i++)
	{
		if(tmpregpool[i].value==a)
			return i;
	}
	return 0;
}
static void read_reg(symbItem *a,string reg)//进来的时候不能是常数
{
	if(a->level==level)
	{
		if(a->kind=="parameter")
		{
			if(a->type=="integer")
			{	if(a->para_ifvar)
					fprintf(x86codes,"mov eax,[ebp+%d]\nmov %s,[eax]\n",4+(level+a->adr)*4,reg.data());
				else
					fprintf(x86codes,"mov %s,[ebp+%d]\n",reg.data(),4+4*(level+a->adr));
			}
			else
			{
				if(a->para_ifvar)
					fprintf(x86codes,"mov eax,[ebp+%d]\nmovsx %s,byte [eax]\n",4+4*(level+a->adr),reg.data());
				else
					fprintf(x86codes,"movsx %s,byte [ebp+%d]\n",reg.data(),4+4*(level+a->adr));
			}
		}else if(a->kind=="arrvar")
		{
			fprintf(x86codes,"mov eax,[ebp%d]\nmov %s,[eax]\n",a->adr,reg.data());//这里很隐蔽的错误
		}
		else if(a->type=="integer")
		{
			fprintf(x86codes,"mov %s,[ebp%d]\n",reg.data(),a->adr);	
		}
		else
			fprintf(x86codes,"movsx %s,byte [ebp%d]\n",reg.data(),a->adr);
	}
	else
	{
		if(a->kind=="parameter")
		{
			if(a->type=="integer")
			{
				if(a->para_ifvar)
					fprintf(x86codes,"mov eax,[ebp+%d]\nlea eax,[eax+%d]\nmov %s,[eax]\n",8+a->level*4,4+(a->level+a->adr)*4,reg.data());
				else
					fprintf(x86codes,"mov eax,[ebp+%d]\nmov %s,[eax+%d]\n",8+a->level*4,reg.data(),4+(a->level+a->adr)*4);
			}
			else
			{
				if(a->para_ifvar)
					fprintf(x86codes,"mov eax,[ebp+%d]\nlea eax,[eax+%d]\nmovsx %s,byte [eax]\n",8+a->level*4,4+(a->level+a->adr)*4,reg.data());
				else
					fprintf(x86codes,"mov eax,[ebp+%d]\nmovsx %s,byte [eax+%d]\n",8+a->level*4,reg.data(),4+(a->level+a->adr)*4);

			}		
		}
		else if(a->adr%4)
		{
			if(a->type=="integer")
			{
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov %s,[eax-%d]\n",8+a->level*4,reg.data(),4+4*a->adr);

			}
			else 
			{
				fprintf(x86codes,"mov eax,[ebp+%d]\nmovsx %s,byte [eax-%d]\n",8+a->level*4,reg.data(),4+4*a->adr);
	
			}

		}
		else
		{
			if(a->type=="integer")
				fprintf(x86codes,"mov eax,[ebp+%d]\nmov %s,[eax%d]\n",8+a->level*4,reg.data(),a->adr);
			else
				fprintf(x86codes,"mov eax,[ebp+%d]\nmovsx %s,byte[eax%d]\n",8+a->level*4,reg.data(),a->adr);

		}
	}
}
void register_reg(symbItem*a,string reg,int write_back)
{	
	mod_reg_pool(tmpreg_adr[reg],1,a,write_back);
}
static int register_reg(symbItem* a,int write_back)//向临时寄存器池注册寄存器
{
	int i,flag=0;
	int target;
	for(i=1;i<=3;i++)
	{
		if(!tmpregpool[i].ifuse&&nolock[i])
			break;
		
	}
	if(i==4)//满了
	{
		int j;
		for(j=1;j<=3;j++)
		{
			if(!tmpregpool[j].write_back&&!mark[j]&&nolock[j])
				break;
		}
		if(j==4)
		{
			int m;
			for(m=1;m<=3;m++)
			{
				if(!mark[m]&&nolock[m])
					break;
			}
			if(m==4)
			{
				cout << "impossible" << endl;
			//	fprintf(x86codes,"/
				cout << mark[1]<< mark[2]<<mark[3]<<nolock[1]<<nolock[2]<<nolock[3]<<endl;
			}
			target=m;
		}
		else
		{
			target=j;
		}
	}
	else
		target=i;
	if(tmpregpool[target].write_back)//需要写回;
	{
		hwrite_back(tmpregpool[target].value,tmpadr_reg[target]);
	}
	//将要占用寄存器
	mod_reg_pool(target,1,a,write_back);
	return target;
}
//num1,是一个计算号的寄存器值，或者内存。
static void mod_mark(quadruple *tmp)
{
	for(int i=1;i<4;i++)
		mark[i]=0;
	mark[if_in_tmp_rag(tmp->src1)]=1;
	mark[if_in_tmp_rag(tmp->src2)]=1;
	mark[if_in_tmp_rag(tmp->ans)]=1;
}
static void putsrcinreg(symbItem *src,string &reg,int ifread,quadruple* tt,int write_back)//将操作数的值所在的寄存器放到reg1里面。
{//constpool,real parameter,formal para,array,var在全局寄存器中，arrvar,integer,char
	int id;
	if((id=checkifintmpreg(src))!=0)//src1已经存在在临时寄存器中。//扩展?
	{
		reg=tmpadr_reg[id];
		if(write_back)
			tmpregpool[id].write_back=write_back;
	}
	else if(src->level==level&&src->kind!="parameter"&&src->adr%4)//
	{
		reg=adr_reg[src->adr];
	}
	else
	{
		reg=tmpadr_reg[register_reg(src,write_back)];
		if(ifread)
			read_reg(src,reg);//把内容读到寄存器中。
	}	
	mod_mark(tt);
}

static void generate_basic(quadfunc *nowfunc,quadblock *block)//
{
	clear_tmpregpool();
	quadruple *nowquad=block->firstcode;
	level=nowfunc->table->level;
	while(nowquad!=block->lastcode->link)
	{
		print_pool_info();
		fprintf(x86codes,";%s\n",nowquad->opr.data());
		if(nowquad->opr=="lab")
		{
			clear_tmpregpool();
			fprintf(x86codes,"%s:\n",nowquad->src1->name.data());
		}
/*
display 区的构造总述如下:假定是从第 i 层模块进入到第 j 层模块,则:
(1) 如果 j=i+1(即进入一个 BEGIN 类型的模块或调用当前模块局部声明
的过程块),则复制 i 层的 display 区,然后增加一个指向 i 层模块活动记录基址
的指针。
(2) 如果 j<=i(即调用对当前模块来说是属于全程声明的过程模块),则来
自 i 层模块活动记录中的 display 区前面 j-1 个入口将组成 j 层模块的 display
区。
*/
		else if(nowquad->opr=="call")
		{    
	  		/*i*/                           /*j*/           
			clear_tmpregpool();
			int mynum=0;			
			int callerlev=nowfunc->table->level,calleelev=nowquad->src1->level+1;//函数代码所在层次=定义层次+1
			if(callerlev+1==calleelev)//main调用他自己定义的过程,需要加入callerlve的dispaly区以及callerlev的ebp，注意反填
			{
				fprintf(x86codes,"push ebp\n");
				for(int i=(callerlev-1)*4;i>=0;)
				{
					fprintf(x86codes,"push dword [ebp+%d]\n",i+8);
					i-=4;
					mynum++;
				}	
			}
			else//caller>=callee
			{
				for(int i=(calleelev-1)*4;i>=0;)
				{
					fprintf(x86codes,"push dword [ebp+%d]\n",i+8);
					i-=4;
					mynum++;
				}
			}
			#ifdef WIN_FORM
			fprintf(x86codes,"mov [ebp-8],ebx\nmov [ebp-12],edi\ncall _%s0%d\nmov ebx,[ebp-8]\nmov edi,[ebp-12]\nadd esp,%d\n",
					nowquad->src1->name.data(),nowquad->src1->adr,(nowquad->src1->size+mynum)*4);
			#else
			fprintf(x86codes,"mov [ebp-8],ebx\nmov [ebp-12],edi\ncall %s0%d\nmov ebx,[ebp-8]\nmov edi,[ebp-12]\nadd esp,%d\n",
					nowquad->src1->name.data(),nowquad->src1->adr,(nowquad->src1->size+mynum)*4);

			#endif

		}
		else if(nowquad->opr=="ret")
		{
			clear_tmpregpool();
			if(nowfunc->table->level)
			{
				if(nowquad->src1->kind=="function"&&nowquad->src1->type=="char")
					fprintf(x86codes,"movsx eax,byte [ebp-4]\nmov esp,ebp\npop ebp\nret\n");
				else
					fprintf(x86codes,"mov eax,[ebp-4]\nmov esp,ebp\npop ebp\nret\n");
			}
			else
				fprintf(x86codes,"mov esp,ebp\nret\n");
		}
		else if(nowquad->opr=="func")//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			clear_tmpregpool();
			if(nowquad->src1->name=="main")
			{
				#ifdef WIN_FORM
				fprintf(x86codes,"_main:\nmov ebp,esp\nsub esp,%d\n",nowfunc->table->localsnum*4+16);
				#else
				fprintf(x86codes,"main:\nmov ebp,esp\nsub esp,%d\n",nowfunc->table->localsnum*4+16);
				#endif
			}
			else
			{
				#ifdef WIN_FORM
				fprintf(x86codes,"_%s0%d:\npush ebp\nmov ebp,esp\nsub esp,%d\n"
						,nowquad->src1->name.data(),nowquad->src1->adr,nowfunc->table->localsnum*4+16);
				#else
				fprintf(x86codes,"%s0%d:\npush ebp\nmov ebp,esp\nsub esp,%d\n"
						,nowquad->src1->name.data(),nowquad->src1->adr,nowfunc->table->localsnum*4+16);
				#endif
			}
		}
		else if(nowquad->opr=="jmp")
		{
			clear_tmpregpool();
			fprintf(x86codes,"jmp %s\n",nowquad->src1->name.data());
		}
		else if(nowquad->opr=="add"||nowquad->opr=="sub"||nowquad->opr=="imul")
		{//只需要一个寄存器就好。
			string src1,src2,ans;
			mod_mark(nowquad);
			if(nowquad->src1->kind=="constpool")
				src1=nowquad->src1->name;
			else{
			   	putsrcinreg(nowquad->src1,src1,1,nowquad,0);
			}
			if(nowquad->src2->kind=="constpool")
				src2=nowquad->src2->name;
			else putsrcinreg(nowquad->src2,src2,1,nowquad,0);
			putsrcinreg(nowquad->ans,ans,0,nowquad,1);
			if(ans==src1&&ans==src2)
				fprintf(x86codes,"%s %s,%s\n",nowquad->opr.data(),ans.data(),ans.data());
			else if(src1==ans)
			{
				fprintf(x86codes,"%s %s,%s\n",nowquad->opr.data(),src1.data(),src2.data());
			}
			else if(src2==ans)
			{
				fprintf(x86codes,"mov eax,%s\n%s eax,%s\nmov %s,eax\n",src1.data(),
						nowquad->opr.data(),src2.data(),ans.data());
			}
			else
				fprintf(x86codes,"mov %s,%s\n%s %s,%s\n",ans.data(),src1.data(),nowquad->opr.data(),ans.data(),src2.data());
		}else if(nowquad->opr=="idiv")
		{
			string src1,src2,ans;
			mod_mark(nowquad);
			if(nowquad->src2->kind=="constpool")
			{
				int id=register_reg(nowquad->src2,0);
				mod_mark(nowquad);
				fprintf(x86codes,"mov %s,%s\n",tmpadr_reg[id].data(),nowquad->src2->name.data());
				mod_reg_pool(id,1,NULL,0);
				src2=tmpadr_reg[id];
			}
			else
				putsrcinreg(nowquad->src2,src2,1,nowquad,0);
			if(nowquad->src1->kind=="constpool")
				src1=nowquad->src1->name;
			else putsrcinreg(nowquad->src1,src1,1,nowquad,0);
			fprintf(x86codes,"mov eax,%s\n",src1.data());
			if(tmpregpool[tmpreg_adr["edx"]].write_back)
			{
				hwrite_back(tmpregpool[tmpreg_adr["edx"]].value,"edx");
				mod_reg_pool(tmpreg_adr["edx"],0,NULL,0);
			}
			fprintf(x86codes,"cdq\n");
			nolock[tmpreg_adr["edx"]]=0;
			fprintf(x86codes,"idiv %s\n",src2.data());
			nolock[tmpreg_adr["edx"]]=1;//解锁的时机不太对
			putsrcinreg(nowquad->ans,ans,0,nowquad,1);
			fprintf(x86codes,"mov %s,eax\n",ans.data());
		}
		else if(nowquad->opr=="je"||nowquad->opr=="jl"||nowquad->opr=="jle"||
				nowquad->opr=="jg"||nowquad->opr=="jne"||nowquad->opr=="jge")
		{
			string src1,src2;
			mod_mark(nowquad);
			if(nowquad->src1->kind=="constpool")
			{
				fprintf(x86codes,"mov eax,%s\n",nowquad->src1->name.data());
				src1="eax";
			}
			else 
				putsrcinreg(nowquad->src1,src1,1,nowquad,0);
			if(nowquad->src2->kind=="constpool")
					src2=numtostring(nowquad->src2->value);
			else
					putsrcinreg(nowquad->src2,src2,1,nowquad,0);
			clear_tmpregpool();
			fprintf(x86codes,"cmp %s,%s\n%s %s\n",src1.data(),src2.data(),nowquad->opr.data(),nowquad->ans->name.data());
		}
		else if(nowquad->opr=="assign")
		{
			string src1,ans;
			mod_mark(nowquad);
			if(nowquad->src1->kind=="function")
			{
				hwrite_back(nowquad->ans,"eax");
			}
			else if(nowquad->ans->kind=="function")
			{
				int flag=0;
				if(nowquad->src1->kind=="constpool")
				{
					flag=1;
					src1=numtostring(nowquad->src1->value);
				}
				else
					putsrcinreg(nowquad->src1,src1,1,nowquad,0);
				string ifdword=flag?"dword":"";
				if(nowquad->ans->level==level-1)
				{
					fprintf(x86codes,"mov %s [ebp-4],%s\n",ifdword.data(),src1.data());
				}
				else
				{
					fprintf(x86codes,"mov eax,[ebp+%d]\nmov %s [eax-4],%s\n",8+4*(nowquad->ans->level+1),ifdword.data(),src1.data());
				}
			}
			else
			{
				if(nowquad->src1->kind=="constpool")
				{
					int id=register_reg(nowquad->src1,0);
					mod_mark(nowquad);
					fprintf(x86codes,"mov %s,%s\n",tmpadr_reg[id].data(),nowquad->src1->name.data());
					mod_reg_pool(id,1,NULL,0);
					src1=tmpadr_reg[id];
				}
				else
					putsrcinreg(nowquad->src1,src1,1,nowquad,0);
					hwrite_back(nowquad->ans,src1);
				int id=checkifintmpreg(nowquad->ans);
				mod_reg_pool(id,0,NULL,0);
			}
		}
		else if(nowquad->opr=="neg"||nowquad->opr=="inc"||nowquad->opr=="dec")
		{
			mod_mark(nowquad);
			int id=checkifintmpreg(nowquad->src1);
			string src1;
			if(id)
			{
				src1=tmpadr_reg[id];
				mod_reg_pool(id,1,nowquad->src1,1);
			}
			else
				putsrcinreg(nowquad->src1,src1,1,nowquad,1);
			fprintf(x86codes,"%s %s\n",nowquad->opr.data(),src1.data());
		}
		else if(nowquad->opr=="rpara")//rpara增加一个参数(笑脸)取地址或者全局寄存器
		{
			clear_tmpregpool();
			if(nowquad->src1->level==level)
			{
				if(nowquad->src1->kind=="parameter")
				{
					if(nowquad->src1->para_ifvar)
						fprintf(x86codes,"mov eax,[ebp+%d]\npush eax\n",4+4*(level+nowquad->src1->adr));
					else
						fprintf(x86codes,"lea eax,[ebp+%d]\npush eax\n",4+4*(level+nowquad->src1->adr));
				}
				else if(nowquad->src1->adr%4)
				{
					if(nowquad->src1->kind!="arrvar")
						fprintf(x86codes,"lea eax,[ebp-%d]\npush eax\n",4+nowquad->src1->adr*4);
					else
						fprintf(x86codes,"push %s\n",adr_reg[nowquad->src1->adr].data());
				}
				else
				{
					if(nowquad->src1->kind=="arrvar")
						fprintf(x86codes,"push dword [ebp%d]\n",nowquad->src1->adr);
					else
						fprintf(x86codes,"lea eax,[ebp%d]\npush eax\n",nowquad->src1->adr);				
				}
			}
			else
			{
				if(nowquad->src1->kind=="parameter")
				{
					if(nowquad->src1->para_ifvar)
					{
						fprintf(x86codes,"mov eax,[ebp+%d]\npush dword [eax+%d]\n",8+nowquad->src1->level*4,4+4*(nowquad->src1->level+nowquad->src1->adr));
					}
					else
						fprintf(x86codes,"mov eax,[ebp+%d]\nlea eax,[eax+%d]\npush eax\n",8+nowquad->src1->level*4,4+4*(nowquad->src1->level+nowquad->src1->adr));
				}
				else if(nowquad->src1->adr%4)
				{
					if(nowquad->src1->kind=="arrvar")
						fprintf(x86codes,"mov eax,[ebp+%d]\npush dword[eax-%d]\n",8+nowquad->src1->level*4,4+4*nowquad->src1->adr);
					else
						fprintf(x86codes,"mov eax,[ebp+%d]\nlea eax,[eax-%d]\npush eax\n",8+nowquad->src1->level*4,4+4*nowquad->src1->adr);
				}
				else
				{
					if(nowquad->src1->kind=="arrvar")
						fprintf(x86codes,"mov eax,[ebp+%d]\npush dword [eax%d]\n",8+nowquad->src1->level*4,nowquad->src1->adr);
					else
						fprintf(x86codes,"mov eax,[ebp+%d]\nlea eax,[eax%d]\npush eax\n",8+nowquad->src1->level*4,nowquad->src1->adr);
				}
			}
		}
		else if(nowquad->opr=="fpara")
		{
			mod_mark(nowquad);
			string src1="eax";
			symbItem *srca=nowquad->src1;
			if(srca->kind=="constpool")
				fprintf(x86codes,"mov eax,%s\n",srca->name.data());
			else
				putsrcinreg(srca,src1,1,nowquad,0);
			fprintf(x86codes,"push %s\n",src1.data());
		}
		else if(nowquad->opr=="larray")//larray src1 src2 ans : ans=src1[src2]//larray指令具有二义性，arrvar类型代表地址，否则给一个数
		{
			mod_mark(nowquad);int flag=0;
			string src1,src2,ans,tm;
			if(nowquad->src2->kind=="constpool")
			{
				src2=numtostring(nowquad->src2->value);
			}
			else
				putsrcinreg(nowquad->src2,src2,1,nowquad,0);
			putsrcinreg(nowquad->ans,ans,0,nowquad,1);
			if(nowquad->ans->kind=="arrvar")
				flag=1;
			tm=flag==1?ans:"eax";
			if(nowquad->src1->level==level)
			{
				fprintf(x86codes,"lea %s,[ebp+%s*4%d]\n",tm.data(),src2.data(),nowquad->src1->adr);
			}
			else
			{
				fprintf(x86codes,"mov eax,[ebp+%d]\nlea %s,[eax+%s*4%d]\n",8+nowquad->src1->level*4,tm.data(),src2.data(),nowquad->src1->adr);
			}
			if(!flag)
			{
				if(nowquad->ans->type=="integer")//?????????????????????????
					fprintf(x86codes,"mov %s,[eax]\n",ans.data());
				else
					fprintf(x86codes,"mov %s,[eax]\n",ans.data());
			}
			else
			{
				write_backarrvar(nowquad->ans,ans);
			}
		}
		else if(nowquad->opr=="sarray")
		{
			mod_mark(nowquad);
			int id=register_reg(nowquad->src1,0);
			mod_mark(nowquad);
			string eax=tmpadr_reg[id];	
			string src2,ans;
			if(nowquad->src2->kind=="constpool")
			{
				src2=numtostring(nowquad->src2->value);
			}
			else
				putsrcinreg(nowquad->src2,src2,1,nowquad,0);
			if(nowquad->src1->level==level)
			{
				fprintf(x86codes,"lea %s,[ebp+%s*4%d]\n",eax.data(),src2.data(),nowquad->src1->adr);
			}
			else
			{
				fprintf(x86codes,"mov %s,[ebp+%d]\nlea %s,[%s+%s*4%d]\n",eax.data(),8+nowquad->src1->level*4,eax.data(),eax.data(),src2.data(),nowquad->src1->adr);
			}
			if(nowquad->ans->kind=="constpool")
			{
				ans=numtostring(nowquad->ans->value);		
				fprintf(x86codes,"mov dword [%s],%s\n",eax.data(),ans.data());
			}
			else
			{
				putsrcinreg(nowquad->ans,ans,1,nowquad,0);
				fprintf(x86codes,"mov [%s],%s\n",eax.data(),ans.data());
			}
			mod_reg_pool(id,0,NULL,0);
		}
		else if(nowquad->opr=="writes")
		{
			mod_mark(nowquad);

			#ifdef WIN_FORM
			fprintf(x86codes,"push str%d\ncall _printf\nadd esp,4\n",nowquad->src1->value);
			#else
			fprintf(x86codes,"push str%d\ncall printf\nadd esp,4\n",nowquad->src1->value);
			#endif
		}
		else if(nowquad->opr=="writee")//tmp var的类型们不一定是int
		{
			mod_mark(nowquad);
			string src1="eax";
			if(nowquad->src1->kind=="constpool")
			{
				fprintf(x86codes,"mov eax,%s\n",nowquad->src1->name.data());
			}	
			else{
				putsrcinreg(nowquad->src1,src1,1,nowquad,0);
			}
			if(nowquad->src1->type=="integer"||nowquad->src1->type=="char"&&(nowquad->src1->kind!="arrvar"&&nowquad->src1->name[0]=='_'))
			{
				#ifdef WIN_FORM
				fprintf(x86codes,"push %s\npush strint\n",src1.data());
				clear_tmpregpool();
				fprintf(x86codes,"call _printf\nadd esp,8\n");
				#else
				fprintf(x86codes,"push %s\npush strint\n",src1.data());
				clear_tmpregpool();
				fprintf(x86codes,"call printf\nadd esp,8\n");
				#endif
			}
			else
			{
				#ifdef WIN_FORM
				fprintf(x86codes,"push %s\npush stroutchar\n",src1.data());
					clear_tmpregpool();
				fprintf(x86codes,"call _printf\nadd esp,8\n");
				#else
				fprintf(x86codes,"push %s\npush stroutchar\n",src1.data());
					clear_tmpregpool();
				fprintf(x86codes,"call printf\nadd esp,8\n");
				#endif
				
			}
		}
		else if(nowquad->opr=="read")
		{
			string src1;
			mod_mark(nowquad);
			putsrcinreg(nowquad->src1,src1,0,nowquad,1);

			if(nowquad->src1->type=="integer")
			{
				#ifdef WIN_FORM
				fprintf(x86codes,"sub esp,4\npush esp\n push strint\n");
				clear_tmpregpool();
				fprintf(x86codes,"call _scanf\nadd esp,8\n");
				putsrcinreg(nowquad->src1,src1,0,nowquad,1);
				fprintf(x86codes,"mov %s,[esp]\n",src1.data());
				#else
				fprintf(x86codes,"sub esp,4\npush esp\n push strint\n");
				clear_tmpregpool();
				fprintf(x86codes,"call scanf\nadd esp,8\n");
				putsrcinreg(nowquad->src1,src1,0,nowquad,1);
				fprintf(x86codes,"mov %s,[esp]\n",src1.data());
				#endif

			}
			else
			{
				#ifdef WIN_FORM
				fprintf(x86codes,"sub esp,4\npush esp\n push strchar\n");
				clear_tmpregpool();
				fprintf(x86codes,"call _scanf\n");
				putsrcinreg(nowquad->src1,src1,0,nowquad,1);
				fprintf(x86codes,"add esp,8\nmovsx %s,byte[esp]\n",src1.data());
				#else
				fprintf(x86codes,"sub esp,4\npush esp\n push strchar\n");
				clear_tmpregpool();
				fprintf(x86codes,"call scanf\n");
				putsrcinreg(nowquad->src1,src1,0,nowquad,1);
				fprintf(x86codes,"add esp,8\nmovsx %s,byte[esp]\n",src1.data());
				#endif	
			}
				fprintf(x86codes,"add esp,4\n");
		}
		else
		{
			cout << "IMPOSSIBLE!"<<endl;
		}
		nowquad=nowquad->link;
	}
clear_tmpregpool();
}
void generate_main()
{
	#ifdef WIN_FORM
	fputs("global _main\nextern _printf\nextern _scanf\nsection .data\nstrint:db'\%d',0\nstrchar: db' %c',0\nstroutchar: db '%c',0\n",x86codes);
	#else
	fputs("global main\nextern printf\nextern scanf\nsection .data\nstrint:db'\%d',0\nstrchar: db' %c',0\nstroutchar: db '%c',0\n",x86codes);
	#endif
	init_pool();
	for(int i=1;i<my_writes_num;i++)
	{
		fprintf(x86codes,"str%d: db \"%s\",0\n",i,my_write_string.front().data());
		my_write_string.pop();
	}
	fprintf(x86codes,"section .text\n");
	quadfunc *nowfunc=quadruple_codes;
	while(nowfunc)
	{
		for(int i=0;i<nowfunc->blocknum;i++)
		{
			generate_basic(nowfunc,nowfunc->blocks[i]);
		}
		nowfunc=nowfunc->link;
	}
}
