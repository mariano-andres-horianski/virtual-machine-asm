#include "maquina.h"

int main(int argc, char *argv[])
//los argumentos en el main se obtienen desde consola
//argc= contador de argumentos. *argv[]= vector de cadenas con los argumentos
// ejemplo: "vmx filename.vmx -d" argc=3 argv=["vmx","filename.vmx","-d"]
{
    uint8_t memoria[MEM];                   // memoria principal de 16 KiB
    infoSegmento tablaSegmento[ENT];        // tabla de segmentos
    uint32_t registros[REG];                // registros de la máquina
    // Vector de punteros a funciones
    int tam=0, resultado=0;
    
    if (argc>=2)
        tam=strlen(argv[1]);
    if (argc < 2 || argc > 3||strstr(argv[0], "vmx") == NULL||tam<5||(argv[1][tam-1]!='x'||argv[1][tam-2]!='m'||argv[1][tam-3]!='v'||argv[1][tam-4]!='.')) {
        //verifico que la entrada por terminal sea valida
        printf("Modo de uso: vmx filename.vmx [-d]\n");
        return 1;
    }
    else {
        //argv[1] es el nombre del archivo

        leerEncabezado(argv[1], registros, tablaSegmento, memoria, &resultado);
        printf("resultado: %d\n", resultado);
        if (resultado){
            printf("Inicio de ejecucion del programa %s\n", argv[1]);
            ejecucion(registros, tablaSegmento, memoria);
            printf("Fin de ejecucion del programa\n");
            if (argc == 3 && strcmp(argv[2], "-d") == 0)
                disassembler(memoria, tablaSegmento, tablaSegmento[0].tamanio, registros);
            return 0;
        }
        else
            return 1;
    }
}
