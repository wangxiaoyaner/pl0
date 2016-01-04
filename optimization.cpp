#include"global.h"
//EBX,EDI,ESI 123
static void sigtable_addr_alloc(quadfunc *target)
{
	int local_start=-16,ebxediesi=1;
	symbTable *tmptable=target->table;
	tmptable->localsnum=0;
	symbItem *tmpitem=tmptable->first_item;
	while(tmpitem!=NULL)
	{
		if(!tmpitem->adr&&tmpitem->kind=="var"||tmpitem->kind=="array"||tmpitem->kind=="arrvar")//未分配地址
		{
			if(ebxediesi<3&&tmpitem->kind!="array"&&tmpitem->kind!="arrvar")
				tmpitem->adr=ebxediesi++;
			else if(tmpitem->kind=="array")
			{
				local_start-=(4*tmpitem->size);
				tmpitem->adr=local_start+4;//数组元素递增
				tmptable->localsnum+=tmpitem->size;
			}
			else
			{
				tmpitem->adr=local_start;
				local_start-=4;
				tmptable->localsnum++;//记录在栈上分配变量的地址数。
			}
		}
		tmpitem=tmpitem->link;
	}
}
void optimazation_adr_alloc()
{
	quadruple_codes_now=quadruple_codes;
	while(quadruple_codes_now!=NULL)
	{
		sigtable_addr_alloc(quadruple_codes_now);
		quadruple_codes_now=quadruple_codes_now->link;
	}
}
