#include"global.h"
//深度好文
//http://wenku.baidu.com/link?url=O9TE1hFXrvT23biXhQI2GgfSVUE0hD9XwVsz0m7t_7SciuVez_-3qixBsKa10qaIUOB1xMMGBLThnBI09eY0njyIZukvvbwROCLMzHkchl3
//还没有删掉指定的Node(A),A不是叶子节点。
typedef struct dagnode
{
	int id;
	int exist;
	string opr;
	symbItem* relational_item;
	int fathernum;
	vector<int> childs;
	vector<symbItem*>locals;
	vector<symbItem*>tmpvars;
}dagnode;
static map<string,int> basicblock_opr_kind;
static stack<int> all_nodes;
static void opr_kind_init()//标记的是操作数的个数
{
	basicblock_opr_kind["func"]=-2;
	basicblock_opr_kind["ret"]=-2;
	basicblock_opr_kind["lab"]=-2;
	basicblock_opr_kind["jmp"]=-2;

	basicblock_opr_kind["add"]=2;
	basicblock_opr_kind["sub"]=2;
	basicblock_opr_kind["imul"]=2;
	basicblock_opr_kind["idiv"]=2;

	basicblock_opr_kind["assign"]=6;
	basicblock_opr_kind["neg"]=1;
	basicblock_opr_kind["inc"]=1;
	basicblock_opr_kind["dec"]=1;
	basicblock_opr_kind["larray"]=2;//
	basicblock_opr_kind["sarray"]=3;//

	basicblock_opr_kind["jne"]=4;
	basicblock_opr_kind["je"]=4;
	basicblock_opr_kind["jge"]=4;
	basicblock_opr_kind["jg"]=4;
	basicblock_opr_kind["jl"]=4;
	basicblock_opr_kind["jle"]=4;

	basicblock_opr_kind["call"]=-3;//
	basicblock_opr_kind["rpara"]=6;
	basicblock_opr_kind["fpara"]=6;
	basicblock_opr_kind["writes"]=6;
	basicblock_opr_kind["writee"]=6;
	basicblock_opr_kind["read"]=-2;
}
static vector<dagnode*> block_dag_graph;//天啊，这里只能先这么不安全着了。
static int dagnodenum=0;
static map<symbItem*,int> symbitem_int;
static int if_new_b=0;
static int if_new_c=0;
static int if_new_d=0;
static int hehe;
static int adr_b;
static int adr_c;
static int adr_d;

static int if_opr_is_const(dagnode *t)//判断节点是否是常数
{
	return t->opr=="0"&&t->relational_item->kind=="constpool";
}


static void create_new_node(string opr,symbItem *relation,symbItem *identifer)
{
	dagnode *ans=new dagnode();
	ans->exist=1;
	ans->id=block_dag_graph.size();
	ans->opr=opr;
	ans->relational_item=relation;
	ans->fathernum=0;
	if(identifer!=NULL)
		if(identifer->name[0]=='_')//tmp var;
			ans->tmpvars.push_back(identifer);
		else
			ans->locals.push_back(identifer);
	if(relation!=NULL)
		symbitem_int[relation]=block_dag_graph.size();
	if(identifer!=NULL)
		symbitem_int[identifer]=block_dag_graph.size();
	block_dag_graph.push_back(ans);
}

static int if_item_in_dag(symbItem *t)//判断在表中是否有元素t，有久返回对应的节点
{
	if(symbitem_int.find(t)!=symbitem_int.end()&&symbitem_int[t]!=-1)
		return symbitem_int[t];
	return -1;
}

static int find_or_birth_child(symbItem *target)
{
	if(symbitem_int.find(target)!=symbitem_int.end()&&symbitem_int[target]!=-1)
	{
		int id=symbitem_int[target];
		symbItem *ta=block_dag_graph[id]->relational_item;
		if(ta==target&&ta&&(ta->kind=="constpool"||ta->kind=="const"))//当初自己定义的名字乱七八糟，现在倒是不记得什么是什么了。
			return id;
	}
	create_new_node("0",target,NULL);
	return block_dag_graph.size()-1;
}

