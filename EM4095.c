/********************************************************************************/
/*							DRIVER LECTOR EM4095								*/
/*..............................................................................*/
/*				Revisión:				1.00									*/
/*				Interrupciones:			TMR2.									*/
/*				Compilador:				MPLAB IDE 8.70 - HI-TECH 9.60			*/
/*				Fecha de creación:		22/05/2014								*/
/*				Fecha de modificación:	22/08/2014								*/
/*				Autor:					Mariano Ariel Deville					*/
/********************************************************************************/
/*		Tarjetas tipo EM (solo lectura), formato de los datos:					*/
/*		1111 11111  = Header			= 9 bit									*/
/*		xxxxP,xxxxp = Custom #1,#2 		= 10 bit								*/
/*		xxxxP,xxxxP = data #1,#2		= 10 bit								*/
/*		xxxxP,xxxxP = data #3,#4		= 10 bit								*/
/*		xxxxP,xxxxP = data #5,#6		= 10 bit								*/
/*		xxxxP,xxxxP = data #7,#8		= 10 bit								*/
/*		PPPP0 		= 4 Parity + 1 stop	= 5 bit									*/
/*		55 bits de datos + header 9		= 64 bits totales.						*/
/*																				*/
/*		Header (9 bits)															*/
/*		------__--__--__--__--__--__--__--__--__ (wave)							*/
/*		1-1 1-0 1-0 1-0 1-0 1-0 1-0 1-0 1-0 1-0  (edge)							*/
/*		(x) (1) (1) (1) (1) (1) (1) (1) (1) (1)  (code)							*/
/*..............................................................................*/
/*	 	Valores devueltos por la función según lo detectado:					*/
/*			0 - Lectura realizada y chequeo ok.									*/
/*			1 - No hay tarjeta (ancho del pulso no standar.						*/
/*			2 - Paridad del nibble incorrecta.									*/
/*			3 - Paridad de la columna incorrecta.								*/
/*			4 - No hay tarjeta (timeout en la lectura).							*/
/*		 Utilizo el timer 2 para contar tiempo, 4 us por ciclo.					*/
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*..............................................................................*/
char Leo_Tarjeta(void);
char Formato(void);
void Convierto_Decimal(void);
/********************************************************************************/
/*								MACROS											*/
/*..............................................................................*/
#define	TIME_OUT	if(TMR2>250)return 4
#define ANCHO_MIN	54
#define ANCHO_MAX	75
/********************************************************************************/
/*						LECTURA DE UNA TARJETA.									*/
/*..............................................................................*/
char Leo_Tarjeta(void)
{
	unsigned char timer,i;
	switch(synchro)
	{
		case 131:
			synchro=132;
			break;
		case 132:
			synchro=133;
			break;
		case 133:
			synchro=131;
			break;
		default:
			synchro=131;
	}
	SHD=0;							// Dispositivo activo.
	MOD=0;							// Comienzo la lectura.
	i=0;
	TMR2=0;							// Reseteo el contador del timer para usarlo en el timeout.
	while(DEMOD_OUT)				// Espero el cambio de estado y mido el ancho del pulso.
	{
		TIME_OUT;					// Hubo un timeout? salgo.
		timer=TMR2;					// Cargo el valor del timer para controlarlo.
	}
	if(timer<120 || timer>131)		// Ancho del pulso incorrecto?
		return 1;					// Salgo y aviso que no hay tarjeta.
	TMR2=0;							// Reseteo el contador del timer para usarlo en el timeout.
	while(!DEMOD_OUT)				// Espero el cambio de estado y mido el ancho del pulso.
	{
		TIME_OUT;					// Hubo un timeout? salgo.
		timer=TMR2;					// Cargo el valor del timer para controlarlo.
	}
	if(timer<ANCHO_MIN||timer>ANCHO_MAX)	// Ancho del pulso incorrecto?
		return 1;					// Salgo y aviso que no hay tarjeta.
	while(i++<7)					// Controlo si hay una cabecera correcta.
	{
		TMR2=0;						// Reseteo el contador del timer para usarlo en el timeout.
		while(DEMOD_OUT)			// Espero el cambio de estado y mido el ancho del pulso.
		{
			TIME_OUT;				// Hubo un timeout? salgo.
			timer=TMR2;				// Cargo el valor del timer para controlarlo.
		}
		if(timer<ANCHO_MIN||timer>ANCHO_MAX)	// Ancho del pulso incorrecto?
			return 1;				// Salgo y aviso que no hay tarjeta.
		TMR2=0;						// Reseteo el contador del timer para usarlo en el timeout.
		while(!DEMOD_OUT)			// Espero el cambio de estado y mido el ancho del pulso.
		{
			TIME_OUT;				// Hubo un timeout? salgo.
			timer=TMR2;				// Cargo el valor del timer para controlarlo.
		}
	}
	TMR2=0;							// Reseteo el contador del timer para usarlo en el timeout.
	while(DEMOD_OUT)				// Espero el cambio de estado y mido el ancho del pulso.
	{
		TIME_OUT;					// Hubo un timeout? salgo.
		timer=TMR2;					// Cargo el valor del timer para controlarlo.
	}
	if(timer<ANCHO_MIN||timer>ANCHO_MAX)	// Ancho del pulso incorrecto?
		return 1;					// Salgo y aviso que no hay tarjeta.
	pos_cad=9;
	lectura=0;
	TMR2=98;						// Ciclos del timer hasta la lectura del primer bit.
	TMR2IF=0;
	TMR2IE=1;						// Habilito la interrupción timer 2.
	while(!lectura);				// Espero a que se cargue una lectura completa.
	TMR2IE=0;
	return Formato();
}
/********************************************************************************/
/*		CONTROLO EL CRC Y CONVIERTO LA CADENA PARA ENVIARLA POR RS232			*/
/*..............................................................................*/
char Formato(void)
{
	volatile char i,e,a;
	for(a=0;a<10;a++)				// Armo la cadena con los diez digitos leidos.
	{
		e=0;
		cad[a]=0;					// Vacio el valor de la cadena en este punto.
		for(i=9;i<14;i++)			// Barro el nibble.
		{
			if(i<13)				// No estoy en el bit de paridad?
				cad[a]=(cad[a]<<1)+cad[i+(a*5)];
			e=e+cad[i+(a*5)];		// Calculo la suma de la paridad.
		}
		if((e&1))	return 2;		// Paridad del nibble incorrecta?
	}
	for(i=59;i<63;i++)				// Cargo el checksum de la cadena en la posición 10 de la cadena.
	{
		cad[10]=((cad[10]<<1)+cad[i])&0x0f;	// Solo 4 bits.
	}
	for(a=0;a<4;a++)				// Barro las cuatro columnas para calcular la paridad vertical.
	{
		e=0;
		for(i=0;i<11;i++)			// Barro los 11 bits de cada columna (10 de datos + 1 de paridad).
		{
			e=e+((cad[i]>>a)&1);	// Sumo el checksum parcial de cada columna..
		}
		if((e&1))	return 3;		// Paridad de la columna incorrecta?
	}
#ifdef DECIMAL
	Convierto_Decimal();
#else
	for(a=0;a<10;a++)				// Adecuacion de los 10 digitos para su transmisión.
	{
		cad[a]=cad[a]+48;			// Convierto a ASCII y hexa.
		if(cad[a]>57)				// Debo agregar letras?
			cad[a]+=7;
	}
#endif
	cad[10]=0;						// Final de cadena.
	return 0;						// Chequeo de las paridades y armado de la cadena ok.
}
/********************************************************************************/
/*					CONVIERTO LA CADENA DE HEXA A DECIMAL						*/
/*..............................................................................*/
#ifdef DECIMAL
void Convierto_Decimal(void)
{
	volatile char i;
	unsigned long inter;
	inter=0;
	for(i=0;i<8;i++)				// Paso la cadena leida a una variable de 32 bits.
	{
		inter=inter<<4;				// Paso al nibble siguiente.
		inter=inter+(cad[i+2]&0x0f);
	}
	sprintf(cad,"%10lu",inter);		// Paso el valor de la variable a un string.
	for(i=0;i<10;i++)				// Busco caracteres no válidos.
	{
		if(cad[i]<48||cad[i]>57)
			cad[i]=48;				// Los completo con ceros.
	}
	return;
}
#endif

