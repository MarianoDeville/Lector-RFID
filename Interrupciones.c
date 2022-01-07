/********************************************************************************/
/*								INTERRUPCIONES									*/
/********************************************************************************/
void interrupt isr(void)
{
	unsigned char resp;
	if(TMR2IE && TMR2IF)
	{
		TMR2=synchro;				// Ciclos del timer para sincronizar con la onda de respuesta.
		cad[pos_cad++]=!DEMOD_OUT;	// Cargo en bit dentro de la cadena.
		if(pos_cad>66)				// Llené la cadena completa?
			lectura=1;				// Bandera para bajar las interrupciones.
		TMR2IF=0;					// Bajo la bandera de la interrupción.
		return;						// Salgo de las interrupciones.
	}
	if(RCIF && RCIE)				// Interrupcion por RS232?
	{
		resp=RCREG;					// Vacío el buffer del nódulo RS232.
		return;
	}
	return;							// Salgo de las interrupciones.
}