static int find_or_birth_child(symbItem *target,int &mark)
{
	if(symbitem_int.find(target)!=symbitem_int.end()&&symbitem_int[target]!=-1)//已经存在在某一节点
	{
		mark=0;
		return symbitem_int[target];
	}
	else
	{
		mark=1;
		create_new_node("0",target,NULL);
		return block_dag_graph.size()-1;
	}
}

static int find_or_make_father(string opr,symbItem *fa,symbItem *rela,int adra,int adrb,int adrc)
{
	int len=block_dag_graph.size();
	int i;
	for(i=0;i<len;i++)
	{
		if(block_dag_graph[i]->opr==opr)
		{
			if(block_dag_graph[i]->childs[0]==adra&&block_dag_graph[i]->childs[1]==adrb&&block_dag_graph[i]->childs[2]==adrc)
				break;
		}
	}
	if(len==i)
	{
		create_new_node(opr,rela,fa);
		block_dag_graph[adra]->fathernum++;
		block_dag_graph[adrb]->fathernum++;
		block_dag_graph[adrc]->fathernum++;
		block_dag_graph.back()->childs.push_back(adra);
		block_dag_graph.back()->childs.push_back(adrb);
		block_dag_graph.back()->childs.push_back(adrc);
	}
	else
	{
		if(fa->name[0]=='_')
			block_dag_graph[i]->tmpvars.push_back(fa);
		else
			block_dag_graph[i]->locals.push_back(fa);
		symbitem_int[fa]=i;
	}
}
static void add_identifers(int id,symbItem *t)
{
	if(t->name[0]=='_')
	{
		block_dag_graph[id]->tmpvars.push_back(t);
	}
	else
		block_dag_graph[id]->locals.push_back(t);
}
static void meet_call_code()
{
	for(map<symbItem*,int>::iterator it=symbitem_int.begin();it!=symbitem_int.end();it++)
	{
		if(it->first->kind=="constpool"||it->first->name[0]=='_');
		else
			symbitem_int[it->first]=-1;
	}
}
static int find_or_make_father(string opr,symbItem *fa,symbItem *rela,int adra)
{
	if(symbitem_int.find(fa)!=symbitem_int.end()&&symbitem_int[fa]!=-1)
	{
		dagnode *tmp=block_dag_graph[symbitem_int[fa]];
		for(vector<symbItem*>::iterator it=tmp->locals.begin();it!=tmp->locals.end();it++)
		{
			if(*it==fa)
			{
				tmp->locals.erase(it);
				break;
			}
		}
	}
	if(opr=="assign")
	{
		add_identifers(adra,fa);
		symbitem_int[fa]=adra;
	}
	else if(opr=="inc"||opr=="dec"||opr=="neg")//在这里仅仅更新了local的symbitem_int值
	{
		create_new_node(opr,fa,NULL);
		block_dag_graph[adra]->fathernum++;
	}
	else
	{
		int len=block_dag_graph.size();
		int i;
		for(i=0;i<len;i++)
		{
			if(block_dag_graph[i]->opr==opr&&block_dag_graph[i]->childs[0]==adra)
			break;
		}
		if(len==i||opr=="writes"||opr=="writee")
		{
			create_new_node(opr,rela,fa);
			block_dag_graph[adra]->fathernum++;
			block_dag_graph.back()->childs.push_back(adra);
		}
		else
		{
			if(fa->name[0]=='_')
				block_dag_graph[i]->tmpvars.push_back(fa);
			else
				block_dag_graph[i]->locals.push_back(fa);
			symbitem_int[fa]=i;
		}
	}
	
}
static int find_or_make_father(string opr,symbItem *fa,symbItem *rela,int adra,int adrb)
{
	if(fa)
	{
		if(symbitem_int.find(fa)!=symbitem_int.end()&&symbitem_int[fa]!=-1)
		{
			dagnode *tmp=block_dag_graph[symbitem_int[fa]];
			for(vector<symbItem*>::iterator it=tmp->locals.begin();it!=tmp->locals.end();it++)
			{
				if(*it==fa)
				{
					tmp->locals.erase(it);
					break;
				}
			}
		}
	}
	int len=block_dag_graph.size(),i;
	for(i=0;i<len;i++)
	{
		if(block_dag_graph[i]->opr==opr)
		{
			if(block_dag_graph[i]->childs[0]==adra&&block_dag_graph[i]->childs[1]==adra
					||((opr=="*"||opr=="+")&&(block_dag_graph[i]->childs[0]==adrb&&block_dag_graph[i]->childs[1]==adra)))
					break;
			
		}
	}
	if(i==len)//not find
	{
		create_new_node(opr,rela,fa);
		block_dag_graph[adra]->fathernum++;
		block_dag_graph[adrb]->fathernum++;
		block_dag_graph.back()->childs.push_back(adra);
		block_dag_graph.back()->childs.push_back(adrb);
	}
	else
	{
		if(fa->name[0]=='_')
			block_dag_graph[i]->tmpvars.push_back(fa);
		else
			block_dag_graph[i]->locals.push_back(fa);
		symbitem_int[fa]=i;
	}

}
static int handle_if_required_node(int i)//判断是否舍弃
{
	dagnode *t=block_dag_graph[i];
	if(t->fathernum==0&&t->opr=="0"&&t->locals.empty())
	{
		t->exist=0;//这不就把孤立节点删去了么。
		return 0;
	}
	else if(t->fathernum==0)
	{
		t->exist=0;
		for(int i=0;i<t->childs.size();i++)
		{
			block_dag_graph[t->childs[i]]->fathernum--;
		}
		return 1;
	}
	else
		return 0;//还有父亲
}
static int find_right_node()
{
	int len=block_dag_graph.size();
	for(int j=len-1;j>=0;j--)
	{
		if(!block_dag_graph[j]->exist)
			continue;
		if(handle_if_required_node(j))
		{
			all_nodes.push(j);
			if(!block_dag_graph[j]->childs.empty())
			{
				int tmp=block_dag_graph[j]->childs[0];
				while(block_dag_graph[tmp]->opr!="call"&&handle_if_required_node(tmp))
				{
					all_nodes.push(tmp);
					if(block_dag_graph[tmp]->childs.empty())
						break;
					else
						tmp=block_dag_graph[tmp]->childs[0];
				}
				
			}
			return 1;
		}
	}
	return 0;
}
static void delete_last_node()//删除图里最新建立的节点,这种情况只会发生在新建立的点是常数身上，也就是有relation
{
	symbitem_int[block_dag_graph.back()->relational_item]=-1;
	block_dag_graph.pop_back();
}
static quadruple* create_new_quadruple_in_block(string opr,symbItem* src1,symbItem*src2,symbItem*ans)
{
	quadruple *new_item=new quadruple();
	new_item->opr=opr;
	new_item->src1=src1;
	new_item->src2=src2;
	new_item->ans=ans;
	new_item->link=NULL;
	if(src1)
		src1->exist=1;
	if(src2)
		src2->exist=1;
	if(ans)
		ans->exist=1;
	return new_item;
}
static symbItem* find_src_in_dag(int i)
{
	dagnode* t=block_dag_graph[i];
	if(t->opr=="0"||t->opr=="inc"||t->opr=="dec"||t->opr=="neg"||basicblock_opr_kind[t->opr]==-2)
		return t->relational_item;
	else
	{
		if(!t->locals.empty())
		{
			return t->locals[0];
		}
		else
		{
			if(!t->tmpvars.empty())
			{
				return t->tmpvars[0];
			}
			else
			{
				cout << "You are kidding me!" << endl;
				return NULL;
			}
		}
	}
}
static void add_quad_to_block(quadruple *new_item,quadblock *block)
{
	if(block->firstcode==NULL)
	{
		block->firstcode=new_item;
		block->lastcode=new_item;
	}
	else
	{
		block->lastcode->link=new_item;
		block->lastcode=block->lastcode->link;
	}
}
static void create_new_dag_quad(int i,quadblock *block)
{
	quadruple *new_item;
	dagnode *t=block_dag_graph[i];
	switch(basicblock_opr_kind[t->opr])
	{
		case -2:
			new_item=create_new_quadruple_in_block(t->opr,t->relational_item,NULL,NULL);			
			add_quad_to_block(new_item,block);
		/*	if(!t->locals.empty())
			{
				for(int i=0;i<t->locals.size();i++)
				{
					new_item=create_new_quadruple_in_block("assign",t->relational_item,NULL,t->locals[i]);
				
					add_quad_to_block(new_item,block);
				}
			}*/
			break;
		case 2:
			if(!t->locals.empty())
			{
				new_item=create_new_quadruple_in_block(t->opr,find_src_in_dag(t->childs[0]),find_src_in_dag(t->childs[1]),t->locals[0]);

				add_quad_to_block(new_item,block);
				for(int i=1;i<t->locals.size();i++)
				{
					new_item=create_new_quadruple_in_block("assign",t->locals[0],NULL,t->locals[i]);
				
					add_quad_to_block(new_item,block);
				}
			}
			else
			{
				if(t->tmpvars.empty())
					cout << "IMPOSSIBLE|||" << endl;
				else
				{
					new_item=create_new_quadruple_in_block(t->opr,find_src_in_dag(t->childs[0]),find_src_in_dag(t->childs[1]),t->tmpvars[0]);
					add_quad_to_block(new_item,block);
				}
			}
			break;
		case 1:
			new_item=create_new_quadruple_in_block(t->opr,t->relational_item,NULL,NULL);
			add_quad_to_block(new_item,block);
			for(int i=0;i<t->locals.size();i++)
			{
				new_item=create_new_quadruple_in_block("assign",t->relational_item,NULL,t->locals[i]);
				add_quad_to_block(new_item,block);
			}
			break;
		case 4:
			new_item=create_new_quadruple_in_block(t->opr,find_src_in_dag(t->childs[0]),find_src_in_dag(t->childs[1]),t->relational_item);
			add_quad_to_block(new_item,block);
			break;
		case -3:
			new_item=create_new_quadruple_in_block(t->opr,t->relational_item,NULL,NULL);
			add_quad_to_block(new_item,block);
			if(!t->locals.empty())
			{
				for(int i=0;i<t->locals.size();i++)
				{
					new_item=create_new_quadruple_in_block("assign",t->relational_item,NULL,t->locals[i]);
					add_quad_to_block(new_item,block);

				}
			}
			else
			{
				if(!t->tmpvars.empty())
				{
					new_item=create_new_quadruple_in_block("assign",t->relational_item,NULL,t->tmpvars[0]);
					add_quad_to_block(new_item,block);

				}
			}
			break;
		case 6:
			new_item=create_new_quadruple_in_block(t->opr,find_src_in_dag(t->childs[0]),t->relational_item,NULL);
			add_quad_to_block(new_item,block);

			break;
		case 3:
			new_item=create_new_quadruple_in_block(t->opr,find_src_in_dag(t->childs[0]),find_src_in_dag(t->childs[1]),find_src_in_dag(t->childs[2]));
			add_quad_to_block(new_item,block);

			break;
		default:
			for(int i=0;i<t->locals.size();i++)
			{
				new_item=create_new_quadruple_in_block("assign",t->relational_item,NULL,t->locals[i]);
				add_quad_to_block(new_item,block);
			}
	}
}
static void print_info()//输出dag图的信息
{
	int len=block_dag_graph.size();
	for(int i=0;i<len;i++)
	{
		cout << "===" << endl;
		dagnode *tmp=block_dag_graph[i];
		printf("opr:%s\n",tmp->opr.data());
		if(tmp->relational_item!=NULL)
			printf("relation:%s\nbound iden:\n",tmp->relational_item->name.data());
		for(int j=0;j<tmp->locals.size();j++)
		{
			printf("%s\t",tmp->locals[j]->name.data());
		}		
		for(int j=0;j<tmp->tmpvars.size();j++)
			printf("%s\t",tmp->tmpvars[j]->name.data());
		printf("\n");
		printf("childs:\n");
		for(int j=0;j<tmp->childs.size();j++)
		{
			printf("%d\t",tmp->childs[j]);
		}
		cout << "fathernum:" << tmp->fathernum<< endl;
	}
}

