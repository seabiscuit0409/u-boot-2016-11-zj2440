#define NFCONF (*(volatile unsigned short*)0x4E000000)
#define NFCONT (*(volatile unsigned short*)0x4E000004)
#define NFCMMD (*(volatile unsigned short*)0x4E000008)
#define NFADDR (*(volatile unsigned char*)0x4E00000C)
#define NFDATA (*(volatile unsigned char*)0x4E000010)
#define NFSTAT (*(volatile unsigned char*)0x4E000020)

#define CLKCON	  (*(volatile unsigned int*)0x4C00000C)
#define GPHCON	  (*(volatile unsigned int*)0x56000070)
#define GPHUP	  (*(volatile unsigned char*)0x56000078)
#define ULCON0	  (*(volatile unsigned char*)0x50000000)
#define UCON0	  (*(volatile unsigned short*)0x50000004)
#define UBRDIV0   (*(volatile unsigned short*)0x50000028)
#define UTRSTAT0  (*(volatile unsigned char*)0x50000010)
#define UTXH0	  (*(volatile unsigned char*)0x50000020)
#define UFCON0	  (*(volatile unsigned char*)0x50000008)
#define UMCON0	  (*(volatile unsigned char*)0x5000000C)
#define CLOCK	  50000000
#define BUARD_RUAT	115200 
#define UART_BRD  ((int)(CLOCK / (BUARD_RUAT * 16)) -1)

void nandflash_init(void);

void copy_code_to_sdram(unsigned char* src, unsigned char* dst, unsigned int len);

void clear_bss(void);

void put_c(unsigned   char str);
void put_s (char* str);
void put_hex (void);
void test_mem(void);


