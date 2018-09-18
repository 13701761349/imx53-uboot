/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define	DEBUG	*/

// add by zed  for vprintf
#include <common.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <exports.h>
// end by zed  for vprintf

#include <common.h>
#include <watchdog.h>
#include <command.h>
#ifdef CONFIG_MODEM_SUPPORT
#include <malloc.h>		/* for free() prototype */
#endif

#ifdef CONFIG_SYS_HUSH_PARSER
#include <hush.h>
#endif

#include <post.h>



#if defined(CONFIG_SILENT_CONSOLE) || defined(CONFIG_POST) || defined(CONFIG_CMDLINE_EDITING)
DECLARE_GLOBAL_DATA_PTR;
#endif

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
void inline __show_boot_progress (int val) {}
void show_boot_progress (int val) __attribute__((weak, alias("__show_boot_progress")));

#if defined(CONFIG_BOOT_RETRY_TIME) && defined(CONFIG_RESET_TO_RETRY)
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);		/* for do_reset() prototype */
#endif

extern int do_bootd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if defined(CONFIG_UPDATE_TFTP)
void update_tftp (void);
#endif /* CONFIG_UPDATE_TFTP */

#define MAX_DELAY_STOP_STR 32

#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
static int abortboot(int);
#endif

#undef DEBUG_PARSER

char        console_buffer[CONFIG_SYS_CBSIZE];		/* console I/O buffer	*/

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen);
static char erase_seq[] = "\b \b";		/* erase sequence	*/
static char   tab_seq[] = "        ";		/* used to expand TABs	*/

#ifdef CONFIG_BOOT_RETRY_TIME
static uint64_t endtime = 0;  /* must be set, default is instant timeout */
static int      retry_time = -1; /* -1 so can call readline before main_loop */
#endif

#define	endtick(seconds) (get_ticks() + (uint64_t)(seconds) * get_tbclk())

#ifndef CONFIG_BOOT_RETRY_MIN
#define CONFIG_BOOT_RETRY_MIN CONFIG_BOOT_RETRY_TIME
#endif

#ifdef CONFIG_MODEM_SUPPORT
int do_mdm_init = 0;
extern void mdm_init(void); /* defined in board.c */
#endif
// add by panzidong for radom() mac

//#define  __REG8(x)    (*(volatile u8 *)(0x63f98000+0x800+0x0020+x*4))

//#define  __REG8(x)    readl(0x63f98000+0x800+0x0020+(int)x*4)

static unsigned char random(int i)
 {
      unsigned long long rand = get_timer(i) * 100000;
      unsigned char a;

     // See "Numerical Recipes in C", second edition, p. 284
  

     // rand = rand * 1664525L + 1013904223L;
      rand = rand * 1664055525L + 101390400222L;
      a = rand >> 24; //不是最低8位

    //为了得到 ‘0~9, a ~ f, A ~ F’ 的值
 

      if (a < 'A')
           a = a % 10 + 48;
       else if (a < 'F')
           a = a % 6 + 65;
       else if (a < 'a' || a > 'f')
           a = a % 6 + 97;
 

      return a;
  }


void imx53_cpuid(void){
    char hexTchar[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};
    char cpuid[17]={'0',};
    int tmp;
    int i;
    for(i=0;i<8;i++){
    tmp=((*(volatile u8 *)(0x63f98820+i*4)) & 0xf0)>>4;
    cpuid[i*2]= hexTchar[tmp];
    tmp=(*(volatile u8 *)(0x63f98820+i*4)) & 0x0f;
    cpuid[i*2+1]=hexTchar[tmp];
    }
    cpuid[16]='\0';
    printf("imx53 cpu id : %s \n",cpuid);        
}
#ifndef MD5_H  
   #define MD5_H  
     
    typedef struct  
    {  
        unsigned int count[2];  
        unsigned int state[4];  
        unsigned char buffer[64];     
    }MD5_CTX;  
     
     
   #define F(x,y,z) ((x & y) | (~x & z))  
   #define G(x,y,z) ((x & z) | (y & ~z))  
   #define H(x,y,z) (x^y^z)  
   #define I(x,y,z) (y ^ (x | ~z))  
   #define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))  
   #define FF(a,b,c,d,x,s,ac) \  
   { \  
       a += F(b,c,d) + x + ac; \  
       a = ROTATE_LEFT(a,s); \  
       a += b; \  
   }  
   #define GG(a,b,c,d,x,s,ac) \  
  { \  
      a += G(b,c,d) + x + ac; \  
      a = ROTATE_LEFT(a,s); \  
       a += b; \  
   }  
   #define HH(a,b,c,d,x,s,ac) \  
   { \  
       a += H(b,c,d) + x + ac; \  
       a = ROTATE_LEFT(a,s); \  
       a += b; \  
   }  
   #define II(a,b,c,d,x,s,ac) \  
   { \  
       a += I(b,c,d) + x + ac; \  
       a = ROTATE_LEFT(a,s); \  
       a += b; \  
   }                                              
   void MD5Init(MD5_CTX *context);  
   void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);  
   void MD5Final(MD5_CTX *context,unsigned char digest[16]);  
   void MD5Transform(unsigned int state[4],unsigned char block[64]);  
   void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);  
   void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);       
