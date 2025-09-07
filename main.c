#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEM 16384
#define ENT 8
#define REG 32

typedef enum {
    LAR = 0,MAR,MBR,IP,OPC,OP1,OP2,
    EAX = 10,EBX,ECX,EDX,EEX,EFX,AC,CC,
    CS = 26,DS } registro;

typedef struct {
    unsigned short base;
    unsigned short tamanio; } infoSegmento;              //unsigned short tiene exact. 2 bytes

void inicioRegistro(uint32_t reg[]);
void inicioTablaSegmento(infoSegmento tabla[],uint16_t tamanio);
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint32_t memoria[MEM]);
uint32_t calcDirFisica(infoSegmento tablaSegmento[ENT],uint32_t regIP,int cantBytes);
int operando(uint8_t tipo);
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint32_t memoria[MEM]);

int main()
{
    uint32_t memoria[MEM];                         // memoria de 16 kib
    infoSegmento tablaSegmento[ENT];                    // tabla de segmentos sin inicializar
    uint32_t registros[REG];                            // cada pos tiene disponible 4 bytes >> con unsigned char tiene 1 byte

    leerEncabezado("sample.vmx",registros,tablaSegmento,memoria);
    printf("inicio de ejecucion");
    ejecucion(registros,tablaSegmento,memoria);
       
    return 0;
}

/// FUNCIONES
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
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint32_t memoria[MEM]){
    FILE *arch;
    char ident[5];    
    char version;
    uint16_t tamanio;
    int resultado = 0;

    arch = fopen(nombre,"rb");
    
    if(arch!=NULL){
        fseek(arch,0,0); // inicio de archivo
        if( fread(ident,sizeof(char),5,arch) == 5){
            if (strcmp(ident,"VMX25")){ 
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
        printf("ERROR: no se pudo leer el resultado");

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
    else 
        return 0xFFFFFFFF;
}
int operando(uint8_t tipo){
    int cantByte;
    if(tipo == 0x01)
        cantByte = 1; // registro
    else 
        if(tipo == 0x02) // inmediato
            cantByte = 2;
        else
            if(tipo == 0x03) // memoria
                cantByte = 3;
            else 
                cantByte = 0;
    return cantByte;
}
void ejecucion(uint32_t registros[REG],infoSegmento tablaSegmento[ENT],uint32_t memoria[MEM]){
    uint32_t instruccion, dirFisica;
    uint8_t tipoA, tipoB, codInstruccion;
    int valorA, valorB,cantByteA,cantByteB;
    registros[IP] = registros[CS];
    dirFisica = calcDirFisica(tablaSegmento,registros[IP],1);  // 1?? ref de la cantidad de bytes de acceso
    while (dirFisica != 0xFFFFFFFF){
        instruccion = memoria[registros[IP]]; 
        codInstruccion = instruccion & 0x1F;
        tipoA =  (codInstruccion >> 6) & 0x3;// 0x3 para que no sean shift aritmeticos
        tipoB = (instruccion >> 4) & 0x3;
        cantByteA = operando(tipoA);
        cantByteB = operando(tipoB);
        registros[IP] = registros[IP] + cantByteA + cantByteB + 1; // IP apunta a nueva direccion en memoria
    }
}
