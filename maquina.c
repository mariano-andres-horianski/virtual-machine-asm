#include "maquina.h"
#include <stdlib.h>
#include <stdio.h>
void inicioRegistro(uint32_t reg[]){
    reg[CS] = 0x00000000;
    reg[DS] = 0x00010000;
    reg[IP] = reg [CS];
}
void inicioTablaSegmento(infoSegmento tabla[],uint16_t tamanioCod){
    tabla[0].base = 0x0000;
    tabla[0].tamanio = tamanioCod;

    tabla[1].base = tamanioCod;
    tabla[1].tamanio = MEM - tamanioCod;
}
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){
    FILE *arch;
    char ident[6];    
    char version;
    uint16_t tamanio;
    int resultado = 0;

    arch = fopen(nombre,"rb");
    
    if(arch!=NULL){
        fseek(arch,0,0); // inicio de archivo
        if( fread(ident,sizeof(char),5,arch) == 5){
            ident[5] = '\0';
            if (strcmp(ident,"VMX25") == 0){ 
                fread(&version,sizeof(char),1,arch);
                if(version == 1){
                    fread(&tamanio,sizeof(uint16_t),1,arch);
                    if(tamanio < MEM){
                        resultado = 1;
                        fread(memoria,tamanio,1,arch);
                    }
              }
          }
        }
    }
    fclose(arch);
    if(resultado){
        inicioRegistro(registros);
        inicioTablaSegmento(tablaSegmento,tamanio);
    }
    else 
        printf("ERROR: no se pudo leer el resultado\n");

}
uint32_t calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t regIP,int cantBytes){
    uint32_t direcFisica;
    uint32_t numSegmento = (regIP >> 16) & 0x0000FFFF;
    uint32_t desplaz = regIP & 0x0000FFFF; // obtengo los 2 bytes menos significativos
    direcFisica = tablaSegmento[numSegmento].base + desplaz;
 
    uint32_t limSegmento = tablaSegmento[numSegmento].base + tablaSegmento[numSegmento].tamanio;
    uint32_t limAcceso = direcFisica + cantBytes ;
    if(tablaSegmento[numSegmento].base <= direcFisica && limSegmento >= limAcceso)
        return direcFisica;
    else{
        printf("SEGMENTATION  FAULT\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
        return 0xFFFFFFFF;
    }
}

void leerInstrucciones(uint8_t instruccion, uint8_t memoria[], uint32_t registros[REG], uint32_t *dirFisica){
    //usar los OP para calcular los bytes que necesitamos leer y poner en lectura
    
    uint32_t lectura;
    uint8_t cantByteA,cantByteB;
    uint32_t posicion;
    
    registros[OPC]=instruccion & 0x1F;
    registros[OP2]=instruccion >> 6;
    cantByteB = registros[OP2];
    registros[OP1]=(instruccion >> 4) & 0x03;
    cantByteA = registros[OP1];

    if (registros[OP1]==0 && registros[OP2]!=0)
        registros[OP1]=registros[OP2];
    if ((OPC!=0x0F && registros[OP1]==0)||(OPC<=0x1F && OPC>=0x10 && registros[OP2]==0)){ //reviso si es una operacion invalida
        printf("ERROR: operacion invalida\n");
        *dirFisica = 0xFFFFFFFF;
    }
    else {
        //aca hay que avanzar el IP en 1 y leer byte a byte OP2 veces
        posicion = memoria[registros[IP] + 1];

        operandos(&lectura,registros[OP2],registros,memoria,&posicion);  /// agregar a .h ----------------------------------------?
        registros[OP2]=registros[OP2] << 24;
        registros[OP2]=registros[OP2] | lectura;
        //aca hay que avanzar el IP en 1 y leer byte a byte OP1 veces
        operandos(&lectura,registros[OP1],registros,memoria,&posicion);
        registros[OP1]=registros[OP1] << 24;
        registros[OP1]=registros[OP1] | lectura;

        //Aca ya tengo OPC, OP1 y OP2 para ejecutar

    }
    registros[IP] = registros[IP] + cantByteA + cantByteB + 1; // IP apunta a nueva direccion en memoria
}

void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint8_t memoria[MEM]){
    uint8_t instruccion;
    uint32_t dirFisica;
    uint8_t codInstruccion;
    uint8_t Tipo1,Tipo2;
    uint32_t OP2,OP1;
    
    registros[IP] = registros[CS];
    dirFisica = calcDirFisica(tablaSegmento,registros[IP],1);  // 1?? ref de la cantidad de bytes de acceso
    while (dirFisica != 0xFFFFFFFF){
        leerInstrucciones(memoria[registros[IP]], memoria, registros, &dirFisica); //la dirFisica es una posicion del vector memoria que tenemos que chequear constantemente
        //aca iria una funcion que ejecute las instrucciones en base a lo que hay en el vector registros en las posiciones OP1, OP2 y OPC
        //lo haria accediendo a posiciones de un vector de punteros a funciones donde cada una es una instruccion
        
        codInstruccion = memoria[registros[IP]];
        Tipo1 = (registros[OP1] >> 30 ) & 0x3;
        Tipo2 = (registros[OP2] >> 30 ) & 0x3;
        OP1 = registros[OP1] & 0x00FFFFFF;
        OP2 = registros[OP1] & 0x00FFFFFF;
       // instrucciones[codInstruccion](Tipo1,Tipo2,OP1,OP2,registros,memoria);
    }
}

