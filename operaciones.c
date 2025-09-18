#include "maquina.h"

void SYS(uint32_t registros[],uint8_t memoria[]){
    //falta leer eax para determinar el formato de lectura
    uint16_t cantBytes = (registros[ECX] >> 16) & 0x000000FF;
    uint16_t cantCeldas = (registros[ECX]) & 0x000000FF;
    uint8_t modo_lectura = registros[EAX];
    int valor;
    if(registros[OP1] == 0x1){ //lectura
        //lectura, permite almacenar los datos leidos desde el teclado a partir de la posicion de memoria apuntada por EDX.
        operacion_memoria(registros,memoria,registros[EDX], 0, LECTURA, cantBytes);
    }
    else{ //escritura
        scanf("%d", &valor);
        operacion_memoria(registros,memoria,registros[EDX], valor, ESCRITURA, cantBytes);
    }
}
void JMP(uint32_t registros[]){
    registros[IP] = registros[OP1];
}

void JZ(uint32_t registros[]){
    if((registros[CC] & 0x01) == 1)
        registros[IP] = registros[OP1];
}
void JP(uint32_t registros[]){
    if(registros[CC] == 0)
        registros[IP] = registros[OP1];
}
void JN(uint32_t registros[]){
    if((registros[CC] & 0x02) == 2)
        registros[IP] = registros[OP1];
}
void JNZ(uint32_t registros[]){
    if((registros[CC] & 0x01) == 0)
        registros[IP] = registros[OP1];
}
void JNP(uint32_t registros[]){
    if((registros[CC] & 0x03) == 2 || (registros[CC] & 0x03) == 1)
        registros[IP] = registros[OP1];
}
void JNN(uint32_t registros[]){
    if((registros[CC] & 0x02) < 2)
        registros[IP] = registros[OP1];
}

void ADD(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) + get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado); // ----------------------se tiene en cuenta el orden de las funciones actCC y luego resultado??
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void SUB(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) - get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void MUL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) * get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado); 
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos); 
}
void DIV(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(registros[OP2],registros,memoria) != 0){
        resultado = get(registros[OP1],registros,memoria) / get(registros[OP2],registros,memoria);
        actualizarCC(registros,resultado);
        registros[AC] = get(registros[OP1],registros,memoria) % get(registros[OP2],registros,memoria);
        ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
    }
    else{ 
        printf("ERROR: division por cero\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        STOP(registros,memoria,tablaSegmentos);
    }
}
void CMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) - get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
}
void SHL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP1],registros,memoria) << get(registros[OP2],registros,memoria)) & ~(0x1 << get(registros[OP2],registros,memoria));  // ???
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void SHR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP1],registros,memoria) >> get(registros[OP2],registros,memoria)) & ~(0x1 >> get(registros[OP2],registros,memoria));  // ???
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void SAR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(registros[OP1],registros,memoria) & 0x80000000 == 1) // si es neg, el resultado tambien lo sera: agrego unos
        resultado = get(registros[OP1],registros,memoria) >> get(registros[OP2],registros,memoria);
    else 
        resultado = (get(registros[OP1],registros,memoria) >> get(registros[OP2],registros,memoria)) & ~(0x1 >> get(registros[OP2],registros,memoria)); // si es positivo: agrego ceros
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void AND(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) & get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void OR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) | get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void XOR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria) ^ get(registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void SWAP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    ResultadoOperacion(registros,memoria,get(registros[OP2],registros,memoria),tablaSegmentos);
    ResultadoOperacion(registros,memoria,get(registros[OP1],registros,memoria),tablaSegmentos);
}
void LDL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP2],registros,memoria) & 0x0000FFFF) | (get(registros[OP1],registros,memoria) & 0xFFFF0000);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void LDH(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((get(registros[OP2],registros,memoria) << 16) & 0xFFFF0000) | (get(registros[OP1],registros,memoria) & 0x0000FFFF);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void NOT(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ~get(registros[OP1],registros,memoria);
    ResultadoOperacion(registros,memoria,resultado,tablaSegmentos);
}
void STOP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = 0xFFFFFFFF;
}
void MOV(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    set(registros,memoria,registros[OP1],get(registros[OP2], registros, memoria));
}
/*
.
.
- AGREGAR FUNCION MOV, RND Y SYS
- SE DETECTARON LOS 3 TIPOS DE ERRORES Y SE INFORMO POR PANTALLA
.
.
*/
void NO_ACCESIBLE(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    printf("INSTRUCCION INVALIDA: codigo de operacion de la instruccion a ejecutar no existe\n");  // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}