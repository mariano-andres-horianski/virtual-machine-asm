#include "maquina.h"

void SYS(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    //falta leer eax para determinar el formato de lectura
    uint16_t cantBytes = (registros[ECX] >> 16) & 0x000000FF;
    uint16_t cantCeldas = (registros[ECX]) & 0x000000FF;
    uint8_t modo_lectura = registros[EAX],byteActual;
    uint64_t valor;
    int i,b,j,caracter;
    if(registros[OP1] == 0x1){ //lectura
        //guarda en memoria
        for(i = 0; i<cantCeldas; i++){
            scanf("%d", &valor);
            for(j = 0; j < cantBytes; j++){
                byteActual =  valor >> ((cantBytes - 1 - j) * 8) & 0xFF;
                operacion_memoria(registros,memoria,registros[EDX]+i*cantBytes+j, byteActual, ESCRITURA, 1, tablaSegmentos); // pongo un 4 porque es la maxima cantidad de bytes que entra en el MBR
            }
            printf("%04x", registros[EDX]+i*cantBytes);
            switch (modo_lectura){
                case 0x10:
                    //muestro en hexa la direccion
                    for(b = 0; b < cantBytes; b++){
                        printf("%d ",valor >> ((cantBytes - 1 - b) * 8) & 0x01);//muestro el valor en binario
                    }
                    printf("\n");
                break;
                
                case 0x08:
                    printf("%x\n",valor);
                case 0x04:
                    printf("%llo\n",valor);
                case 0x02:
                    printf("%lc\n",valor);
                    for(caracter = 0; caracter < cantBytes; caracter++){
                        printf("%c",valor >> ((cantBytes - 1 - caracter) * 8) & 0xFF);
                        printf("\n");
                    }
                case 0x01:
                    printf("%d\n",valor);
            }
        }
        
    }
    else{ //escribe en pantalla
        valor = 0;
        
        for(i = 0; i<cantCeldas; i++){
            for(j = 0; j < cantBytes; j++){
                operacion_memoria(registros,memoria,registros[EDX]+i*cantBytes+j, 0, LECTURA, 1, tablaSegmentos);
                byteActual = memoria[MBR];
                valor = valor | (byteActual << ((cantBytes - 1 - j) * 8));
            }
            printf("%d\n", valor);
            switch (modo_lectura){
                case 0x10:
                    //muestro en hexa la direccion
                    for(b = 0; b < cantBytes; b++){
                        printf("%d ",valor >> ((cantBytes - 1 - b) * 8) & 0x01);//muestro el valor en binario
                    }
                    printf("\n");
                break;
                
                case 0x08:
                    printf("%x\n",valor);
                case 0x04:
                    printf("%llo\n",valor);
                case 0x02:
                    printf("%lc\n",valor);
                    for(caracter = 0; caracter < cantBytes; caracter++){
                        printf("%c",valor >> ((cantBytes - 1 - caracter) * 8) & 0xFF);
                        printf("\n");
                    }
                case 0x01:
                    printf("%d\n",valor);
            }
            valor = 0;
        }

    }
}
void RND(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    srand(time(NULL));

    set(registros,memoria,registros[OP1], rand() % (get(registros[OP2], registros, memoria,tablaSegmentos) + 1),tablaSegmentos);
}
void JMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = registros[OP1];
}

void JZ(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & 0x01) == 1)
        registros[IP] = registros[OP1];
}
void JP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if(registros[CC] == 0)
        registros[IP] = registros[OP1];
}
void JN(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & 0x02) == 2)
        registros[IP] = registros[OP1];
}
void JNZ(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & 0x01) == 0)
        registros[IP] = registros[OP1];
}
void JNP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & 0x03) == 2 || (registros[CC] & 0x03) == 1)
        registros[IP] = registros[OP1];
}
void JNN(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & 0x02) < 2)
        registros[IP] = registros[OP1];
}

void ADD(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) + get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado); // ----------------------se tiene en cuenta el orden de las funciones actCC y luego resultado??
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SUB(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) - get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void MUL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) * get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado); 
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos); 
}
void DIV(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(registros[OP2],registros,memoria,tablaSegmentos) != 0){
        resultado = get(registros[OP1],registros,memoria,tablaSegmentos) / get(registros[OP2],registros,memoria,tablaSegmentos);
        actualizarCC(registros,resultado);
        registros[AC] = get(registros[OP1],registros,memoria,tablaSegmentos) % get(registros[OP2],registros,memoria,tablaSegmentos);
        set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
    }
    else{ 
        printf("ERROR: division por cero\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        STOP(registros,memoria,tablaSegmentos);
    }
}
void CMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) - get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
}
//Los shifts en C serán lógicos porque siempre trabajamos con unsigned
void SHL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP1],registros,memoria,tablaSegmentos) << get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SHR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP1],registros,memoria,tablaSegmentos) >> get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SAR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(get(registros[OP1],registros,memoria,tablaSegmentos) & 0x80000000 == 1) // si es neg, el resultado tambien lo sera: agrego unos
        resultado = get(registros[OP1],registros,memoria,tablaSegmentos) >> get(registros[OP2],registros,memoria,tablaSegmentos) | ~(0xFFFFFFFF >> get(registros[OP2],registros,memoria,tablaSegmentos));
    else 
        resultado = (get(registros[OP1],registros,memoria,tablaSegmentos) >> get(registros[OP2],registros,memoria,tablaSegmentos)); // si es positivo: agrego ceros
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void AND(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) & get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void OR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) | get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void XOR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) ^ get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SWAP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int aux = get(registros[OP1],registros,memoria,tablaSegmentos);
    set(registros,memoria, registros[OP1],get(registros[OP2],registros,memoria,tablaSegmentos),tablaSegmentos);
    set(registros,memoria, registros[OP2],aux,tablaSegmentos);
}
void LDL(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (get(registros[OP2],registros,memoria,tablaSegmentos) & 0x0000FFFF) | (get(registros[OP1],registros,memoria,tablaSegmentos) & 0xFFFF0000);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void LDH(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((get(registros[OP2],registros,memoria,tablaSegmentos) << 16) & 0xFFFF0000) | (get(registros[OP1],registros,memoria,tablaSegmentos) & 0x0000FFFF);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void NOT(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ~get(registros[OP1],registros,memoria,tablaSegmentos);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void STOP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = 0xFFFFFFFF;
}
void MOV(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    set(registros,memoria,registros[OP1],get(registros[OP2], registros, memoria,tablaSegmentos),tablaSegmentos);
}
/*
.
.
- SE DETECTARON LOS 3 TIPOS DE ERRORES Y SE INFORMO POR PANTALLA
.
.
*/
void NO_ACCESIBLE(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    printf("INSTRUCCION INVALIDA: codigo de operacion de la instruccion a ejecutar no existe\n");  // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}