static void block_dag_block(quadblock* block)
{
	quadruple *t=block->firstcode;
	symbItem *ta=NULL,*tb=NULL;
	while(t!=block->lastcode->link)
	{
			
	cout <<"处理四元式的名字是:" << t->opr << endl;
		switch(basicblock_opr_kind[t->opr]){
			case -2:
				cout <<"before create_new_node"<< endl;
				create_new_node(t->opr,t->src1,NULL);
				cout <<"end create_new_node"<<endl;
				if(t->opr=="read")
					symbitem_int[t->src1]=-1;
				break;
			case 2:
				adr_b=find_or_birth_child(t->src1,if_new_b);
				adr_c=find_or_birth_child(t->src2,if_new_c);
				if(if_opr_is_const(block_dag_graph[adr_b])&&if_opr_is_const(block_dag_graph[adr_c]))
				{
					int answer;
					ta=block_dag_graph[adr_b]->relational_item;
					tb=block_dag_graph[adr_c]->relational_item;
					if(if_new_c&&if_new_b)
					{
						delete_last_node();
						delete_last_node();
						if_new_c=0;
						if_new_b=0;
					}
					else if(if_new_b&&!if_new_c)
					{
						delete_last_node();
						if_new_b=0;
					}
					else if(if_new_c&&!if_new_b)
					{
						if_new_c=0;
						delete_last_node();
					}
					if(t->opr=="add")
						answer=ta->value+tb->value;
					else if(t->opr=="idiv")
						answer=ta->value/tb->value;
					else if(t->opr=="imul")
						answer=ta->value*tb->value;
					else
						answer=ta->value-tb->value;
					symbItem *new_item=new symbItem();
					new_item->value=answer;
					if(ta->type=="char"&&tb->type=="char")
					{
						char tmp=answer;
						new_item->name="\'";
						new_item->name+=tmp;
						new_item->name+="\'";
						new_item->kind="constpool";
						new_item->type="char";
						new_item->link=NULL;
					}
					else
					{
						new_item->name=numtostring(answer);
						new_item->type="integer";
						new_item->kind="constpool";
						new_item->link=NULL;
					}
					global_const_pool.push(new_item);
					create_new_node("0",new_item,t->ans);//如果是孤立的临时变量结点，久可以不处理他了
				}else
				{
					cout << "before find_or_make_father" <<endl;
					find_or_make_father(t->opr,t->ans,NULL,adr_b,adr_c);
					cout << "end find_or_make_fahter"<<endl;
				}
				break;
			case 4:
				adr_b=find_or_birth_child(t->src1,if_new_b);
				adr_c=find_or_birth_child(t->src2,if_new_c);
				find_or_make_father(t->opr,NULL,t->ans,adr_b,adr_c);
				break;
			case 3:
				adr_b=find_or_birth_child(t->src1,if_new_b);
				adr_c=find_or_birth_child(t->src2,if_new_c);
				adr_d=find_or_birth_child(t->ans,if_new_d);
				find_or_make_father(t->opr,NULL,NULL,adr_b,adr_c,adr_d);
				break;
			case 6:
				if(t->opr=="writee"||t->opr=="writes"||t->opr=="rpara")
				{
					symbitem_int[t->src1]=-1;
				}
				if(t->opr=="assign"&&t->src1->name[0]!='_')
				{
					adr_b=find_or_birth_child(t->src1);//b:=a，就得有一个a0
				}
				else
					adr_b=find_or_birth_child(t->src1,if_new_b);
				find_or_make_father(t->opr,t->ans,t->src2,adr_b);
				break;
			case -3:
				meet_call_code();

				create_new_node(t->opr,t->src1,NULL);
				for(int i=block_dag_graph.size()-2;(block_dag_graph[i]->opr=="fpara"||block_dag_graph[i]->opr=="rpara")&&i>=0;i--)
				{
					block_dag_graph[i]->fathernum++;
					block_dag_graph.back()->childs.push_back(i);
				}
				break;
			case 1:
				adr_b=find_or_birth_child(t->src1,if_new_b);
				if(if_opr_is_const(block_dag_graph[adr_b]))
				{
					int tmpv=block_dag_graph[adr_b]->relational_item->value;
					int answer=t->opr=="neg"?tmpv*(-1):t->opr=="inc"?tmpv+1:tmpv-1;
					symbItem *new_item=new symbItem();
					new_item->value=answer;
					if(block_dag_graph[adr_b]->relational_item->type=="char")
					{
						char tmp=answer;
						new_item->name="\'";
						new_item->name+=tmp;
						new_item->name+="\'";
						new_item->kind="constpool";
						new_item->type="char";
						new_item->link=NULL;
					}
					else
					{
						new_item->name=numtostring(answer);
						new_item->type="integer";
						new_item->kind="constpool";
						new_item->link=NULL;
					}
					global_const_pool.push(new_item);
					create_new_node("0",new_item,t->src1);//如果是孤立的临时变量结点，久可以不处理他了
					break;
				}
				find_or_make_father(t->opr,t->src1,NULL,adr_b);
				break;
		}
		cout << "ok"<<endl;
		t=t->link;
	}
/*	cout << "***" << endl;
	print_info();
	cout << "***" << endl;*/
	cout << "begin dag to quad" << endl;
	while(find_right_node());
	cout << "end dag to quad" << endl;
	block->firstcode=NULL;
	block->lastcode=NULL;
	while(!all_nodes.empty())
	{
		create_new_dag_quad(all_nodes.top(),block);
		all_nodes.pop();
	}
	symbitem_int.clear();
	block_dag_graph.clear();
}
//基本块最终决定从0开始编号
static void print_block(quadblock* block)
{
	printf("%d 后继：",block->num);
	for(int i=0;i<block->subsn;i++)
	{
		printf("%d ",block->subsnum[i]);
	}
	printf("\n");
	cout << endl;
	//转..
	block_dag_block(block);
	quadruple *tmp=block->firstcode;
	while(tmp!=NULL)
	{
		cout << tmp->opr <<'\t';
		if(tmp->src1!=NULL)
			cout<<tmp->src1->name<<'\t';
		if(tmp->src2!=NULL)
			cout<<tmp->src2->name<<'\t';
		if(tmp->ans!=NULL)
			cout<<tmp->ans->name;
		cout << endl;
		tmp=tmp->link;
	}
	/*
	while(tmp!=block->lastcode->link)
	{
		cout << tmp->opr <<endl;
		tmp=tmp->link;
	}*/
}

