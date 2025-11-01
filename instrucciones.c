#include "maquina.h"

void SYS(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    /*
    En EDX se almacena la dirección lógica de los datos a leer/escribir, en la parte alta el segmento y en la baja el offset.
    En ECX se almacena en la parte alta la cantidad de bytes por celda y en la baja la cantidad de celdas a leer/escribir
    En EAX se almacena el formato de lectura/escritura
    */
    uint16_t cantBytes = (registros[ECX] >> 16) & 0x0000FFFF, segmento =26 + ((registros[EDX] >> 16) & 0x0000FFFF);
    uint16_t cantCeldas = (registros[ECX]) & 0x0000FFFF, direccion;
    uint8_t modo_lectura = registros[EAX],byteActual;
    int64_t valor; //esto es int64 porque tiene que tener signo y en caso de que a alguien se le ocurra celdas de tamaño mayor a 4 bytes
    int i,b,j,caracter;
    

    uint32_t llamado = get(registros[OP1],registros,memoria,tablaSegmentos);
    switch (llamado){
        case  0x1: { //lectura
            //guarda en memoria
            for(i = 0; i<cantCeldas; i++){
                if(scanf("%lld", &valor) != 1) {
                    printf("ERROR: Entrada inválida\n");
                    STOP(registros, memoria, tablaSegmentos);
                    return;
                }
                for(j = 0; j < cantBytes; j++){
                    byteActual =  (valor >> ((cantBytes - 1 - j) * 8)) & 0xFF;
                    //llamamos a operacion memoria y no a set para no escribir exclusivamente 4 bytes de golpe
                    operacion_memoria(registros,memoria,registros[EDX]+i*cantBytes+j, byteActual, ESCRITURA, 1, tablaSegmentos, registros[segmento]);
                }
                printf("[%04X]: ",(registros[MAR] & 0x0000FFFF) - cantBytes + 1);
                if((modo_lectura & 0x10) != 0){
                    // binario
                    for(b = 0; b < cantBytes; b++){
                        printf("%d ",valor >> ((cantBytes - 1 - b) * 8) & 0x01);
                    }
                    printf(" ");
                }
                if((modo_lectura & 0x08) != 0){
                    //hexa
                    printf("%x",valor);
                    printf(" ");
                }
                if((modo_lectura & 0x04) != 0){
                    //octal
                    printf("%llo",valor);
                    printf(" ");
                }
                if((modo_lectura & 0x02) != 0){
                    //caracter
                    printf("%lc",valor);
                    for(caracter = 0; caracter < cantBytes; caracter++){
                        printf("%c",valor >> ((cantBytes - 1 - caracter) * 8) & 0xFF);
                    }
                    printf(" ");
                }
                if((modo_lectura & 0x01) != 0){
                    //decimal
                    printf("%lld",valor);
                }
            printf("\n");
            }
            printf("\n");
        }
        break;
        case 0x2: { //escribe en pantalla 
            for(i = 0; i<cantCeldas; i++){
                valor = 0;
                for(j = 0; j < cantBytes; j++){
                    //escribimos byte a byte, pasamos 0 en el valor para asegurarnos que el MBR va a estar limpio
                    operacion_memoria(registros,memoria,registros[EDX]+i*cantBytes+j, 0, LECTURA, 1, tablaSegmentos, registros[segmento]);
                    byteActual = registros[MBR];
                    valor = valor | (byteActual << ((cantBytes - 1 - j) * 8));
                }
                printf("[%04X]: ",(registros[MAR] & 0x0000FFFF) - cantBytes + 1);
                if((modo_lectura & 0x10) != 0){
                    for(b = 0; b < cantBytes; b++){
                        printf("%d ",valor >> ((cantBytes - 1 - b) * 8) & 0x01);//muestro el valor en binario
                    }
                    printf(" ");
                }
                if((modo_lectura & 0x08) != 0){
                    printf("0x%x",valor);
                    printf(" ");
                }
                if((modo_lectura & 0x04) != 0){
                    printf("%o",valor);
                    printf(" ");
                }
                if((modo_lectura & 0x02) != 0){
                    //printf("%c",valor);
                    for(caracter = 0; caracter < cantBytes; caracter++){
                        printf("%c",valor >> ((cantBytes - 1 - caracter) * 8) & 0xFF);
                    }
                    printf(" ");
                }
                if((modo_lectura & 0x01) != 0){
                    printf("%d",valor);
                }
            printf("\n");
            }
        printf("\n");
        }
        break;
        case 0x3: {//STRING READ osea guarda un string en memoria
            uint16_t cantCaracteres = registros[ECX] & 0xFFFF;
            char caracter;
            if(cantCaracteres != -1)
                for(i=0; i<cantCaracteres; i++){ 
                    caracter = getchar();
                    operacion_memoria(registros,memoria,registros[EDX]+i, caracter, ESCRITURA, 1, tablaSegmentos, registros[segmento]);
                }
            else {
                caracter = getchar();
                i=0;
                while(caracter != '\n'){
                    operacion_memoria(registros,memoria,registros[EDX]+i, caracter, ESCRITURA, 1, tablaSegmentos, registros[segmento]);
                    caracter = getchar();
                    i++;
                }
            }
            operacion_memoria(registros,memoria,registros[EDX]+i,'\0', ESCRITURA, 1, tablaSegmentos, registros[segmento]);
        }
        case 0x4: { //SRTING WRITE osea muestra un string
            i = 0;
            operacion_memoria(registros,memoria,registros[EDX], 0, LECTURA, 1, tablaSegmentos, registros[segmento]);
            char caracter = registros[MBR] & 0xFF;
            while(caracter != '\0'){
                printf("%c",caracter);
                i++;
                operacion_memoria(registros,memoria,registros[EDX]+i, 0, LECTURA, 1, tablaSegmentos, registros[segmento]);
                caracter = registros[MBR] & 0xFF;
            } 
        break;
        }
        case 0x7: {
            system("cls");
            break; 
        }
        case 0xF:{//BREAKPOINT
            if(imagenVMI == 1){ //--------------CONDICION DE VMI----------------------------------------------------------------
               do{
                generar_imagen(registros,memoria,tablaSegmentos);
                caracter = getchar(); 
                if (caracter != '\n') {
                    int temp_c;
                    while ((temp_c = getchar()) != '\n' && temp_c != EOF);
                }
                if(caracter == 'q')
                    STOP(registros,memoria,tablaSegmentos);
                else
                    if(caracter == '\n' && registros[IP] != 0xFFFFFFFF && registros[CS] + tablaSegmentos[registros[CS]].tamanio > registros[IP])
                        leerInstrucciones(memoria, registros, tablaSegmentos);
                } while(caracter != 'q' && caracter != 'g' && registros[IP] != 0xFFFFFFFF && registros[CS] + tablaSegmentos[registros[CS]].tamanio > registros[IP]);
                break; 
            }
        }
    }
}
void RND(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    srand(time(NULL));

    set(registros,memoria,registros[OP1], rand() % (get(registros[OP2], registros, memoria,tablaSegmentos) + 1),tablaSegmentos);
}
void JMP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;;
}