#endif  
unsigned char PADDING[]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
      
   void MD5Init(MD5_CTX *context)  
  {  
       context->count[0] = 0;  
       context->count[1] = 0;  
       context->state[0] = 0x67452301;  
       context->state[1] = 0xEFCDAB89;  
       context->state[2] = 0x98BADCFE;  
       context->state[3] = 0x10325476;  
  }  
  void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen)  
  {  
       unsigned int i = 0,index = 0,partlen = 0;  
       index = (context->count[0] >> 3) & 0x3F;  
       partlen = 64 - index;  
       context->count[0] += inputlen << 3;  
       if(context->count[0] < (inputlen << 3))  
           context->count[1]++;  
       context->count[1] += inputlen >> 29;  
     
       if(inputlen >= partlen)  
       {  
           memcpy(&context->buffer[index],input,partlen);  
           MD5Transform(context->state,context->buffer);  
           for(i = partlen;i+64 <= inputlen;i+=64)  
               MD5Transform(context->state,&input[i]);  
           index = 0;          
       }    
       else  
       {  
           i = 0;  
       }  
       memcpy(&context->buffer[index],&input[i],inputlen-i);  
   }  
   void MD5Final(MD5_CTX *context,unsigned char digest[16])  
   {  
       unsigned int index = 0,padlen = 0;  
       unsigned char bits[8];  
       index = (context->count[0] >> 3) & 0x3F;  
       padlen = (index < 56)?(56-index):(120-index);  
       MD5Encode(bits,context->count,8);  
       MD5Update(context,PADDING,padlen);  
       MD5Update(context,bits,8);  
       MD5Encode(digest,context->state,16);  
   }  
   void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len)  
   {  
       unsigned int i = 0,j = 0;  
       while(j < len)  
       {  
           output[j] = input[i] & 0xFF;    
           output[j+1] = (input[i] >> 8) & 0xFF;  
           output[j+2] = (input[i] >> 16) & 0xFF;  
           output[j+3] = (input[i] >> 24) & 0xFF;  
           i++;  
           j+=4;  
       }  
   }  
   void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len)  
   {  
      unsigned int i = 0,j = 0;  
       while(j < len)  
       {  
           output[i] = (input[j]) |  
              (input[j+1] << 8) |  
              (input[j+2] << 16) |  
              (input[j+3] << 24);  
           i++;  
           j+=4;   
       }  
   }  
  void MD5Transform(unsigned int state[4],unsigned char block[64])  
   {  
       unsigned int a = state[0];  
       unsigned int b = state[1];  
       unsigned int c = state[2];  
       unsigned int d = state[3];  
       unsigned int x[64];  
       MD5Decode(x,block,64);  
       FF(a, b, c, d, x[ 0], 7, 0xd76aa478); /* 1 */  
       FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /* 2 */  
       FF(c, d, a, b, x[ 2], 17, 0x242070db); /* 3 */  
       FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /* 4 */  
       FF(a, b, c, d, x[ 4], 7, 0xf57c0faf); /* 5 */  
       FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /* 6 */  
       FF(c, d, a, b, x[ 6], 17, 0xa8304613); /* 7 */  
       FF(b, c, d, a, x[ 7], 22, 0xfd469501); /* 8 */  
       FF(a, b, c, d, x[ 8], 7, 0x698098d8); /* 9 */  
       FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */  
       FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */  
       FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */  
       FF(a, b, c, d, x[12], 7, 0x6b901122); /* 13 */  
      FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */  
      FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */  
      FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */  
    
      /* Round 2 */  
      GG(a, b, c, d, x[ 1], 5, 0xf61e2562); /* 17 */  
      GG(d, a, b, c, x[ 6], 9, 0xc040b340); /* 18 */  
      GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */  
      GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */  
      GG(a, b, c, d, x[ 5], 5, 0xd62f105d); /* 21 */  
      GG(d, a, b, c, x[10], 9,  0x2441453); /* 22 */  
      GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */  
      GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */  
      GG(a, b, c, d, x[ 9], 5, 0x21e1cde6); /* 25 */  
      GG(d, a, b, c, x[14], 9, 0xc33707d6); /* 26 */  
      GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */  
      GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */  
      GG(a, b, c, d, x[13], 5, 0xa9e3e905); /* 29 */  
      GG(d, a, b, c, x[ 2], 9, 0xfcefa3f8); /* 30 */  
      GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */  
      GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */  
    
      /* Round 3 */  
      HH(a, b, c, d, x[ 5], 4, 0xfffa3942); /* 33 */  
      HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */  
      HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */  
      HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */  
      HH(a, b, c, d, x[ 1], 4, 0xa4beea44); /* 37 */  
      HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */  
      HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */  
      HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */  
      HH(a, b, c, d, x[13], 4, 0x289b7ec6); /* 41 */  
      HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */  
      HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */  
      HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */  
      HH(a, b, c, d, x[ 9], 4, 0xd9d4d039); /* 45 */  
      HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */  
      HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */  
      HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */  
    
      /* Round 4 */  
      II(a, b, c, d, x[ 0], 6, 0xf4292244); /* 49 */  
      II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */  
      II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */  
      II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */  
      II(a, b, c, d, x[12], 6, 0x655b59c3); /* 53 */  
      II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */  
      II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */  
      II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */  
      II(a, b, c, d, x[ 8], 6, 0x6fa87e4f); /* 57 */  
      II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */  
      II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */  
      II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */  
      II(a, b, c, d, x[ 4], 6, 0xf7537e82); /* 61 */  
      II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */  
      II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */  
      II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */  
      state[0] += a;  
      state[1] += b;  
      state[2] += c;  
      state[3] += d;  
  } 



int linux_Encrypt(unsigned char* encrypt, unsigned char* licenses){
 //   if(strlen(encrypt)!=16)
 //   {  
 //      printf("error \n");
 //      return  -1;
 //   }
  unsigned  char KEY[18]={'z',
                  'e',
                  'd',
                  'c',
                  'h',
                  'a',
                  'o',
                  'r',
                  'e',
                  'n',
                  '2',
                  '0',
                  '1',
                  '3',
                  '1',
                  '0',
                  '1',
                  '0',
                  };
  unsigned  char KEY1[18]={
                   'a',
                   'b',
                   'd',
                   'c',
                   'h',
                   'a',
                   'z',
                   'z',
                   '2',
                   '0',
                   '1',
                   '3',
                   '4',
                   '5',
                   '6',
                   '7',
                   '1',
                   '0',
                  };
  unsigned  char KEY2[18]={1,2,3,4,5,6,7,8,
                           9,6,7,5,3,2,4,7,
                           1,2,};
    int i;
    for(i=0;i<16;i++)
      {
         encrypt[i]=(unsigned char)((encrypt[i]^KEY[i])+KEY1[i]+KEY2[i]);     
      }

//     printf("for(i=0;i<16;i++)\n");
//        for(i=0;i<16;i++){
//         printf("%02x\n",encrypt[i]);  
//       }
//     printf("\n");

    for(i=1;i<15;i++)
      {
         encrypt[i]=(unsigned char)(encrypt[i-1]*KEY2[i-1] + encrypt[i+1]*KEY2[i+1] + encrypt[i]*KEY2[i]); 
      }

//      printf("for(i=1;i<15;i++)\n");
//      for(i=0;i<16;i++){
//         printf("%02x\n",encrypt[i]);  
//       }
//     printf("\n");

      encrypt[0]=(unsigned char)(encrypt[15]*KEY2[16] + encrypt[0]*KEY2[0] + encrypt[1]*KEY2[1]);
      encrypt[15]=(unsigned char)(encrypt[14]*KEY2[14] +encrypt[0]*KEY2[0] + encrypt[15]*KEY2[15]);



    return 0;

}
 