void operandos(uint32_t *lectura,uint32_t tipo,uint32_t registros[],uint8_t memoria[],uint32_t *posicion){
    switch (tipo){
    case 0b01: {
        *lectura = memoria[*posicion]; 
        *posicion += 1;
        break;
    }
    case 0b10: {
        *lectura = (memoria[*posicion] << 8)  |  memoria[*posicion + 1];
        *posicion += 2;
    }
    case 0b11: {
        *lectura = (memoria[*posicion] << 16) | (memoria[*posicion + 1] << 8) | (memoria[*posicion + 2]);
        *posicion += 3;
    }
    }
}
void ResultadoOperacion(uint8_t Tipo1,uint32_t registros[],uint8_t memoria[],int resultado,infoSegmento tablaSegmentos[]){
    uint32_t codRegistro;
    uint32_t direFisica;
    if( Tipo1 == 0b01){ // registro
        codRegistro = registros[OP1] & 0x00FFFFFF;
        registros[codRegistro] = resultado;
    }
    else {
        if(Tipo1 == 0b11) { // memoria
            direFisica = calcDirFisica(tablaSegmentos,registros[IP],1);
            memoria[direFisica] =  (resultado >> 24) & 0x000000FF;
            memoria[direFisica + 1] = (resultado >> 16) & 0x00FF;
            memoria[direFisica + 2] = (resultado >> 8) & 0x0000FF;
            memoria[direFisica + 3] = resultado & 0x000000FF;
        }
    }
}
void actualizarCC(uint32_t registros[],uint32_t resultado){
    if(resultado < 0)
        registros[CC] = 0x80000000;
    else
        if(resultado == 0)
            registros[CC] = 0x40000000;
        else 
            registros[CC] = 0x0;
}
void funcion_ADD(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 + OP2;
    actualizarCC(registros,resultado); // ----------------------se tiene en cuenta el orden de las funciones actCC y luego resultado??
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_SUB(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 - OP2;
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_MUL(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 * OP2;
    actualizarCC(registros,resultado); 
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos); 
}
void funcion_DIV(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(OP2 != 0){
        resultado = OP1 / OP2;
        actualizarCC(registros,resultado);
        registros[AC] = OP1 % OP2;
        ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
    }
    else 
        printf("ERROR: division por cero\n"); // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}
void funcion_CMP(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 - OP2;
    actualizarCC(registros,resultado);
}
void funcion_SHL(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (OP1 << OP2) & ~(0x1 << OP2);  // ???
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_SHR(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (OP1 >> OP2) & ~(0x1 >> OP2);  // ???
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_SAR(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    if(OP1 & 0x80000000 == 1) // si es neg, el resultado tambien lo sera: agrego unos
        resultado = OP1 >> OP2;
    else 
        resultado = (OP1 >> OP2) & ~(0x1 >> OP2); // si es positivo: agrego ceros
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_AND(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 & OP2;
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_OR(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 | OP2;
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_XOR(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = OP1 ^ OP2;
    actualizarCC(registros,resultado);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_SWAP(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    ResultadoOperacion(Tipo1,registros,memoria,OP2,tablaSegmentos);
    ResultadoOperacion(Tipo2,registros,memoria,OP1,tablaSegmentos);
}
void funcion_LDL(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = (OP2 & 0x0000FFFF) | (OP1 & 0xFFFF0000);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_LDH(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ((OP2 << 16) & 0xFFFF0000) | (OP1 & 0x0000FFFF);
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_NOT(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    int resultado;
    resultado = ~OP1;
    ResultadoOperacion(Tipo1,registros,memoria,resultado,tablaSegmentos);
}
void funcion_STOP(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
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
void funcion_NO_ACCESIBLE(uint8_t Tipo1,uint8_t Tipo2,int OP1,int OP2,uint32_t registros[],uint8_t memoria[],infoSegmento tablaSegmentos[]){
    printf("INSTRUCCION INVALIDA: codigo de operacion de la instruccion a ejecutar no existe\n");  // detecta uno de los 3 errores que se deben tener en cuenta segun requisitos
}