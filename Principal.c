/********************************************************************************/
/*							LECTOR DE TARJETAS RFID								*/
/*..............................................................................*/
/*				Revisión:				1.00									*/
/*				PIC:					PIC16F268A								*/
/*				Compilador:				MPLAB IDE 8.70 - HI-TECH 9.60			*/
/*				Integrado:				EM4095									*/
/*				Comunicación:			RS232									*/
/*				Checksum:				0X13AD	(DECIMAL)						*/
/*										0Xfcbc	(HEXA)							*/
/*				Fecha de creación:		21/05/2014								*/
/*				Fecha de modificación:	02/10/2014								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*								MACROS											*/
/*..............................................................................*/
#define		_XTAL_FREQ	4000000	// 4Mhz - Oscilador interno.					*/
#define		ENTRADA		1		//												*/
#define		SALIDA		0		//												*/
#define		DECIMAL				// Defino el formato de la cadena de salida.	*/
/*------------------------------------------------------------------------------*/
/*				Defino los nombres de los pines de E/S							*/
/*..............................................................................*/
#define		MOD			RA0		// Habilito/deshabilito la modulación.			*/
#define		SHD			RA1		// Habilito/deshabilito el bajo consumo.		*/
#define		LED_PRES	RA2		// Led que indica presencia de tarjeta.			*/
#define		DEMOD_OUT	RA7		// Llega la cadena de bits de la lectura.		*/
#define		RDYCLK		RB0		// Llega CLK del EM4095.						*/
/********************************************************************************/
/*							VARIABLES GLOBALES									*/
/*..............................................................................*/
bit lectura;
bank1 unsigned char cad[70],pos_cad;
unsigned char synchro;
unsigned char comp[12];
/********************************************************************************/
/*							ARCHIVOS CABECERA									*/
/*..............................................................................*/
#include	"htc.h"				// Necesario para el compilador.				*/
#include	"stdio.h"			// Necesario para el sprintf.					*/
#include	"string.h"			// Manejo de cadenas.							*/
#include	"Delay.c"			// Rutinas de demoras.							*/
#include	"RS232.c"			// Manejo de la comunicación RS232.				*/
#include	"Interrupciones.c"	// Manejo de las interrupciones del PIC.		*/
#include	"EM4095.c"			// Driver del lector RFID EM4095.				*/
/********************************************************************************/
__CONFIG(PROTECT & LVPDIS & BORDIS & MCLREN & PWRTEN & WDTEN & INTCLK);
/********************************************************************************/
void main(void)
{
	char i;
	OSCF=1;				// Oscilador interno a 4 MHz.
	CMCON=0B00000111;	// Dehabilito los comparadores.
/********************************************************************************/
/*				Configuración de los puertos									*/
/*------------------------------------------------------------------------------*/
	LED_PRES=1;
	SHD=1;				// EM4095 en modo normal (SHD=1 modo sleep).
	MOD=1;				// Modulación (MOD=0 sin modulación, MOD=1 100% modulación)
	PORTB=0;			// Inicializo el puerto en cero.
/*------------------------------------------------------------------------------*/
	TRISA0=SALIDA;	   	// Salgo al pin MOD del EM4095.							*/
	TRISA1=SALIDA;   	// Va al pin SHD del EM4095.							*/
	TRISA2=SALIDA; 	 	// Led que indica la presencia de tarjeta en el lector.	*/
	TRISA3=SALIDA;   	// Sin uso.												*/
	TRISA4=SALIDA;   	// Sin uso.												*/
	TRISA6=SALIDA;   	// Sin uso.												*/
	TRISA7=ENTRADA;   	// Viene del pin DEMOD_OUT del EM4095.					*/
/*------------------------------------------------------------------------------*/
	TRISB0=ENTRADA;		// Interrupción generada desde RDY/CLK del EM4095.		*/
	TRISB1=ENTRADA;		// RS232 - RX.											*/
	TRISB2=SALIDA;		// RS232 - TX.											*/
	TRISB3=SALIDA;		// Sin uso.												*/
	TRISB4=SALIDA;		// Sin uso.												*/
	TRISB5=SALIDA;		// Sin uso.												*/
	TRISB6=ENTRADA;		// ICSP.												*/
	TRISB7=ENTRADA;		// ICSP.												*/
/********************************************************************************/
/*			TIMER 0 - NO UTILIZADO												*/
/*------------------------------------------------------------------------------*/
	T0CS=1;				// Oscilador externo.									*/
	T0SE=0;				// Flanco ascendente.									*/
	PSA=1;				// Asigno el preescaler a WDT.							*/
	PS0=1;				// Configuro el Preescaler.								*/
	PS1=1;				// 														*/
	PS2=1;				// 														*/
	T0IF=0;				// Bajo la bandera de la interrupción.					*/
/********************************************************************************/
/*			TIMER 1 - NO UTILIZADO												*/
/*------------------------------------------------------------------------------*/
	T1CKPS0=1; 			// Preescaler TMR1 a 1:8.								*/
	T1CKPS1=1; 			//														*/
	T1SYNC=1;			// No sincronizo con clock externo.						*/
	T1OSCEN=0;			// Oscilador deshabilitado.								*/
	TMR1CS=1;  			// Reloj interno Fosc/4.								*/
	TMR1IF=0;			// Bajo la bandera de la interrupción.					*/
	TMR1ON=0;			// Apago el TMR1.										*/
	TMR1H=0;			// Configuro el tiempo que tarda en generar				*/
	TMR1L=0;			// la interrupcion.										*/
/********************************************************************************/
/*			TIMER 2 - UTILIZADO PARA CAPTURAR LA CADENA PROVENIENTE DEL EM4095.	*/
/*------------------------------------------------------------------------------*/
	TMR2ON=1;			// Timer 2 prendido.									*/
	T2CKPS0=1;			// Configuro el Preescaler.								*/
	T2CKPS1=0;			// 														*/
	TOUTPS0=0;			// Configuro el postscaler.								*/
	TOUTPS1=0;			// 														*/
	TOUTPS2=0;			// 														*/
	TOUTPS3=0;			// 														*/
	TMR2IF=0;			// Bajo la bandera de la interrupción.					*/
/********************************************************************************/
/*			Configuración de las interrupciones									*/
/*------------------------------------------------------------------------------*/
	GIE=1;				// Utilizo interrupciones.								*/
	PEIE=1;				// Interrupcion externa habilitada.						*/
	INTE=0;				// Interrupcion RB0/INT deshabilitada.					*/
	T0IE=0;				// Interrupcion desborde TMR0 deshabilitada.			*/
	TMR1IE=0;			// Interrupcion desborde TMR1 habilitada.				*/
	TMR2IE=0;			// Interrupcion desborde TMR2 deshabilitada.			*/
	CCP1IE=0;			// CCP1 Interrupt disable.								*/
	CMIE=0;				// Comparator Interrupt disable.						*/
	EEIE=0;				// EEPROM Write Operation Interrupt disable.			*/
	RBIE=0;				// Interrupcion por RB deshabilitada.					*/
	RCIE=0;				// Interrupcion recepcion USART deshabilitada.			*/
 	INTEDG=1;			// Interrupcion en el flanco ascendente de RB0.			*/
	RBPU=1;				// RB pull-ups estan deshabilitadas.					*/
/********************************************************************************/
	Serial_Setup(9600);					// Configuro la velocidad del puerto serie.
	DelayMs(200);
	PutStr("\n\r");						// Al iniciar el programa mando un
	LED_PRES=0;
	while(1)
	{
		CLRWDT();
		i=Leo_Tarjeta();				// Realizo una lectura si hay una tarjeta.
		if(!i)							// Lectura correcta?
		{
			LED_PRES=1;					// Lectura correcta de una tarjeta.
			if(strcmp(comp,cad))		// Comparo para no imprimir dos veces la misma cadena.
			{
				strcpy(comp,cad);
				PutStr(comp);			// Mando el número leido.
				PutStr("\n\r");			// Final de linea y retorno de carro.
			}
		}
		else
			LED_PRES=0;					// Apago el led si no hubo una lectura excitosa.
		if(i==4)						// Alejaron la tarjeta del lector?
			comp[0]=0;					// Borro la cadena.
	}
}