void imx53_licenses(void){
     // MD5_CTX md5;
     // MD5Init(&md5);

      int i;
      unsigned char encrypt[16];
      unsigned char decrypt[16];
      unsigned char licenses[32+1];
    char hexTchar[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};
     int tmp;
    for(i=0;i<8;i++){
    tmp=((*(volatile u8 *)(0x63f98820+i*4)) & 0xf0)>>4;
    encrypt[i*2]= hexTchar[tmp];
    tmp=(*(volatile u8 *)(0x63f98820+i*4)) & 0x0f;
    encrypt[i*2+1]=hexTchar[tmp];
    }

     // MD5Update(&md5,encrypt,strlen((char *)encrypt));
     // MD5Final(&md5,decrypt);
      printf("before encrypt:%s\n after encrypt:\n",encrypt);

      linux_Encrypt(encrypt,licenses);
      for(i=0;i<16;i++)
      {
         //printf("%02x",decrypt[i]);        
         licenses[2*i]=hexTchar[(encrypt[i]&0xf0)>>4];
         licenses[2*i+1]=hexTchar[encrypt[i]&0xf];
          //
        //  licenses[2*i]=hexTchar[(decrypt[i]&0xf0)>>4];
        //  licenses[2*i+1]=hexTchar[decrypt[i]&0xf];
       
      }
      licenses[32]='\0';
//              printf("licenses=%s\n",licenses);
      printf("\n");
        char  *s;
       if ((s = getenv ("licenses")) != NULL) {
                //load_addr = simple_strtoul (s, NULL, 16);
  //              printf("env_licneses=%s\n",s);
     }
      for(i=0;i<32;i++){
         if(s[i]!=licenses[i]){ 
           printf("licenses error \n");   
           while(1);
          }
      }
      printf("licenses pass\n");
}


// add by panzidong for set  radom mac

void autoset_mac_addr(void)
  {

        printf("one   \n");
        char s;
        int sysready;

        s = getenv("sysready");
        printf("sysready= %c \n",s);
      //  sysready = s ? (int)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;
      //  if(s != 'b')
      //  sysready = 0;
      //  printf("sysready= %d   \n",sysready);
      //  if (sysready != 1) /* system not ready */
       if(s == NULL)
        {
              printf("two  \n");
             /*set mac addr */
             int tmp;
             char hexTchar[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};
             char mac[18]={'0','0',};
             mac[2] = mac[5] = mac[8] = mac[11] = mac[14] = ':';

             tmp=((*(volatile u8 *)0x63f98820) & 0xf0)>>4;
             printf("tmp=%d \n",tmp);
               mac[0]= hexTchar[tmp];
               mac[0]= hexTchar[0];
              printf("four  \n");
             tmp=(*(volatile u8 *)0x63f98820) & 0x0f;
             printf("tmp=%d \n",tmp);
              printf("five \n");
             mac[1]=hexTchar[tmp];
             mac[1]=hexTchar[0];

              

             tmp=((*(volatile u8 *)0x63f98824) & 0xf0)>>4;
             printf("tmp=%d \n",tmp);
               mac[3]= hexTchar[tmp];
              printf("four  \n");
             tmp=(*(volatile u8 *)0x63f98824) & 0x0f;
             printf("tmp=%d \n",tmp);
              printf("five \n");
             mac[4]=hexTchar[tmp];




             tmp=((*(volatile u8 *)0x63f98828) & 0xf0)>>4;
             printf("tmp=%d \n",tmp);
               mac[6]= hexTchar[tmp];
              printf("four  \n");
             tmp=(*(volatile u8 *)0x63f98828) & 0x0f;
             printf("tmp=%d \n",tmp);
              printf("five \n");
             mac[7]=hexTchar[tmp];




             tmp=((*(volatile u8 *)0x63f9882c) & 0xf0)>>4;
             printf("tmp=%d \n",tmp);
               mac[9]= hexTchar[tmp];
              printf("four  \n");
             tmp=(*(volatile u8 *)0x63f9882c) & 0x0f;
             printf("tmp=%d \n",tmp);
              printf("five \n");
             mac[10]=hexTchar[tmp];


             tmp=((*(volatile u8 *)0x63f98830) & 0xf0)>>4;
             printf("tmp=%d \n",tmp);
               mac[12]= hexTchar[tmp];
              printf("four  \n");
             tmp=(*(volatile u8 *)0x63f98830) & 0x0f;
             printf("tmp=%d \n",tmp);
              printf("five \n");
             mac[13]=hexTchar[tmp];


             tmp=((*(volatile u8 *)0x63f98834) & 0xf0)>>4;
             printf("tmp=%d \n",tmp);
               mac[15]= hexTchar[tmp];
              printf("four  \n");
             tmp=(*(volatile u8 *)0x63f98834) & 0x0f;
             printf("tmp=%d \n",tmp);
              printf("five \n");
             mac[16]=hexTchar[tmp];
             mac[17] = '\0';
             printf("333333333333333333333\n");
             printf("mac =%s  \n",mac);
             setenv("ethaddr", mac);
             setenv("sysready", "1");
             saveenv();
        }

       //  unsigned long ethaddr_low, ethaddr_high;

       // srand(time(0) | getpid());
       // ethaddr_high = (rand() & 0xfeff) | 0x0200;
       // ethaddr_low = rand();

       // printf("%02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n",
       // ethaddr_high >> 8, ethaddr_high & 0xff,
       // ethaddr_low >> 24, (ethaddr_low >> 16) & 0xff,
       // (ethaddr_low >> 8) & 0xff, ethaddr_low & 0xff);


  }
