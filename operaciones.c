#include "maquina.h"


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

void ADD(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) + get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado); // ----------------------se tiene en cuenta el orden de las funciones actCC y luego resultado??
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void SUB(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) - get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void MUL(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) * get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado); 
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos); 
}
void DIV(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(Tipo2,registros[OP2],registros,memoria) != 0){
        resultado = get(Tipo1,registros[OP1],registros,memoria) / get(Tipo2,registros[OP2],registros,memoria);
        actualizarCC(registros,resultado);
        registros[AC] = get(Tipo1,registros[OP1],registros,memoria) % get(Tipo2,registros[OP2],registros,memoria);
        ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
    }
    else 
        printf("ERROR: division por cero\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}
void CMP(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) - get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
}
void SHL(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(Tipo1,registros[OP1],registros,memoria) << get(Tipo2,registros[OP2],registros,memoria)) & ~(0x1 << get(Tipo2,registros[OP2],registros,memoria));  // ???
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void SHR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(Tipo1,registros[OP1],registros,memoria) >> get(Tipo2,registros[OP2],registros,memoria)) & ~(0x1 >> get(Tipo2,registros[OP2],registros,memoria));  // ???
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void SAR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(Tipo1,registros[OP1],registros,memoria) & 0x80000000 == 1) // si es neg, el resultado tambien lo sera: agrego unos
        resultado = get(Tipo1,registros[OP1],registros,memoria) >> get(Tipo2,registros[OP2],registros,memoria);
    else 
        resultado = (get(Tipo1,registros[OP1],registros,memoria) >> get(Tipo2,registros[OP2],registros,memoria)) & ~(0x1 >> get(Tipo2,registros[OP2],registros,memoria)); // si es positivo: agrego ceros
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void AND(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) & get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void OR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) | get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void XOR(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(Tipo1,registros[OP1],registros,memoria) ^ get(Tipo2,registros[OP2],registros,memoria);
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void SWAP(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    ResultadoOperacion(Tipo1,registros,memoria,get(Tipo2,registros[OP2],registros,memoria),tablaSegmentos);
    ResultadoOperacion(Tipo2,registros,memoria,get(Tipo1,registros[OP1],registros,memoria),tablaSegmentos);
}
void LDL(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(Tipo2,registros[OP2],registros,memoria) & 0x0000FFFF) | (get(Tipo1,registros[OP1],registros,memoria) & 0xFFFF0000);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void LDH(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((get(Tipo2,registros[OP2],registros,memoria) << 16) & 0xFFFF0000) | (get(Tipo1,registros[OP1],registros,memoria) & 0x0000FFFF);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void NOT(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ~get(Tipo1,registros[OP1],registros,memoria);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void STOP(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = 0xFFFFFFFF;
}
/*
.
.
- AGREGAR FUNCION MOV, RND Y SYS
- SE DETECTARON LOS 3 TIPOS DE ERRORES Y SE INFORMO POR PANTALLA
.
.
*/
void NO_ACCESIBLE(uint8_t Tipo1,uint8_t Tipo2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    printf("INSTRUCCION INVALIDA: codigo de operacion de la instruccion a ejecutar no existe\n");  // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}