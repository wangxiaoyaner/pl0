//#define MAIN_DEBUG
#include"global.h"
int main()
{
	char filename[100];
	printf("Enter filename:\n");
	scanf("%s",filename);
	if((sourcefile=fopen(filename,"r"))==NULL)
	{
			cout << "file open error\n";
	}
	else{
		global_init();
		symbtable_new_level("main");
		parser_program();
		basicblock_func_to_block();
		if(!global_error_num)
		{
			optimazation_adr_alloc();
			#ifdef MAIN_DEBUG
			symbtable_display();
			global_quadruple_display();
			printf("Program compiles successfully!\n");
			#else
				printf("Program compiles successfully!\n");
			#endif
			generate_main();
		}
	}
	return 0;
}  
