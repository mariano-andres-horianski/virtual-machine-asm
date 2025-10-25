#include "maquina.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    

    uint8_t *memoria = NULL;
    uint32_t registros[REG];
    infoSegmento tablaSegmento[ENT];
    uint8_t num_segmentos = 0;
    int resultado = 0;
    uint8_t version = 0;

    char *archivoVMX = NULL;
    char *archivoVMI = NULL;
    int mostrarDisassembler = 0;
    int tamano_memoria = MEM;
    int indiceParametros = -1;

    char *nombreArchivo;

    uint32_t tamParamSegment;
    int argc_param;

    if (argc < 2 || !strstr(argv[0], "vmx")) {
        printf("Modo de uso:\n");
        printf("vmx [archivo.vmx|archivo.vmi] [archivo.vmi] [m=M] [-d] [-p param1 ...]\n");
        return 1;
    }

    //Procesamiento de argumentos
    for (int i = 1; i < argc; i++) {
        if (strstr(argv[i], ".vmx"))
            archivoVMX = argv[i];
        else 
            if (strstr(argv[i], ".vmi"))
                archivoVMI = argv[i];
            else 
                if (strncmp(argv[i], "m=", 2) == 0)
                    tamano_memoria = (atoi(argv[i] + 2))*1024; //atoi convierte un string a un entero, argc[i]+2 accede al "xxx" de "m=xxx"
                else 
                    if (strcmp(argv[i], "-d") == 0)
                        mostrarDisassembler = 1;
                    else 
                        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc && archivoVMX != NULL) //se fija si -p tiene al menos un parametro y si hay un archivo vmx
                            indiceParametros = i + 1; //guarda el indice al primer parametro
    }

 
    memoria = (uint8_t *)malloc(tamano_memoria);
    if (!memoria) {
        printf("Error: no se pudo reservar memoria\n");
        return 1;
    }

    //Cargar archivo y detectar version
    nombreArchivo = archivoVMX ? archivoVMX : archivoVMI;
    if (!nombreArchivo) {
        printf("Error: debe especificarse un archivo .vmx o .vmi\n");
        free(memoria);
        return 1;
    }

    tamParamSegment = 0;
    registros[31] = 0xFFFFFFFF;
    if (indiceParametros != -1) {
        argc_param = argc - indiceParametros;
        construirParamSegment(memoria, &argv[indiceParametros], argc_param, &tamParamSegment);

        tablaSegmento[0].base = 0x0000;
        tablaSegmento[0].tamanio = tamParamSegment;
        registros[31] = 0x00000000;

        //num_segmentos = 1;
    }


    version = detectarVersion(nombreArchivo);
    leerEncabezado(nombreArchivo, registros, tablaSegmento, memoria, &resultado, &num_segmentos, tamParamSegment);

    if (!resultado) {
        printf("No se pudo ejecutar el programa.\n");
        free(memoria);
        return 1;
    }

    printf("Inicio de ejecucion del programa %s (version %d)\n", nombreArchivo, version);
    ejecucion(registros, tablaSegmento, memoria, argc, argv);
    printf("Fin de ejecucion del programa\n");

    if (mostrarDisassembler) {
        if (version == 2)
            disassemblerMV2(memoria, tablaSegmento, registros);
        else
            disassembler(memoria, tablaSegmento, tablaSegmento[0].tamanio, registros);
    }


    if (indiceParametros != -1) {
        printf("Parámetros pasados al programa:\n");
        for (int i = indiceParametros; i < argc; i++)
            printf("  %s\n", argv[i]);
    }

    free(memoria);
    return 0;
}
