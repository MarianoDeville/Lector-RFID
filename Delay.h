#ifndef __DELAY_H
#define __DELAY_H

extern unsigned char delayus_variable;

//delay routine

#if _XTAL_FREQ == 4000000
	#define DelayDivisor 4
	#define WaitFor1Us asm("nop")
	#define Jumpback asm("goto $ - 2")
#elif _XTAL_FREQ == 8000000
	#define DelayDivisor 2
	#define WaitFor1Us asm("nop")
	#define Jumpback asm("goto $ - 2")
#elif ( (_XTAL_FREQ == 16000000) || (_XTAL_FREQ==16257000) )
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop")
	#define Jumpback asm("goto $ - 2")
#elif _XTAL_FREQ == 20000000
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop"); asm("nop")
	#define Jumpback asm("goto $ - 3")
#elif _XTAL_FREQ == 32000000
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")
	#define Jumpback asm("goto $ - 6")
#else
	#error delay.h - Por favor defina una de estas velocidades de clock:
	#error 4000000 8000000 16000000 16257000 20000000 32000000
#endif

#define DelayUs(x) { \
			delayus_variable=(unsigned char)(x/DelayDivisor); \
			WaitFor1Us; } \
			asm("decfsz _delayus_variable,f"); \
			Jumpback;

/********************************************************************************/
/*				PROTOTIPO DE FUNCIONES											*/
/*-----------------------------------------------------------------------------*/
void DelayBigUs(unsigned int cnt);
void DelayMs(unsigned char cnt);
void DelayS(unsigned char cnt);

#endif
