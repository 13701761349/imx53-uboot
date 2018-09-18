#include <common.h>
#include <command.h>
#ifdef CONFIG_CMD_CPUID

extern void imx53_cpuid(void);
int do_cpuid(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
  imx53_cpuid();
  return 0;
}
U_BOOT_CMD(
cpuid,1,0,do_cpuid,"-imx53 cpu id\n","-imx53 cpu id\n"
);
#endif

