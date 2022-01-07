/****************************************************************/
/*		For Microchip 12C67x, 16C7x, 16F87x and Hi-Tech C		*/
/****************************************************************/
#ifndef __DELAY_C
#define __DELAY_C
#include "Delay.h"

unsigned char delayus_variable;
/****************************************************************/
/*		PAUSA MAYOR A 255 MICROSEGUNDOS							*/
/****************************************************************/
void DelayBigUs(unsigned int cnt)
{
	unsigned char	i;
	i = (unsigned char)(cnt>>8);
	while(i>=1)
	{
		i--;
		DelayUs(253);
		CLRWDT();
	}
	DelayUs((unsigned char)(cnt & 0xFF));
}
/****************************************************************/
/*		PAUSA DE HASTA 255 MILISEGUNDOS							*/
/****************************************************************/
void DelayMs(unsigned char cnt)
{
	unsigned char i;
	do
	{
		i=4;
		do
		{
			DelayUs(250);
			CLRWDT();
		}while(--i);
	}while(--cnt);
}
/****************************************************************/
/*		PAUSA DE HASTA 255 SEGUNDOS								*/
/****************************************************************/
void DelayS(unsigned char cnt)
{
	unsigned char i;
	do
	{
		i=4;
		do
		{
			DelayMs(250);
			CLRWDT();
		}while(--i);
	}while(--cnt);
}
#endif
