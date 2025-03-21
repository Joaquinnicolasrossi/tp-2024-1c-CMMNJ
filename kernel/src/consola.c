#include "consola.h"
#include "planificador.h"
#include "kernel_cpu_dispatch.h"

int pid_buscado = 0;

void inicializar_consola()
{
    log_info(kernel_logger, "CONSOLA INICIALIZADA");
    char *leido;
    leido = readline("> Ingrese un comando: ");

    while (strcmp(leido, "\0") != 0)
    {
        if (!validar_instruccion(leido))
        {
            free(leido);
            leido = readline("> Ingrese un comando: ");
            continue; // Salta y continua con el resto de la iteracion
        }

        atender_instruccion(leido);
        free(leido);
        leido = readline("> Ingrese un comando: ");
    }
    free(leido);
}

bool validar_instruccion(char *leido)
{
    bool resultado_validacion = false;

    // TODO: VALIDAR PATH, PID y VALOR

    char **comando_consola = string_split(leido, " ");

    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0)
    {
        resultado_validacion = true;
    }
    else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0)
    {
        resultado_validacion = true;
    }
    else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0)
    {
        resultado_validacion = true;
    }
    else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0)
    {
        if (planif_iniciada == false)
        {
            resultado_validacion = true;
        }
        else
        {
            resultado_validacion = false;
            printf("La planificacion ya ha sido iniciada previamente \n");
        }
    }
    else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0)
    {
        if (planif_iniciada == true)
        {
            resultado_validacion = true;
        }
        else
        {
            resultado_validacion = false;
            printf("La planificacion ya ha sido pausada previamente \n");
        }
    }
    else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0)
    {
        resultado_validacion = true;
    }
    else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0)
    {
        resultado_validacion = true;
    }
    else
    {
        log_error(kernel_logger, "ERROR: Comando NO reconocido");
        resultado_validacion = false;
    }
    string_array_destroy(comando_consola);

    return resultado_validacion;
}

void atender_instruccion(char *leido)
{
    char **comando_consola = string_split(leido, " ");
    // t_buffer *un_buffer = crear_buffer();

    if (strcmp(comando_consola[0], "EJECUTAR_SCRIPT") == 0)
    {
        ejecutar_script(comando_consola[1]);
    }
    else if (strcmp(comando_consola[0], "INICIAR_PROCESO") == 0)
    {
        iniciar_proceso(comando_consola[1]);
    }
    else if (strcmp(comando_consola[0], "FINALIZAR_PROCESO") == 0)
    {
        pid_buscado = atoi(comando_consola[1]);
        finalizar_proceso();
    }
    else if (strcmp(comando_consola[0], "DETENER_PLANIFICACION") == 0)
    {
        detener_planificacion();
    }
    else if (strcmp(comando_consola[0], "INICIAR_PLANIFICACION") == 0)
    {
        iniciar_planificacion();
    }
    else if (strcmp(comando_consola[0], "MULTIPROGRAMACION") == 0)
    {
        int valor = atoi(comando_consola[1]);
        cambiar_grado_multiprogramacion(valor);
    }
    else if (strcmp(comando_consola[0], "PROCESO_ESTADO") == 0)
    {
        listar_procesos_por_estado();
    }
}

int asignar_pid()
{
    int valor_pid;

    pthread_mutex_lock(&mutex_pid);
    valor_pid = identificador_pid;
    identificador_pid++;
    pthread_mutex_unlock(&mutex_pid);

    return valor_pid;
}

t_pcb *crear_pcb(int pid)
{
    // CREACION DE PCB CON VALORES EN 0

    t_pcb *pcb = malloc(sizeof(t_pcb));

    pcb->pid = pid;
    pcb->program_counter = 0;
    pcb->estado = NEW;
    pcb->motivo_block = NONE_BLOCK;
    pcb->motivo_exit = NONE_EXIT;
    if (strcmp(algoritmo_planificacion, "VRR") == 0)
    {
        pcb->quantum_remanente = quantum;
    }
    else
    {
        pcb->quantum_remanente = 0;
    }
    t_registros *registros = malloc(sizeof(t_registros));
    pcb->registros_cpu = registros;
    inicializar_registros_pcb(pcb);
    pcb->recursos_usados = string_array_new();
    agregar_pcb(lista_global_pcb, pcb, &mutex_lista_global_pcb);
    return pcb;
}

void inicializar_registros_pcb(t_pcb *pcb)
{

    pcb->registros_cpu->ax = 0;
    pcb->registros_cpu->bx = 0;
    pcb->registros_cpu->cx = 0;
    pcb->registros_cpu->dx = 0;
    pcb->registros_cpu->eax = 0;
    pcb->registros_cpu->ebx = 0;
    pcb->registros_cpu->ecx = 0;
    pcb->registros_cpu->edx = 0;
    pcb->registros_cpu->si = 0;
    pcb->registros_cpu->di = 0;
}

