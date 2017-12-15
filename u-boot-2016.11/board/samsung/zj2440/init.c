/*added by zhj
  nand flash init
*/
#include "init.h"

#define TACLES 0x3
#define TWRPH0 0x3
#define TWRPH1 0x7

void nandflash_init(void)
{
	put_c('a');
	//put_s("hello c program\r\n");
	/*setup TACLS =0  TWRPH0 = 1 TWRPH1 = 0*/
	unsigned int tmp = NFCONF;
	tmp &= ~((TACLES << 12) | (TWRPH0 << 8) | (TWRPH1 << 4));
	tmp |= (0x1 << 8);
	
	/*8-bit bus*/
	tmp &= ~(1 << 0);
	NFCONF = tmp;
	
	/*disable ecc*/
	tmp = NFCONT;
	/*enable nand controler, disable chip select*/
	tmp |= ((1 << 0) | (1 << 1) | (1 << 4));
	NFCONT = tmp;
}

/*nand flash stat*/
int nand_stat(void)
{
	unsigned int tmp = NFSTAT;
	if(tmp & 0x1)	/*ready*/
	{
		return 0;
	}
	else			/*busy*/
	{
		return 1;
	}
}
int is_nor_or_nand(void)
{
	volatile unsigned int* addr = (volatile unsigned int*)0x0;
	unsigned int tmp = *addr;
	*addr = 0x12345678;
	if (*addr == 0x12345678)	/*从nand启动*/
	{
		*addr = tmp;
		return 1;
	}
	else				/*从 nor启动*/
	{
		return 0;	
	}
}

void select_chip(void)
{
	/*
	unsigned int tmp;
	tmp = NFCONT;
	tmp &= ~(1 << 1);
	NFCONT = tmp;
	*/
	NFCONT &= ~(1 << 1);
}

void disselect_chip(void)
{
	/*
	unsigned int tmp;
	tmp = NFCONT;
	tmp |= (1 << 1);
	NFCONT = tmp;
	*/
	NFCONT |= (1<<1);
}

static void nand_wait_ready(void)
{
	while (!(NFSTAT & 1));
}

unsigned char nand_data(void)
{
	return NFDATA;
}

void nand_cmd(unsigned char cmd)
{	
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);
}

void nand_addr(unsigned     int addr)
{

	unsigned int i;
	unsigned short col = addr % 2048;
	unsigned short row = addr / 2048;
	/*Column Address*/
	NFADDR = (col & 0xFF);
	for(i = 0; i < 1024; i++);
	NFADDR = ((col >> 8) & 0x7);
	for(i = 0; i < 1024; i++);
	/*Row Address*/
	NFADDR = (row & 0xFF);
	for(i = 0; i < 1024; i++);
	NFADDR = ((row >> 8) & 0xFF);
	for(i = 0; i < 1024; i++);
	NFADDR = ((row >> 16) & 0x1);	

}

void copy_code_from_nand(unsigned char* src, unsigned char* dst, unsigned int len)
{
	unsigned int i= 0;
	unsigned int col = ((unsigned int)src) % 2048;

	put_c('e');
	select_chip();
	while (i < len)
	{
		nand_cmd(0x00);
		nand_addr((unsigned int)src);
		nand_cmd(0x30);
		while(nand_stat());		/*busy*/
		/*i应该控制的是写入的地址，
		src控制的是nand读出的地址
		col 控制的是读出的第一个2K数据的长度
		*/
		for (; (col < 2048) && (i < len); i++, col++)	/*every time read 2K Byte data*/
		{
			*(dst + i) = NFDATA;
			src++;
		}
		col = 0;
		put_c('f');
	}
	disselect_chip();
	put_c('g');
}
void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = addr % 2048;
	int i = 0;
		
	/* 1. 选中 */
	select_chip();

	while (i < len)
	{
		/* 2. 发出读命令00h */
		nand_cmd(0x00);

		/* 3. 发出地址(分5步发出) */
		nand_addr(addr);

		/* 4. 发出读命令30h */
		nand_cmd(0x30);

		/* 5. 判断状态 */
		nand_wait_ready();

		/* 6. 读数据 */
		for (; (col < 2048) && (i < len); col++)
		{
			buf[i] = nand_data();
			i++;
			addr++;
		}
		
		col = 0;
	}

	/* 7. 取消选中 */		
	disselect_chip();
}

void copy_code_to_sdram(unsigned char* src, unsigned char* dst, unsigned int len)
{
	put_c('b');
	if(!(is_nor_or_nand()))	/*start from nor*/
	{
		put_c('c');
		unsigned int i = 0;
	 /*
		for (i = 0; i < len; i++)
		{
			dst[i] = src[i];
			//put_c('d');
		}
		//put_c('e');
	*/
		while(i < len)
		{
			dst[i] = src[i];
			i++;
		}
	}
	else			/*start form nand*/
	{
		nandflash_init();
		copy_code_from_nand(src, dst, len);
		//nand_read((unsigned int)src, dst, len);
	}
		
}

/*
void clear_bss(unsigned int* src, unsigned int* dst)
{
	for(;src < dst; src++)
	{
		*src = 0;
	}
}
*/
void clear_bss(void)
{
	extern int __bss_start, __bss_end;
	int* p= &__bss_start;
	put_c('h');
	for(; p < &__bss_end; p++)
	{
		*p = 0;
	}
}
void uart0_init(void)
{
	/*init uart0*/
	/*enable uart*/
	CLKCON |= (0x1 << 10);

	/*gphcon init*/
	GPHCON |= 0xa0;

	GPHUP = 0x0c;	
	/*
		Word Length = 8 bits
		1 stop bit per frame
		no parity
		normal mode operation
		0000011 = 0x03
	*/
	ULCON0 = 0x03;


	/*
	Receive/Transmit = Mode Interrupt request or polling mode
	Clock Selection = PCLK
	0000 0000 0101
	*/
	UCON0 =0x05;

	/*init uart*/
	/*
	UBRDIVn = (int)(50000000/(115200*16)) - 1
			  = (int)(50000000/1843200) - 1
			  = 27 - 1
			  = 26
	*/
	UFCON0 = 0x00;
	UMCON0 = 0x00;
	UBRDIV0 = UART_BRD;
}

void delay(unsigned long n)
{
	while(--n);
}
void put_c(unsigned   char str)
{
	while(!(UTRSTAT0 & 0x4));

	UTXH0 = str;
}
void put_s (char* str)
{
	int i =0;
	while(str[i])
	{
		put_c(str[i]);
		i++;
	}
}
void put_hex (void)
{

}
/*测试内存是否可写，如果不可写，说明初始化出了问题*/
void test_mem(void)
{
	volatile unsigned char* mem = 0x30000000;
	*mem = 'k';		//写到内存基地址
	put_c(*mem);	//读出来
}