static void print_func_blocks(quadfunc *func)
{
	cout << "########################################################################" << endl;
	cout << "输出"<<func->table->name<<"的基本块:"<< endl;
	for(int i=0;i<func->blocknum;i++)
	{
		cout << "&&&&&&&&&&&&&" << endl;
		print_block(func->blocks[i]);
		block_dag_block(func->blocks[i]);
		print_info();
		cout << "&&&&&&&&&&&&" << endl;
	}
	cout << "########################################################################"<< endl;
}
void basicblock_print_all_blocks()
{
	quadfunc *tmp=quadruple_codes;
	while(tmp!=NULL)
	{
		print_func_blocks(tmp);
		tmp=tmp->link;
	}
}
void bb_display_quad()
{
	quadfunc *tmp=quadruple_codes;
	while(tmp!=NULL)
	{
		cout << "函数的名字是："<<tmp->table->name<<"\t"<<tmp->table->level<<" 函数内容如下:"<<endl;
		for(int i=0;i<tmp->blocknum;i++)
		{
			quadruple *now=tmp->blocks[i]->firstcode;
			cout << "div----div------div--------div----------div"<<endl;
			while(now!=NULL)
			{
				cout << now->opr <<'\t';
				if(now->src1)
					cout<<now->src1->name<<'\t';
				if(now->src2)
					cout<<now->src2->name<<'\t';
				if(now->ans)
					cout<<now->ans->name;
				cout << endl;
				now=now->link;
			}
		}
		tmp=tmp->link;
	}
}
map<symbItem*,int> refer_count;
vector<pair<symbItem*,int> > refers;