void iniciar_proceso(char *path)
{

    // CREAR PCB
    int pid = asignar_pid();
    t_pcb *pcb = crear_pcb(pid);

    // AGREGAR PCB A NEW
    log_info(kernel_logger, "Se crea el proceso %d en NEW", pid);
    agregar_pcb(cola_new, pcb, &mutex_cola_new);
    sem_post(&sem_new);

    // ENVIAR A MEMORIA PATH Y PID
    t_buffer *a_enviar = crear_buffer();
    agregar_string_a_buffer(a_enviar, path);
    agregar_uint32_a_buffer(a_enviar, pcb->pid);
    t_paquete *paquete = crear_super_paquete(CREAR_PROCESO, a_enviar);
    enviar_paquete(paquete, socket_conexion_memoria);
    eliminar_paquete(paquete);
}

void ejecutar_script(char *path)
{
    FILE *archivo_instrucciones = fopen(path, "r");
    if (!archivo_instrucciones)
    {
        log_error(kernel_logger, "Hubo un error abriendo el archivo");
    }
    long int tamanio_archivo = tamanio_del_archivo(archivo_instrucciones); // Vemos el tamaño del archivo

    char *archivo_string = malloc(tamanio_archivo + 1);               
    fread(archivo_string, tamanio_archivo, 1, archivo_instrucciones); // leo
    if (archivo_string == NULL)
    {
        log_error(kernel_logger, "El archivo se encuentra vacio");
    }
    archivo_string[tamanio_archivo] = '\0'; // Agregamos el caracter centinela 
    fclose(archivo_instrucciones);          

    char **array_instrucciones = string_split(archivo_string, "\n"); // Separamos lo leído por línea ya que cada línea es una instrucción y prosigo a liberar la memoria de lo leído
    int tamanio_array_ins = string_array_size(array_instrucciones);
    free(archivo_string);

    for (size_t i = 0; i < tamanio_array_ins; i++)  // Agarro cada instruccion del array y la ejecuto 
    {
        char* instruccion = array_instrucciones[i];
        char** instruccion_separada = string_split(instruccion, " ");
        ejecutar_instruccion(instruccion_separada);
    }
    
    string_array_destroy(array_instrucciones);
}

long int tamanio_del_archivo(FILE *archivo)
{
    fseek(archivo, 0, SEEK_END);
    long int tamanio_archivo = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);
    return tamanio_archivo;
}

void ejecutar_instruccion(char** instruccion_separada){
    if (strcmp(instruccion_separada[0], "INICIAR_PROCESO") == 0)
    {
        iniciar_proceso(instruccion_separada[1]);
    }
    // TODO MANEJO DE LOS DEMAS COMANDOS
}

void finalizar_proceso()
{
    // PRIMERO BUSCO EN COLA NEW
    t_pcb *pcb = (t_pcb *)list_remove_by_condition(cola_new, es_pcb_buscado);
    if (pcb != NULL)
    {
        sem_wait(&sem_new);
        cambiar_estado(pcb, FINISH_ERROR);
        pcb->motivo_exit = INTERRUPTED_BY_USER;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        return;
    }

    // LUEGO BUSCO EN COLA READY
    pcb = (t_pcb *)list_remove_by_condition(cola_ready, es_pcb_buscado);
    if (pcb != NULL)
    {
        sem_wait(&sem_ready);
        cambiar_estado(pcb, FINISH_ERROR);
        pcb->motivo_exit = INTERRUPTED_BY_USER;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        return;
    }

    // LUEGO BUSCO EN COLA READY PRIORIDAD
    pcb = (t_pcb *)list_remove_by_condition(cola_ready_prioridad, es_pcb_buscado);
    if (pcb != NULL)
    {
        sem_wait(&sem_ready_prioridad);
        cambiar_estado(pcb, FINISH_ERROR);
        pcb->motivo_exit = INTERRUPTED_BY_USER;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        return;
    }

    // SI ESTA EN COLA EXEC, MANDO INTERRUPCION A CPU
    if (!list_is_empty(cola_execute))
    {
        pcb = (t_pcb *)list_get(cola_execute, 0);
        if (pcb->pid == pid_buscado)
        {
            op_code codigo = INT_FINALIZAR_PROCESO;
            send(socket_conexion_cpu_interrupt, &codigo, sizeof(op_code), 0);
            return;
        }
    }

    // BUSCO EN COLA DE BLOCK DE LOS RECURSOS
    for (int i = 0; i < list_size(lista_recursos); i++)
    {
        t_recurso *recurso = list_get(lista_recursos, i);
        pcb = (t_pcb *)list_remove_by_condition(recurso->cola_block_asignada, es_pcb_buscado);

        if (pcb != NULL)
        {
            remove_string_from_array(&pcb->recursos_usados, recurso->recurso);
            recurso->instancias++;
            for (int i = 0; i < string_array_size(pcb->recursos_usados); i++)
            {
                char *nombre_recurso = pcb->recursos_usados[i];
                t_recurso *recurso = buscar_recurso(nombre_recurso);
                recurso->instancias++;
                if (recurso->instancias <= 0)
                {
                    t_pcb *pcb_bloqueado = remover_pcb(recurso->cola_block_asignada, &recurso->mutex_asignado);
                    agregar_pcb(cola_block, pcb_bloqueado, &mutex_cola_block);
                    sem_post(&sem_block_return);
                }
            }
            cambiar_estado(pcb, FINISH_ERROR);
            pcb->motivo_exit = INTERRUPTED_BY_USER;
            agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
            sem_post(&sem_exit);
            return;
        }
    }

    // BUSCO EN COLA DE BLOCK DE LAS IO CONECTADAS
    for (int i = 0; i < list_size(lista_io_conectadas); i++)
    {
        t_interfaz_kernel *interfaz = list_get(lista_io_conectadas, i);
        pcb = (t_pcb *)list_remove_by_condition(interfaz->cola_block_asignada, es_pcb_buscado);
        if (pcb != NULL)
        {
            cambiar_estado(pcb, FINISH_ERROR);
            pcb->motivo_exit = INTERRUPTED_BY_USER;
            agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
            sem_post(&sem_exit);
            return;
        }
    }

    log_warning(kernel_logger, "No se encontro el pid a finalizar"); // En caso de que la funcion no retorne antes
};

