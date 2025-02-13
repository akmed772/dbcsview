/*
This content is under the MIT license.
Copyright (C) 2025 akm.
*/
#define _TARGET_DOS 1

#include <stdio.h>
#include <stdlib.h>

const char METACREDIT[] = "DBCS Bald Character Set viewer for DOS (platform independent); (c) 2025 akm.";
typedef signed char        int8_t;
typedef short              int16_t;
typedef long              int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;

#ifdef _TARGET_DOS
	typedef struct _DBCS_TABLE
	{
	   uint8_t start;
	   uint8_t end;
	} DBCS_TABLE;
	DBCS_TABLE dbcs_tbl[128];
#endif

/*
	This utility uses ANSI escape sequences for the text user interface.
	In MS-DOS and PC DOS, you must use the ANSI.SYS device driver.
*/

void screen_jump(uint16_t index)
{
	char chr;
	int i, y;
	int isdbcs;
#ifdef _TARGET_DOS
	printf("\x1b[2J");
#endif
	printf("\r\n");
	index &= 0xffe0u;
	for (y=0;y<18;y++)
	{
		if ((index & 0xffu) < 0x40u)
		{
			index &= 0xff00u;
			index |= 0x0040u;
		}
		if (index >= 0x2040) {
#ifdef _TARGET_DOS
			isdbcs = 0;
			for (i=0;i<128;i++) {
				if (dbcs_tbl[i].end == 0) break;
				if (((index >> 8) >= dbcs_tbl[i].start) && ((index >> 8) <= dbcs_tbl[i].end))
				{
					isdbcs = 1;
					break;
				}
			}
			if (isdbcs) printf("  %04X: ", index);
			else  printf(" *%04X: ", index);
#else
			printf("  %04X: ", index);
#endif
			for (i=0;i<32;i++)
			{
				if (i==16) putchar(' ');
				chr = (index >> 8) & 0xffu;
				putchar(chr);
				chr = index & 0xffu;
				putchar(chr);
				index++;
		//		putchar(' ');
			}
		}
		else
		{
			index += 32;
		}
		printf("\r\n");
	}
#ifdef _TARGET_DOS
	printf("\x1b[3B");
	printf(" \x1b[1mW\x1b[0m Scroll Up    \x1b[1mA\x1b[0m Page Up    \x1b[1mS\x1b[0m Scroll Down    \x1b[1mD\x1b[0m Page Down    \x1b[1mJ\x1b[0m Jump    \x1b[1mESC\x1b[0m Exit\r\n");
	printf("\x1b[2A");
#else
	printf("[W] Scroll Up  [A] Page Up  [S] Scroll Down  [D] Page Down  [J] Jump  [ESC] Exit\r\n");
#endif
	return;
}

uint16_t scroll_up(uint16_t index)
{
	if ((index & 0xffu) < 0x40u)
	{
		index -= 0x40u;
	}
	if (index < 0x2000u) 
	{
		index -= 0x2000u;
	}
	screen_jump(index);
	return index; /* return the next index to scroll */
}

uint16_t scroll_down(uint16_t index)
{
	if ((index & 0xffu) < 0x40u)
	{
		index += 0x40u;
	}
	if (index < 0x2000u) 
	{
		index += 0x2000u;
	}
	screen_jump(index);
	return index;
}


int main(void)
{
	char inkey;
	char instr[256];
	uint16_t index = 0x8140u;
	int keywait = 1;
	int i;
#ifdef _TARGET_DOS
	dbcs_tbl[0].start = 0;
	dbcs_tbl[0].end = 0;
_asm
{
	push	es
	push	ds
	lea	ax, dbcs_tbl
	mov	di, ax
	mov	ax, ds
	mov	es, ax
	mov	ax, 0x6300
	clc
	int	0x21
	jc	getdbcs_end
	cld
getdbcs_loop:
	mov	al, [si+1]
	movsb
	movsb
	cmp	al, 0
	je	getdbcs_end
	jmp	getdbcs_loop
getdbcs_end:
	pop	ds
	pop	es
}
/*
	for (i=0;i<128;i++) {
		if (dbcs_tbl[i].end == 0) break;
		printf("%x ,%x\r\n", dbcs_tbl[i].start, dbcs_tbl[i].end);
	}
	return 0;
	*/
#endif
	scroll_up(index);
	while (keywait){
		inkey = getch();
		switch(inkey)
		{
			case ' ':
				keywait = 0;
				break;
			case 0x6A: /* J */
				printf("Jump to (in hex):");
				fseek(stdin,0,SEEK_END);
				scanf("%4x", &index);
				index = scroll_up(index);
				break;
			case 0x77: /* W */
				index = scroll_up(index - 0x20u);
				break;
			case 0x73: /* S */
				index = scroll_down(index + 0x20u);
				break;
			case 0x61: /* A */
				index = scroll_up(index - 0x300u);
				break;
			case 0x64: /* D */
				index = scroll_down(index + 0x300u);
				break;
			case 0x03: /* Ctrl + C */
			case 0x1b: /* Escape */
				keywait = 0;
				break;
			default:
				break;
		}
//		printf("%02X",inkey);
	}
	printf("\r\n");
	return 0;
}