int cmp(const pair<symbItem*,int>&x,const pair<symbItem*,int>&y)
{
	return x.second>y.second;
}
void sortMapByValue(map<symbItem*,int>& tmap,vector<pair<symbItem*,int> >&tvec)
{
	for(map<symbItem*,int>::iterator curr=tmap.begin();curr!=tmap.end();curr++)
	{
		tvec.push_back(make_pair(curr->first,curr->second));
	}
	sort(tvec.begin(),tvec.end(),cmp);
}
static void count_eve_item(symbItem *tmp,int weight,int lev)
{
	if(tmp&&tmp->kind=="var"&&tmp->level==lev)//把临时变量也算上了。
	{
		if(refer_count.find(tmp)!=refer_count.end())
			refer_count[tmp]+=weight;
		else
			refer_count[tmp]=weight;
	}
}
void codes_to_codes()
{
	quadfunc *tmp=quadruple_codes;
	while(tmp!=NULL)
	{
		int level=tmp->table->level;//function level
		for(int i=0;i<tmp->blocknum;i++)
		{
			cout <<"blocknum:i:"<< i << endl;
			block_dag_block(tmp->blocks[i]);
			cout <<"blocknum:i"<< i << endl;
		//judge if block in a loop
			int loop=0;
			for(int j=0;j<tmp->blocks[i]->subsn;j++)
			{
				if(tmp->blocks[i]->subsnum[j]<=i)
				{
					loop=1;
//					t->blocks[i]->ifloop=1;
				}
			}
		/*	if(!loop)
				tmp->blocks[i]->ifloop=0;*/
		//count references in each block;
			quadruple *first=tmp->blocks[i]->firstcode;
			quadruple *last=tmp->blocks[i]->lastcode;
			int weight=loop?8:1;
			while(first!=last->link)
			{
				count_eve_item(first->src1,weight,level);
				count_eve_item(first->src2,weight,level);
				count_eve_item(first->ans,weight,level);
				first=first->link;
			}
		}
		sortMapByValue(refer_count,refers);
		//sort and determin global register with no para
		for(int m=0;m<refers.size()&&m<3;m++)
		{
			refers[m].first->adr=m+1;//地址分配
		}
		refer_count.clear();
		refers.clear();
		tmp=tmp->link;
	}
}
static void func_to_block(quadfunc* func)//划分基本块
{
	quadruple *tmp=func->firstcode;
	map<int,quadblock*>int_block;
	map<string,int>lab_bint;//标签所在的基本块
	map<int,string>bint_subs_lab;//基本块的后继标签
	int num=0;
	quadblock *now=NULL;
	while(tmp!=NULL)
	{
		if(tmp==func->firstcode||tmp->opr=="lab"&&now!=NULL&&now->firstcode!=NULL)
		{
		
				now=new quadblock();
				now->firstcode=tmp;
				now->lastcode=tmp;
				now->link=NULL;
				now->subsn=0;
				lab_bint[tmp->src1->name]=num;
				now->num=num++;
				int_block[num-1]=now;
		}
		else if(now&&(tmp->opr=="jmp"
				||tmp->opr=="jne"||tmp->opr=="je"||tmp->opr=="jge"||tmp->opr=="jle"||tmp->opr=="jg"
				||tmp->opr=="jl"))
		{
			if(now->firstcode==NULL)
			{
				now->firstcode=tmp;
				now->lastcode=tmp;
			}
			else
			{
				now->lastcode->link=tmp;
				now->lastcode=now->lastcode->link;
			}
			if(tmp->opr=="jmp")
				bint_subs_lab[now->num]=tmp->src1->name;
			else
				bint_subs_lab[now->num]=tmp->ans->name;
	
			now=new quadblock();
			now->num=num++;
			now->firstcode=NULL;
			now->lastcode=NULL;
			now->link=NULL;
			now->subsn=0;
			int_block[num-1]=now;
		}
		else
		{
			if(tmp->opr=="lab")
			{
				lab_bint[tmp->src1->name]=now->num;
			}
			if(now->firstcode==NULL)
			{
				now->firstcode=tmp;
				now->lastcode=tmp;
			}
			else
			{
				now->lastcode->link=tmp;
				now->lastcode=now->lastcode->link;
			}
		}
		tmp=tmp->link;				
	}
	func->blocks=new quadblock*[num];
	func->blocknum=num;
	for(int i=0;i<num;++i)
	{
		func->blocks[i]=int_block[i];	
		quadblock *t=func->blocks[i];
		if(bint_subs_lab.find(i)!=bint_subs_lab.end())
		{
			t->subsnum[t->subsn++]=lab_bint[bint_subs_lab[i]];
		}
		if(i<num-1)
			t->subsnum[t->subsn++]=i+1;
	}
}
void basicblock_func_to_block()
{
	 opr_kind_init();
	quadfunc *tmp=quadruple_codes;
	while(tmp!=NULL)
	{
		cout << "aaabb" << endl;
		func_to_block(tmp);
		cout << "okaa" << endl;
		tmp=tmp->link;
	}

}
