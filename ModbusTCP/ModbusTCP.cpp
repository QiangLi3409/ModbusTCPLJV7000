// ModbusTCP.cpp : Defines the entry point for the console application.
//
#include <winsock.h>
#include "stdafx.h"
#include "ModbusTCP.h"


#include <time.h>


double diffclock( clock_t clock1, clock_t clock2)
{
	double diffticks=clock2-clock1;
	double diffms=(diffticks*1000)/CLOCKS_PER_SEC;
	return diffms;
}
int main(int argc, char **argv)
{
  if (argc<5)
  {
    printf("usage: test1 ip_adrs unit reg_no num_regs\n"
    "eg test1 198.138.72 5 0 10\n");
    return 1;
  }
  char *ip_adrs = argv[1];
  unsigned short unit = atoi(argv[2]);
  unsigned short reg_no = atoi(argv[3]);
  unsigned short num_regs = atoi(argv[4]);
  printf("ip_adrs = %s unit = %d reg_no = %d num_regs = %d\n",
  ip_adrs, unit, reg_no, num_regs);

  unsigned short* values = new unsigned short[num_regs];

  for( int i = 0 ;i< num_regs;i++)
  {
	  values[i] =30000+i;
  }


  // for(int i=0 ;i<num_regs; i++)
   // printf("orignal word %d = %d\n", i, values[i]);

  modbus_tcp m;
  double t1,t2;
  if(m.Connect())
	{
		clock_t begin=clock(); 
		if(!m.WriteMultipleRegisters(reg_no,num_regs,values))
			  printf(m.error_msg);
		clock_t end=clock();
		t1 = diffclock(begin,end);
		
		begin=clock(); 
		if ( !m.ReadMultipleRegisters(reg_no,num_regs,values))
			printf(m.error_msg);
		end=clock();
		t2 = diffclock(begin,end);
	
	  }
	  else
	  {
		  printf(m.error_msg);
	  }
  
 // for(int i=0 ;i<num_regs; i++)
  //  printf("modbus word %d = %d\n", i, values[i]);


  printf("write %d register in %4.3f ms\n",num_regs,t1);
  printf("read %d register in %4.3f ms\n",num_regs,t2);

  return 0;
}