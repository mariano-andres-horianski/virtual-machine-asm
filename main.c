#include "maquina.h"

int main()
{
    uint8_t memoria[MEM];                         // memoria de 16 kib
    infoSegmento tablaSegmento[ENT];                    // tabla de segmentos sin inicializar
    uint32_t registros[REG];                            // cada pos tiene disponible 4 bytes >> con unsigned char tiene 1 byte

    leerEncabezado("sample.vmx",registros,tablaSegmento,memoria);
    printf("inicio de ejecucion");
   // ejecucion(registros,tablaSegmento,memoria);
       
    return 0;
}

/// FUNCIONES
