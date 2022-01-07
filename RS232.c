/****************************************************************************/
/*			COMUNICACION RS232 - RUTINAS DE ENVIO DE DATOS					*/
/****************************************************************************/
/*					PROTOTIPO DE FUNCIONES									*/
/*--------------------------------------------------------------------------*/
void Serial_Setup(unsigned int velocidad);
void PutStr(register const char *str);
void PutCh(unsigned char c);
/****************************************************************************/
/*					SETEO EL PUERTO SERIE									*/
/*--------------------------------------------------------------------------*/
void Serial_Setup(unsigned int velocidad)
{
	CLRWDT();
	if(velocidad>9500)
	{
		BRGH=1;				// Data rate for sending high speed.	
		SPBRG=(unsigned char)((_XTAL_FREQ/velocidad)/16)-1;
	}
	else
	{
		BRGH=0;				// Data rate for sending low speed.
		SPBRG=(unsigned char)((_XTAL_FREQ/velocidad)/64)-1;
	}
	SYNC=0;					// Asynchronous.
	SPEN=1;					// Enable serial port pins.
	CREN=0;					// Disable continuous reception.
	SREN=0;					// No effect.
	TXIE=0;					// Disable tx interrupts.
	TX9=0;					// 8-bit transmission.
	RX9=0;					// 8-bit reception.
	TXEN=0;					// Disable transmitter.
	TXEN=1;					// Enable the transmitter.
	CREN=1;					// Enable continuous reception.
	return;
}
/****************************************************************************/
/*		MANDA UNA CADENA DE CARACTERES POR EL PUERTO SERIE.					*/
/*--------------------------------------------------------------------------*/
void PutStr(register const char *str)
{
	while(*str)				// Mientras tenga algo para mandar.
		PutCh(*str++);		// Escribo el caracter en el puerto.
	return;
}
/****************************************************************************/
/*		MANDA SOLO UN CARACTER POR EL PUERTO SERIE							*/
/*--------------------------------------------------------------------------*/
void PutCh(unsigned char c)
{
	CLRWDT();
	while(!TXIF)			// En caso de estar ocupado en otra escritura,
		continue;			// espero a que termine, si tarda mucho salta WDT.
	TXREG=c;				// Escribo el caracter en el puerto.
	while(!TRMT);			// Espero que se vacie el buffer.
	return;
}
