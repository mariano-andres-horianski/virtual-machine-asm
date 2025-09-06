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
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT]);




int main()
{
    uint32_t memoria[MEM];                         // memoria de 16 kib
    infoSegmento tablaSegmento[ENT];                    // tabla de segmentos sin inicializar
    uint32_t registros[REG];                            // cada pos tiene disponible 4 bytes >> con unsigned char tiene 1 byte

    
    leerEncabezado("sample.vmx",registros,tablaSegmento);
       
    
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
void leerEncabezado(char nombre[],uint32_t registros[REG],infoSegmento tablaSegmento[ENT]){
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
                    if(tamanio < MEM)
                        resultado = 1;
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