// end by panzidong for radom() mac


/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot
 *          1 - got key string, abort
 */
#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
# if defined(CONFIG_AUTOBOOT_KEYED)
static __inline__ int abortboot(int bootdelay)
{
	int abort = 0;
	uint64_t etime = endtick(bootdelay);
	struct {
		char* str;
		u_int len;
		int retry;
	}
	delaykey [] = {
		{ str: getenv ("bootdelaykey"),  retry: 1 },
		{ str: getenv ("bootdelaykey2"), retry: 1 },
		{ str: getenv ("bootstopkey"),   retry: 0 },
		{ str: getenv ("bootstopkey2"),  retry: 0 },
	};

	char presskey [MAX_DELAY_STOP_STR];
	u_int presskey_len = 0;
	u_int presskey_max = 0;
	u_int i;

#  ifdef CONFIG_AUTOBOOT_PROMPT
	printf(CONFIG_AUTOBOOT_PROMPT);
#  endif

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	if (delaykey[0].str == NULL)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_DELAY_STR2
	if (delaykey[1].str == NULL)
		delaykey[1].str = CONFIG_AUTOBOOT_DELAY_STR2;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	if (delaykey[2].str == NULL)
		delaykey[2].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR2
	if (delaykey[3].str == NULL)
		delaykey[3].str = CONFIG_AUTOBOOT_STOP_STR2;
#  endif

	for (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i ++) {
		delaykey[i].len = delaykey[i].str == NULL ?
				    0 : strlen (delaykey[i].str);
		delaykey[i].len = delaykey[i].len > MAX_DELAY_STOP_STR ?
				    MAX_DELAY_STOP_STR : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

#  if DEBUG_BOOTKEYS
		printf("%s key:<%s>\n",
		       delaykey[i].retry ? "delay" : "stop",
		       delaykey[i].str ? delaykey[i].str : "NULL");
#  endif
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	do {
		if (tstc()) {
			if (presskey_len < presskey_max) {
				presskey [presskey_len ++] = getc();
			}
			else {
				for (i = 0; i < presskey_max - 1; i ++)
					presskey [i] = presskey [i + 1];

				presskey [i] = getc();
			}
		}

		for (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i ++) {
			if (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
			    memcmp (presskey + presskey_len - delaykey[i].len,
				    delaykey[i].str,
				    delaykey[i].len) == 0) {
#  if DEBUG_BOOTKEYS
				printf("got %skey\n",
				       delaykey[i].retry ? "delay" : "stop");
#  endif

#  ifdef CONFIG_BOOT_RETRY_TIME
				/* don't retry auto boot */
				if (! delaykey[i].retry)
					retry_time = -1;
#  endif
				abort = 1;
			}
		}
	} while (!abort && get_ticks() <= etime);

#  if DEBUG_BOOTKEYS
	if (!abort)
		puts("key timeout\n");
#  endif

#ifdef CONFIG_SILENT_CONSOLE
	if (abort)
		gd->flags &= ~GD_FLG_SILENT;
#endif

	return abort;
}

# else	/* !defined(CONFIG_AUTOBOOT_KEYED) */

#ifdef CONFIG_MENUKEY
static int menukey = 0;
#endif

static __inline__ int abortboot(int bootdelay)
{
	int abort = 0;

#ifdef CONFIG_MENUPROMPT
	printf(CONFIG_MENUPROMPT);
#else
	printf("Hit any key to stop autoboot: %2d ", bootdelay);
#endif
	// add by panzidong
	// autoset_mac_addr();
	// add by panzidong
#if defined CONFIG_ZERO_BOOTDELAY_CHECK
	/*
	 * Check if key already pressed
	 * Don't check if bootdelay < 0
	 */
	if (bootdelay >= 0) {
		if (tstc()) {	/* we got a key press	*/
			(void) getc();  /* consume input	*/
			puts ("\b\b\b 0");
			abort = 1;	/* don't auto boot	*/
		}
	}
#endif

	while ((bootdelay > 0) && (!abort)) {
		int i;

		--bootdelay;
		/* delay 100 * 10ms */
		for (i=0; !abort && i<100; ++i) {
			if (tstc()) {	/* we got a key press	*/
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
# ifdef CONFIG_MENUKEY
				menukey = getc();
# else
				(void) getc();  /* consume input	*/
# endif
				break;
			}
			udelay(10000);
		}

		printf("\b\b\b%2d ", bootdelay);
	}

	putc('\n');

#ifdef CONFIG_SILENT_CONSOLE
	if (abort)
		gd->flags &= ~GD_FLG_SILENT;
#endif

	return abort;
}
# endif	/* CONFIG_AUTOBOOT_KEYED */
#endif	/* CONFIG_BOOTDELAY >= 0  */

/****************************************************************************/

void main_loop (void)
{


printf("------------------------------%s,%d,\n",__FUNCTION__,__LINE__);

#ifndef CONFIG_SYS_HUSH_PARSER
	static char lastcommand[CONFIG_SYS_CBSIZE] = { 0, };
	int len;
	int rc = 1;
	int flag;
#endif

#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	char *s;
	int bootdelay;
#endif
#ifdef CONFIG_PREBOOT
	char *p;
#endif
#ifdef CONFIG_BOOTCOUNT_LIMIT
	unsigned long bootcount = 0;
	unsigned long bootlimit = 0;
	char *bcs;
	char bcs_set[16];
#endif /* CONFIG_BOOTCOUNT_LIMIT */

#if defined(CONFIG_VFD) && defined(VFD_TEST_LOGO)
	ulong bmp = 0;		/* default bitmap */
	extern int trab_vfd (ulong bitmap);

#ifdef CONFIG_MODEM_SUPPORT
	if (do_mdm_init)
		bmp = 1;	/* alternate bitmap */
#endif
	trab_vfd (bmp);
#endif	/* CONFIG_VFD && VFD_TEST_LOGO */

#if defined(CONFIG_UPDATE_TFTP)
	update_tftp ();
#endif /* CONFIG_UPDATE_TFTP */

#ifdef CONFIG_BOOTCOUNT_LIMIT
	bootcount = bootcount_load();
	bootcount++;
	bootcount_store (bootcount);
	sprintf (bcs_set, "%lu", bootcount);
	setenv ("bootcount", bcs_set);
	bcs = getenv ("bootlimit");
	bootlimit = bcs ? simple_strtoul (bcs, NULL, 10) : 0;
#endif /* CONFIG_BOOTCOUNT_LIMIT */

#ifdef CONFIG_MODEM_SUPPORT
	debug ("DEBUG: main_loop:   do_mdm_init=%d\n", do_mdm_init);
	if (do_mdm_init) {
		char *str = strdup(getenv("mdm_cmd"));
		setenv ("preboot", str);  /* set or delete definition */
		if (str != NULL)
			free (str);
		mdm_init(); /* wait for modem connection */
	}
#endif  /* CONFIG_MODEM_SUPPORT */

#ifdef CONFIG_VERSION_VARIABLE
	{
		extern char version_string[];

		setenv ("ver", version_string);  /* set version variable */
	}
#endif /* CONFIG_VERSION_VARIABLE */

#ifdef CONFIG_SYS_HUSH_PARSER
	u_boot_hush_start ();
#endif

#if defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var ();
#endif

#ifdef CONFIG_AUTO_COMPLETE
	install_auto_complete();
#endif

#ifdef CONFIG_PREBOOT
	if ((p = getenv ("preboot")) != NULL) {
# ifdef CONFIG_AUTOBOOT_KEYED
		int prev = disable_ctrlc(1);	/* disable Control C checking */
# endif

# ifndef CONFIG_SYS_HUSH_PARSER
		run_command (p, 0);
# else
		parse_string_outer(p, FLAG_PARSE_SEMICOLON |
				    FLAG_EXIT_FROM_LOOP);
# endif

# ifdef CONFIG_AUTOBOOT_KEYED
		disable_ctrlc(prev);	/* restore Control C checking */
# endif
	}
#endif /* CONFIG_PREBOOT */

#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	s = getenv ("bootdelay");
	bootdelay = s ? (int)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;

	debug ("### main_loop entered: bootdelay=%d\n\n", bootdelay);

# ifdef CONFIG_BOOT_RETRY_TIME
	init_cmd_timeout ();
# endif	/* CONFIG_BOOT_RETRY_TIME */

#ifdef CONFIG_POST
	if (gd->flags & GD_FLG_POSTFAIL) {
		s = getenv("failbootcmd");
	}
	else
#endif /* CONFIG_POST */
#ifdef CONFIG_BOOTCOUNT_LIMIT
	if (bootlimit && (bootcount > bootlimit)) {
		printf ("Warning: Bootlimit (%u) exceeded. Using altbootcmd.\n",
		        (unsigned)bootlimit);
		s = getenv ("altbootcmd");
	}
	else
#endif /* CONFIG_BOOTCOUNT_LIMIT */
		s = getenv ("bootcmd");

	debug ("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");





	if (bootdelay >= 0 && s && !abortboot (bootdelay)) {
# ifdef CONFIG_AUTOBOOT_KEYED
		int prev = disable_ctrlc(1);	/* disable Control C checking */
# endif

# ifndef CONFIG_SYS_HUSH_PARSER
		run_command (s, 0);
# else
		parse_string_outer(s, FLAG_PARSE_SEMICOLON |
				    FLAG_EXIT_FROM_LOOP);
# endif

# ifdef CONFIG_AUTOBOOT_KEYED
		disable_ctrlc(prev);	/* restore Control C checking */
# endif
	}



# ifdef CONFIG_MENUKEY
	if (menukey == CONFIG_MENUKEY) {
	    s = getenv("menucmd");
	    if (s) {
# ifndef CONFIG_SYS_HUSH_PARSER
		run_command (s, 0);
# else
		parse_string_outer(s, FLAG_PARSE_SEMICOLON |
				    FLAG_EXIT_FROM_LOOP);
# endif
	    }
	}
#endif /* CONFIG_MENUKEY */
#endif	/* CONFIG_BOOTDELAY */

#ifdef CONFIG_AMIGAONEG3SE
	{
	    extern void video_banner(void);
	    video_banner();
	}
#endif

	/*
	 * Main Loop for Monitor Command Processing
	 */
#ifdef CONFIG_SYS_HUSH_PARSER
	parse_file_outer();
	/* This point is never reached */
	for (;;);
#else
	for (;;) {
		//printf("------%s,%d,\n",__FUNCTION__,__LINE__);
#ifdef CONFIG_BOOT_RETRY_TIME
		if (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			reset_cmd_timeout();
		}
#endif

		len = readline (CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		if (len > 0)
			strcpy (lastcommand, console_buffer);
		else if (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts ("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset (NULL, 0, 0, NULL);
# else
			return;		/* retry autoboot */
# endif
		}
#endif

		if (len == -1)
			puts ("<INTERRUPT>\n");
		else
			rc = run_command (lastcommand, flag);

		if (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
#endif /*CONFIG_SYS_HUSH_PARSER*/
}

#ifdef CONFIG_BOOT_RETRY_TIME
/***************************************************************************
 * initialize command line timeout
 */
void init_cmd_timeout(void)
{
	char *s = getenv ("bootretry");

	if (s != NULL)
		retry_time = (int)simple_strtol(s, NULL, 10);
	else
		retry_time =  CONFIG_BOOT_RETRY_TIME;

	if (retry_time >= 0 && retry_time < CONFIG_BOOT_RETRY_MIN)
		retry_time = CONFIG_BOOT_RETRY_MIN;
}

/***************************************************************************
 * reset command line timeout to retry_time seconds
 */
void reset_cmd_timeout(void)
{
	endtime = endtick(retry_time);
}
#endif

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str,n)	do {			\
		printf ("%.*s", (int)n, str);	\
	} while (0)

#define CTL_CH(c)		((c) - 'a' + 1)

#define MAX_CMDBUF_SIZE		256

#define CTL_BACKSPACE		('\b')
#define DEL			((char)255)
#define DEL7			((char)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getc()
#define getcmd_cbeep()		getcmd_putch('\a')

#define HIST_MAX		20
#define HIST_SIZE		MAX_CMDBUF_SIZE

static int hist_max = 0;
static int hist_add_idx = 0;
static int hist_cur = -1;
unsigned hist_num = 0;

char* hist_list[HIST_MAX];
char hist_lines[HIST_MAX][HIST_SIZE];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void hist_init(void)
{
	int i;

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	for (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_lines[i];
		hist_list[i][0] = '\0';
	}
}

static void cread_add_to_hist(char *line)
{
	strcpy(hist_list[hist_add_idx], line);

	if (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	if (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static char* hist_prev(void)
{
	char *ret;
	int old_cur;

	if (hist_cur < 0)
		return NULL;

	old_cur = hist_cur;
	if (--hist_cur < 0)
		hist_cur = hist_max;

	if (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} else
		ret = hist_list[hist_cur];

	return (ret);
}

static char* hist_next(void)
{
	char *ret;

	if (hist_cur < 0)
		return NULL;

	if (hist_cur == hist_add_idx)
		return NULL;

	if (++hist_cur > hist_max)
		hist_cur = 0;

	if (hist_cur == hist_add_idx) {
		ret = "";
	} else
		ret = hist_list[hist_cur];

	return (ret);
}

#ifndef CONFIG_CMDLINE_EDITING
static void cread_print_hist_list(void)
{
	int i;
	unsigned long n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	while (1) {
		if (i > hist_max)
			i = 0;
		if (i == hist_add_idx)
			break;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}
#endif /* CONFIG_CMDLINE_EDITING */

#define BEGINNING_OF_LINE() {			\
	while (num) {				\
		getcmd_putch(CTL_BACKSPACE);	\
		num--;				\
	}					\
}

#define ERASE_TO_EOL() {				\
	if (num < eol_num) {				\
		int tmp;				\
		for (tmp = num; tmp < eol_num; tmp++)	\
			getcmd_putch(' ');		\
		while (tmp-- > num)			\
			getcmd_putch(CTL_BACKSPACE);	\
		eol_num = num;				\
	}						\
}

#define REFRESH_TO_EOL() {			\
	if (num < eol_num) {			\
		wlen = eol_num - num;		\
		putnstr(buf + num, wlen);	\
		num = eol_num;			\
	}					\
}

static void cread_add_char(char ichar, int insert, unsigned long *num,
	       unsigned long *eol_num, char *buf, unsigned long len)
{
	unsigned long wlen;

	/* room ??? */
	if (insert || *num == *eol_num) {
		if (*eol_num > len - 1) {
			getcmd_cbeep();
			return;
		}
		(*eol_num)++;
	}

	if (insert) {
		wlen = *eol_num - *num;
		if (wlen > 1) {
			memmove(&buf[*num+1], &buf[*num], wlen-1);
		}

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		while (--wlen) {
			getcmd_putch(CTL_BACKSPACE);
		}
	} else {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(char *str, int strsize, int insert, unsigned long *num,
	      unsigned long *eol_num, char *buf, unsigned long len)
{
	while (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

//add by ljs
extern void imx53_feed_watchdog(void);
//end by ljs
static int cread_line(const char *const prompt, char *buf, unsigned int *len)
{

	//return 0;
	unsigned long num = 0;
	unsigned long eol_num = 0;
	unsigned long rlen;
	unsigned long wlen;
	char ichar;
	int insert = 1;
	int esc_len = 0;
	int rc = 0;
	char esc_save[8];

	while (1) {

		//printf("------%s,%d,\n",__FUNCTION__,__LINE__);
		rlen = 1;

		while (!tstc()) {	/* while no incoming data */
				//udelay(1000*100);
				//printf("+++++++%s,%d,\n",__FUNCTION__,__LINE__);
				imx53_feed_watchdog();
		}

		//CONFIG_BOOT_RETRY_TIME,add by ljs
		#ifdef CONFIG_BOOT_RETRY_TIME
					while (!tstc()) {	/* while no incoming data */
						if (retry_time >= 0 && get_ticks() > endtime)
							return (-2);	/* timed out */
					}
		#endif
		//end by ljs


		ichar = getcmd_getch();

		if ((ichar == '\n') || (ichar == '\r')) {
			putc('\n');
			break;
		}

		/*
		 * handle standard linux xterm esc sequences for arrow key, etc.
		 */
		if (esc_len != 0) {
			if (esc_len == 1) {
				if (ichar == '[') {
					esc_save[esc_len] = ichar;
					esc_len = 2;
				} else {
					cread_add_str(esc_save, esc_len, insert,
						      &num, &eol_num, buf, *len);
					esc_len = 0;
				}
				continue;
			}

			switch (ichar) {

			case 'D':	/* <- key */
				ichar = CTL_CH('b');
				esc_len = 0;
				break;
			case 'C':	/* -> key */
				ichar = CTL_CH('f');
				esc_len = 0;
				break;	/* pass off to ^F handler */
			case 'H':	/* Home key */
				ichar = CTL_CH('a');
				esc_len = 0;
				break;	/* pass off to ^A handler */
			case 'A':	/* up arrow */
				ichar = CTL_CH('p');
				esc_len = 0;
				break;	/* pass off to ^P handler */
			case 'B':	/* down arrow */
				ichar = CTL_CH('n');
				esc_len = 0;
				break;	/* pass off to ^N handler */
			default:
				esc_save[esc_len++] = ichar;
				cread_add_str(esc_save, esc_len, insert,
					      &num, &eol_num, buf, *len);
				esc_len = 0;
				continue;
			}
		}

		switch (ichar) {
		case 0x1b:
			if (esc_len == 0) {
				esc_save[esc_len] = ichar;
				esc_len = 1;
			} else {
				puts("impossible condition #876\n");
				esc_len = 0;
			}
			break;

		case CTL_CH('a'):
			BEGINNING_OF_LINE();
			break;
		case CTL_CH('c'):	/* ^C - break */
			*buf = '\0';	/* discard input */
			return (-1);
		case CTL_CH('f'):
			if (num < eol_num) {
				getcmd_putch(buf[num]);
				num++;
			}
			break;
		case CTL_CH('b'):
			if (num) {
				getcmd_putch(CTL_BACKSPACE);
				num--;
			}
			break;
		case CTL_CH('d'):
			if (num < eol_num) {
				wlen = eol_num - num - 1;
				if (wlen) {
					memmove(&buf[num], &buf[num+1], wlen);
					putnstr(buf + num, wlen);
				}

				getcmd_putch(' ');
				do {
					getcmd_putch(CTL_BACKSPACE);
				} while (wlen--);
				eol_num--;
			}
			break;
		case CTL_CH('k'):
			ERASE_TO_EOL();
			break;
		case CTL_CH('e'):
			REFRESH_TO_EOL();
			break;
		case CTL_CH('o'):
			insert = !insert;
			break;
		case CTL_CH('x'):
		case CTL_CH('u'):
			BEGINNING_OF_LINE();
			ERASE_TO_EOL();
			break;
		case DEL:
		case DEL7:
		case 8:
			if (num) {
				wlen = eol_num - num;
				num--;
				memmove(&buf[num], &buf[num+1], wlen);
				getcmd_putch(CTL_BACKSPACE);
				putnstr(buf + num, wlen);
				getcmd_putch(' ');
				do {
					getcmd_putch(CTL_BACKSPACE);
				} while (wlen--);
				eol_num--;
			}
			break;
		case CTL_CH('p'):
		case CTL_CH('n'):
		{
			char * hline;

			esc_len = 0;

			if (ichar == CTL_CH('p'))
				hline = hist_prev();
			else
				hline = hist_next();

			if (!hline) {
				getcmd_cbeep();
				continue;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_LINE();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			eol_num = strlen(buf);
			REFRESH_TO_EOL();
			continue;
		}
#ifdef CONFIG_AUTO_COMPLETE
		case '\t': {
			int num2, col;

			/* do not autocomplete when in the middle */
			if (num < eol_num) {
				getcmd_cbeep();
				break;
			}

			buf[num] = '\0';
			col = strlen(prompt) + eol_num;
			num2 = num;
			if (cmd_auto_complete(prompt, buf, &num2, &col)) {
				col = num2 - num;
				num += col;
				eol_num += col;
			}
			break;
		}
#endif
		default:
			cread_add_char(ichar, insert, &num, &eol_num, buf, *len);
			break;
		}
	}
	*len = eol_num;
	buf[eol_num] = '\0';	/* lose the newline */

	if (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	return (rc);
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

/*
 * Prompt for input and read a line.
 * If  CONFIG_BOOT_RETRY_TIME is defined and retry_time >= 0,
 * time out when time goes past endtime (timebase time in ticks).
 * Return:	number of read characters
 *		-1 if break
 *		-2 if timed out
 */
int readline (const char *const prompt)
{
	return readline_into_buffer(prompt, console_buffer);
}


int readline_into_buffer (const char *const prompt, char * buffer)
{
	char *p = buffer;
#ifdef CONFIG_CMDLINE_EDITING
	unsigned int len=MAX_CMDBUF_SIZE;
	int rc;
	static int initted = 0;

	/*
	 * History uses a global array which is not
	 * writable until after relocation to RAM.
	 * Revert to non-history version if still
	 * running from flash.
	 */
	if (gd->flags & GD_FLG_RELOC) {
		if (!initted) {
			hist_init();
			initted = 1;
		}

		puts (prompt);

		rc = cread_line(prompt, p, &len);
		return rc < 0 ? rc : len;

	} else {
#endif	/* CONFIG_CMDLINE_EDITING */
	char * p_buf = p;
	int	n = 0;				/* buffer index		*/
	int	plen = 0;			/* prompt length	*/
	int	col;				/* output column cnt	*/
	char	c;

	/* print prompt */
	if (prompt) {
		plen = strlen (prompt);
		puts (prompt);
	}
	col = plen;

	for (;;) {
#ifdef CONFIG_BOOT_RETRY_TIME
		while (!tstc()) {	/* while no incoming data */
		//cancel by ljs
			if (retry_time >= 0 && get_ticks() > endtime)
				return (-2);	/* timed out */
		}
#endif
		//printf("------%s,%d,\n",__FUNCTION__,__LINE__);
		WATCHDOG_RESET();		/* Trigger watchdog, if needed */

#ifdef CONFIG_SHOW_ACTIVITY
		while (!tstc()) {
			extern void show_activity(int arg);
			show_activity(0);
		}
#endif
		//printf("------%s,%d,\n",__FUNCTION__,__LINE__);
		c = getc();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':				/* Enter		*/
		case '\n':
			*p = '\0';
			puts ("\r\n");
			return (p - p_buf);

		case '\0':				/* nul			*/
			continue;

		case 0x03:				/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			return (-1);

		case 0x15:				/* ^U - erase line	*/
			while (col > plen) {
				puts (erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:				/* ^W - erase word	*/
			p=delete_char(p_buf, p, &col, &n, plen);
			while ((n > 0) && (*p != ' ')) {
				p=delete_char(p_buf, p, &col, &n, plen);
			}
			continue;

		case 0x08:				/* ^H  - backspace	*/
		case 0x7F:				/* DEL - backspace	*/
			p=delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			//printf("------%s,%d,\n",__FUNCTION__,__LINE__);
			/*
			 * Must be a normal character then
			 */
			if (n < CONFIG_SYS_CBSIZE-2) {
				if (c == '\t') {	/* expand TABs		*/
#ifdef CONFIG_AUTO_COMPLETE
					/* if auto completion triggered just continue */
					*p = '\0';
					if (cmd_auto_complete(prompt, console_buffer, &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
#endif
					puts (tab_seq+(col&07));
					col += 8 - (col&07);
				} else {
					++col;		/* echo input		*/
					putc (c);
				}
				*p++ = c;
				++n;
			} else {			/* Buffer full		*/
				putc ('\a');
			}
		}
	}
#ifdef CONFIG_CMDLINE_EDITING
	}
#endif
}

/****************************************************************************/

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0) {
		return (p);
	}

	if (*(--p) == '\t') {			/* will retype the whole line	*/
		while (*colp > plen) {
			puts (erase_seq);
			(*colp)--;
		}
		for (s=buffer; s<p; ++s) {
			if (*s == '\t') {
				puts (tab_seq+((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				putc (*s);
			}
		}
	} else {
		puts (erase_seq);
		(*colp)--;
	}
	(*np)--;
	return (p);
}

/****************************************************************************/

int parse_line (char *line, char *argv[])
{
	int nargs = 0;

#ifdef DEBUG_PARSER
	printf ("parse_line: \"%s\"\n", line);
#endif
	while (nargs < CONFIG_SYS_MAXARGS) {

		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf ("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

#ifdef DEBUG_PARSER
	printf ("parse_line: nargs=%d\n", nargs);
#endif
	return (nargs);
}

/****************************************************************************/

static void process_macros (const char *input, char *output)
{
	char c, prev;
	const char *varname_start = NULL;
	int inputcnt = strlen (input);
	int outputcnt = CONFIG_SYS_CBSIZE;
	int state = 0;		/* 0 = waiting for '$'  */

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
#ifdef DEBUG_PARSER
	char *output_start = output;

	printf ("[PROCESS_MACROS] INPUT len %d: \"%s\"\n", strlen (input),
		input);
#endif

	prev = '\0';		/* previous character   */

	while (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		if (state != 3) {
			/* remove one level of escape characters */
			if ((c == '\\') && (prev != '\\')) {
				if (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			if ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			if ((c == '$') && (prev != '\\')) {
				state++;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			if (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} else {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				if (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			if (c == ')' || c == '}') {
				int i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				int envcnt = input - varname_start - 1;	/* Varname # of chars */

				/* Get the varname */
				for (i = 0; i < envcnt; i++) {
					envname[i] = varname_start[i];
				}
				envname[i] = 0;

				/* Get its value */
				envval = getenv (envname);

				/* Copy into the line if it exists */
				if (envval != NULL)
					while ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			if ((c == '\'') && (prev != '\\')) {
				state = 0;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	if (outputcnt)
		*output = 0;
	else
		*(output - 1) = 0;

#ifdef DEBUG_PARSER
	printf ("[PROCESS_MACROS] OUTPUT len %d: \"%s\"\n",
		strlen (output_start), output_start);
#endif
}

/****************************************************************************
 * returns:
 *	1  - command executed, repeatable
 *	0  - command executed but not repeatable, interrupted commands are
 *	     always considered not repeatable
 *	-1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CONFIG_SYS_CBSIZE-1 it is
 *           considered unrecognized)
 *
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from getenv(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */

int run_command (const char *cmd, int flag)
{
	cmd_tbl_t *cmdtp;
	char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CONFIG_SYS_CBSIZE];
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	int argc, inquotes;
	int repeatable = 1;
	int rc = 0;

#ifdef DEBUG_PARSER
	printf ("[RUN_COMMAND] cmd[%p]=\"", cmd);
	puts (cmd ? cmd : "NULL");	/* use puts - string may be loooong */
	puts ("\"\n");
#endif

	clear_ctrlc();		/* forget any previous Control C */

	if (!cmd || !*cmd) {
		return -1;	/* empty command */
	}

	if (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts ("## Command too long!\n");
		return -1;
	}

	strcpy (cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

#ifdef DEBUG_PARSER
	printf ("[PROCESS_SEPARATORS] %s\n", cmd);
#endif
	while (*str) {

		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			if ((*sep=='\'') &&
			    (*(sep-1) != '\\'))
				inquotes=!inquotes;

			if (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    ( sep != str) &&	/* past string start	*/
			    (*(sep-1) != '\\'))	/* and NOT escaped	*/
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		if (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		}
		else
			str = sep;	/* no more commands for next pass */
#ifdef DEBUG_PARSER
		printf ("token: \"%s\"\n", token);
#endif

		/* find macros in this token and replace them */
		process_macros (token, finaltoken);

		/* Extract arguments */
		if ((argc = parse_line (finaltoken, argv)) == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		/* Look up command in command table */
		if ((cmdtp = find_cmd(argv[0])) == NULL) {
			printf ("Unknown command '%s' - try 'help'\n", argv[0]);
			rc = -1;	/* give up after bad command */
			continue;
		}

		/* found - check max args */
		if (argc > cmdtp->maxargs) {
			cmd_usage(cmdtp);
			rc = -1;
			continue;
		}

#if defined(CONFIG_CMD_BOOTD)
		/* avoid "bootd" recursion */
		if (cmdtp->cmd == do_bootd) {
#ifdef DEBUG_PARSER
			printf ("[%s]\n", finaltoken);
#endif
			if (flag & CMD_FLAG_BOOTD) {
				puts ("'bootd' recursion detected\n");
				rc = -1;
				continue;
			} else {
				flag |= CMD_FLAG_BOOTD;
			}
		}
#endif

		/* OK - call function to do the command */
		if ((cmdtp->cmd) (cmdtp, flag, argc, argv) != 0) {
			rc = -1;
		}

		repeatable &= cmdtp->repeatable;

		/* Did the user stop this? */
		if (had_ctrlc ())
			return -1;	/* if stopped then not repeatable */
	}

	return rc ? rc : repeatable;
}

/****************************************************************************/

#if defined(CONFIG_CMD_RUN)
int do_run (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	for (i=1; i<argc; ++i) {
		char *arg;

		if ((arg = getenv (argv[i])) == NULL) {
			printf ("## Error: \"%s\" not defined\n", argv[i]);
			return 1;
		}
#ifndef CONFIG_SYS_HUSH_PARSER
		if (run_command (arg, flag) == -1)
			return 1;
#else
		if (parse_string_outer(arg,
		    FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP) != 0)
			return 1;
#endif
	}
	return 0;
}
#endif