bool es_pcb_buscado(void *data)
{
    t_pcb *pcb = (t_pcb *)data;
    return pid_buscado == pcb->pid;
}

void detener_planificacion() {
    planif_iniciada = false;
    printf("La planificacion ha sido pausada. \n");
    sem_wait(&sem_planif_new);
    sem_wait(&sem_planif_ready);
    sem_wait(&sem_planif_ready_prioridad);
    sem_wait(&sem_planif_exec); // PARA ENTRADA A EXEC
    sem_wait(&sem_planif_exec); // PARA VUELTA DE EXEC (ENVIO_PCB)
    sem_wait(&sem_planif_block);
}

void iniciar_planificacion() {
    planif_iniciada = true;
    if (primera_vez_planif == true)
    {
        planificar();
    }
    primera_vez_planif = false;
    printf("La planificacion ha sido reanudada. ALGORITMO: %s \n", algoritmo_planificacion);
    sem_post(&sem_planif_new);
    sem_post(&sem_planif_ready);
    sem_post(&sem_planif_ready_prioridad);
    sem_post(&sem_planif_exec); // PARA ENTRADA A EXEC
    sem_post(&sem_planif_exec); // PARA VUELTA DE EXEC (ENVIO_PCB)
    sem_post(&sem_planif_block);
}

void cambiar_grado_multiprogramacion(int valor)
{

    int grado_actual = grado_multiprogramacion;

    if (grado_actual < valor)
    {

        for (int i = 0; i < (valor - grado_actual); i++)
        {
            sem_post(&sem_multiprogramacion);
        }
        grado_multiprogramacion = valor;
        printf("El grado de multiprogramacion ahora es de %d\n", grado_multiprogramacion);
        return;
    }

    if (grado_actual > valor)
    {
        for (int i = 0; i < (grado_actual - valor); i++)
        {
            sem_wait(&sem_multiprogramacion);
        }
        grado_multiprogramacion = valor;
        printf("El grado de multiprogramacion ahora es de %d\n", grado_multiprogramacion);
        return;
    }

}

void listar_procesos_por_estado() {

    printf("Procesos en estado NEW:\n");
    estado_filtrado = NEW;
    list_iterate(cola_new, imprimir_pcb_estado);

    printf("Procesos en estado READY:\n");
    estado_filtrado = READY;
    list_iterate(cola_ready, imprimir_pcb_estado);

    printf("Procesos en estado READY-PRIORIDAD:\n");
    estado_filtrado = READY_PRIORIDAD;
    list_iterate(cola_ready_prioridad, imprimir_pcb_estado);

    printf("Procesos en estado EXECUTE:\n");
    estado_filtrado = EXEC;
    list_iterate(cola_execute, imprimir_pcb_estado);

    printf("Procesos en estado BLOCK:\n");
    estado_filtrado = BLOCK;
    list_iterate(cola_block, imprimir_pcb_estado);
    for (int i = 0; i < list_size(lista_recursos); i++)
    {
        t_recurso* elemento = list_get(lista_recursos, i);
        list_iterate(elemento->cola_block_asignada, imprimir_pcb_estado);
    }
    for (int i = 0; i < list_size(lista_io_conectadas); i++)
    {
        t_interfaz_kernel* elemento = list_get(lista_io_conectadas, i);
        list_iterate(elemento->cola_block_asignada, imprimir_pcb_estado);
    }

};

void imprimir_pcb_estado(void* data) {
    t_pcb* pcb = (t_pcb*) data;
    if (pcb->estado == estado_filtrado) {
        printf("PID: %d\n", pcb->pid);
    }
}