void JZ(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x01<<30)) == (1<<30))
        registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;
}
void JP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if(registros[CC] == 0)
        registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;
}
void JN(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x02<<30)) == (2<<30))
        registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;
}
void JNZ(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x01<<30)) == 0)
        registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;
}
void JNP(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x02<<30)) == (2<<30) || (registros[CC] & 0x01<<30) == (1<<30))
        registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;
}
void JNN(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    if((registros[CC] & (0x02<<30)) < (2<<30))
        registros[IP] = get(registros[OP1], registros, memoria, tablaSegmentos) & 0x0000FFFF;
}

void ADD(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (int32_t)get(registros[OP1],registros,memoria,tablaSegmentos) + (int32_t)get(registros[OP2],registros,memoria,tablaSegmentos);
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SUB(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int32_t resultado;
    resultado = get(registros[OP1],registros,memoria,tablaSegmentos) - get(registros[OP2],registros,memoria,tablaSegmentos);
    //printf("operacion SUB: %d = %d - %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
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
    resultado = ((uint32_t)(get(registros[OP1],registros,memoria,tablaSegmentos)) << get(registros[OP2],registros,memoria,tablaSegmentos));
    //printf("SHL: %d = %d << %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SHR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((uint32_t)(get(registros[OP1],registros,memoria,tablaSegmentos)) >> get(registros[OP2],registros,memoria,tablaSegmentos));
    //printf("SHR: %d = %d >> %d\n",resultado,get(registros[OP1],registros,memoria,tablaSegmentos),get(registros[OP2],registros,memoria,tablaSegmentos));
    actualizarCC(registros,resultado);
    set(registros,memoria,registros[OP1],resultado,tablaSegmentos);
}
void SAR(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if((get(registros[OP1],registros,memoria,tablaSegmentos) & 0x80000000) != 0) // si es neg, el resultado tambien lo sera: agrego unos
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

void PUSH(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    registros[SP] = registros[SP] - 4;
    if(registros[SP] < registros[SS]){
        printf("ERROR: Stack Overflow\n");
        STOP(registros,memoria,tablaSegmentos);
    }
    else{
        operacion_memoria(registros, memoria, registros[SP], get(registros[OP1],registros,memoria,tablaSegmentos), ESCRITURA, 4, tablaSegmentos, registros[SS]);
    }
}
void POP(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    /*
    1- extraer 4 bytes del tope de pila
    2- si al realizar esta accion no se pudo completar porque no habia bytes suficientes o la pila estaba vacia, stack underflow
    3- convertir 4 bytes extraidos en un valor donde el byte mas significativo es el tope de la pila y el menos significativo es el byte mas abajo en la pila de esos 4
    4- asignar el valor extraido al primer operando, si el operando es menos a 4 bytes se truncan los mas significativos
    5- incrementar SP en 4
    */
    int32_t operando_memoria = 0x0  |(0x7 << 16) | (0x3 << 24); //creo un falso operando que sería el Stack Pointer
    int32_t valor;
    uint32_t seg_index = registros[SS] >> 16;
    uint32_t stack_bottom = (registros[SS] & 0xFFFF0000) + tablaSegmentos[seg_index].tamanio;
    if(registros[SP] == stack_bottom || registros[SP] + 4 > stack_bottom){
        printf("ERROR: Stack Underflow\n");
        STOP(registros,memoria,tablaSegmentos);
    }
    else{
        valor = get(operando_memoria,registros,memoria,tablaSegmentos);
        registros[SP] += 4;
        set(registros,memoria,registros[OP1],valor,tablaSegmentos);
    }
}

void CALL(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
    /**
     * creamos un falso operando registro que almacenaría el valor del IP y lo guardamos en el registro OP1 para que lo use la instruccion PUSH
     * le devolvemos su valor anterior al registro OP1 y hacemos JMP
    */
    uint32_t aux = registros[OP1];
    uint32_t operando_registro = 0x0 | (0x1 << 24) | 0x3;
    registros[OP1] = operando_registro;
    PUSH(registros,memoria,tablaSegmentos);
    registros[OP1] = aux;
    JMP(registros,memoria,tablaSegmentos);
}/*
void RET(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
   int32_t operando_registro = 0x0 | (0x1 << 24) | 0x7;
   printf("RET\n");
   printf("IP: %08X\n",registros[IP]);
   registros[OP1] = operando_registro;
   //no puedo usar POP
   //leer los siguientes 4 bytes y ponerlos en el IP
   operacion_memoria(registros,memoria,registros[SP],0,LECTURA,4,tablaSegmentos,registros[SS]);
   registros[IP] = registros[MBR];
   registros[SP] += 4;
   printf("IP: %08X\n",registros[IP]);
}*/
void RET(uint32_t registros[], uint8_t memoria[],infoSegmento tablaSegmentos[]){
   int32_t operando_registro = 0x0 | (0x1 << 24) | 0x3;  // ✓ registro IP
   registros[OP1] = operando_registro;
   POP(registros, memoria, tablaSegmentos);
   // POP ya pone el valor en IP (vía set) y actualiza SP
}
void NO_ACCESIBLE(uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    printf("INSTRUCCION INVALIDA: codigo de operacion de la instruccion a ejecutar no existe\n");  // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}
