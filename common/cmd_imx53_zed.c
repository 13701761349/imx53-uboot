/**
 * add by ljs
 */

#include <common.h>
#include <command.h>


extern void update_logo(void);
int do_update_logo(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	update_logo();
  return 0;
}
U_BOOT_CMD(
slogo,1,0,do_update_logo,"do_update_logo","-"
);



extern void update_uImage(void);
int do_update_uImage(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	update_uImage();
  return 0;
}
U_BOOT_CMD(
suImage,1,0,do_update_uImage,"do_update_uImage","-"
);




extern void update_uboot(void);
int do_update_uboot(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	update_uboot();
  return 0;
}
U_BOOT_CMD(
suboot,1,0,do_update_uboot,"do_update_uboot","-"
);







extern void imx53_5v_on(void);
int do_5v_on(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	imx53_5v_on();
  return 0;
}
U_BOOT_CMD(
5ve,1,0,do_5v_on,"do_5v_on","-"
);


extern void imx53_5v_off(void);
int do_5v_off(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	imx53_5v_off();
  return 0;
}
U_BOOT_CMD(
5vd,1,0,do_5v_off,"do_5v_off","-"
);


extern void imx53_33v_on(void);
int do_33v_on(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	imx53_33v_on();
  return 0;
}
U_BOOT_CMD(
33ve,1,0,do_33v_on,"do_33v_on","-"
);

extern void imx53_33v_off(void);
int do_33v_off(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
//  printf("imx53 cpu id: \n ");
	imx53_33v_off();
  return 0;
}
U_BOOT_CMD(
33vd,1,0,do_33v_off,"do_33v_off","-"
);


#ifdef CONFIG_VIDEO_MX5
extern void imx53_pwm0_enalbe();
int do_pwm0_enable(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
	imx53_pwm0_enalbe();
}
U_BOOT_CMD(
pwm0e,1,0,do_pwm0_enable,"do_pwm0_enable","-"
);


extern void imx53_pwm0_disalbe();
int do_pwm0_disable(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
	imx53_pwm0_disalbe();
}
U_BOOT_CMD(
pwm0d,1,0,do_pwm0_disable,"do_pwm0_disable","-"
);

extern void imx53_pwm0_setval(int val);
int do_pwm0_setval(cmd_tbl_t *cmdtp,int flag,int argc,char *argv[])
{
	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	ulong count = simple_strtoul(argv[1], NULL, 10);

	imx53_pwm0_setval(count);
	imx53_pwm0_enalbe();
}
U_BOOT_CMD(
pwm0,2,1,do_pwm0_setval,"set pwm0 value","pwm0 value"
);
#endif

