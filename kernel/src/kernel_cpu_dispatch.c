#include "kernel_cpu_dispatch.h"
#include "planificador.h"

void atender_cpu_dispatch() {
    bool control_key = 1;
    while (control_key)
    {
        int cod_op = recibir_operacion(socket_conexion_cpu_dispatch);
        switch (cod_op)
        {
        case MENSAJE:
            // 
            break;
        case PAQUETE:
            // 
            break;
        case ENVIO_PCB:
            // OBTENGO EL CONTEXTO DE LA CPU
            t_buffer *buffer = recibir_buffer(socket_conexion_cpu_dispatch);
            t_pcb *contexto_recibido = extraer_pcb_de_buffer(buffer);
            if (chequear_quantum(contexto_recibido))
            {
                pthread_cancel(hilo_quantum);
            }
            if (strcmp(algoritmo_planificacion, "VRR") == 0)
            {
                temporal_stop(tiempo_exec);
            }
            log_info(kernel_logger, "Recibi un PCB que me envio el CPU");
            if(contexto_recibido->motivo_exit == FIN_QUANTUM) {
                log_info(kernel_logger, "PID: %d - Desalojado por fin de Quantum", contexto_recibido->pid);
            }
            sem_wait(&sem_planif_exec);
            // ACTUALIZO EL PCB QUE ESTABA EN EXEC
            t_pcb *pcb = remover_pcb(cola_execute, &mutex_cola_exec);
            pcb = contexto_recibido;
            int cop = extraer_int_de_buffer(buffer);
            // RECIBO OTRO COP PARA SABER QUE TENGO QUE HACER CON EL PCB
            
            switch (cop)
            {
                case CAMBIAR_ESTADO:
                    log_info(kernel_logger, "Recibi un aviso de cambio de estado");
                    estado_proceso nuevo_estado = extraer_estado_proceso_de_buffer(buffer);
                    procesar_cambio_estado(pcb, nuevo_estado);
                    sem_post(&sem_exec);
                    break;
                case ATENDER_WAIT:
                    log_info(kernel_logger, "Recibi un aviso de atender WAIT");
                    char* recurso_wait = extraer_string_de_buffer(buffer);
                    atender_wait(pcb, recurso_wait);
                    free(recurso_wait);
                    break;
                case ATENDER_SIGNAL:
                    log_info(kernel_logger, "Recibi un aviso de atender SIGNAL");
                    char* recurso_signal = extraer_string_de_buffer(buffer);
                    atender_signal(pcb, recurso_signal);
                    free(recurso_signal);
                    break;
                case OP_IO_GEN_SLEEP:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion IO_GEN_SLEEP");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    u_int32_t unidades_de_trabajo = extraer_uint32_de_buffer(buffer);
                    atender_io_gen_sleep(pcb, nombre_interfaz, unidades_de_trabajo);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_STDIN_READ:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion IO_STDIN_READ");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    t_list* direcciones_fisicas = extraer_lista_direcciones_de_buffer(buffer);
                    u_int32_t tamanio = extraer_uint32_de_buffer(buffer);
                    atender_io_stdin_read(pcb, nombre_interfaz, direcciones_fisicas, tamanio);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_STDOUT_WRITE:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion IO_STDOUT_WRITE");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    t_list* direcciones_fisicas = extraer_lista_direcciones_de_buffer(buffer);
                    u_int32_t tamanio = extraer_uint32_de_buffer(buffer);
                    atender_io_stdout_write(pcb, nombre_interfaz, direcciones_fisicas, tamanio);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_FS_CREATE:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion DIALFS_CREATE");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    char* nombre_archivo = extraer_string_de_buffer(buffer);
                    atender_io_fs_create(pcb, nombre_interfaz, nombre_archivo);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_FS_DELETE:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion DIALFS_DELETE");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    char* nombre_archivo = extraer_string_de_buffer(buffer);
                    atender_io_fs_delete(pcb, nombre_interfaz, nombre_archivo);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_FS_TRUNCATE:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion DIALFS_TRUNCATE");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    char* nombre_archivo = extraer_string_de_buffer(buffer);
                    u_int32_t tamanio = extraer_uint32_de_buffer(buffer);
                    atender_io_fs_truncate(pcb, nombre_interfaz, nombre_archivo, tamanio);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_FS_WRITE:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion DIALFS_WRITE");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    char* nombre_archivo = extraer_string_de_buffer(buffer);
                    t_list* lista_direcciones = extraer_lista_direcciones_de_buffer(buffer);
                    u_int32_t tamanio = extraer_uint32_de_buffer(buffer);
                    u_int32_t puntero = extraer_uint32_de_buffer(buffer);
                    atender_io_fs_write(pcb, nombre_interfaz, nombre_archivo, lista_direcciones, tamanio, puntero);
                    free(nombre_interfaz);
                    break;
                }
                case OP_IO_FS_READ:{
                    log_info(kernel_logger, "Recibi un aviso de realizar una operacion DIALFS_READ");
                    char* nombre_interfaz = extraer_string_de_buffer(buffer);
                    char* nombre_archivo = extraer_string_de_buffer(buffer);
                    t_list* lista_direcciones = extraer_lista_direcciones_de_buffer(buffer);
                    u_int32_t tamanio = extraer_uint32_de_buffer(buffer);
                    u_int32_t puntero = extraer_uint32_de_buffer(buffer);
                    atender_io_fs_read(pcb, nombre_interfaz, nombre_archivo, lista_direcciones, tamanio, puntero);
                    free(nombre_interfaz);
                    break;
                }
                default:
                    log_warning(kernel_logger, "Operacion a realizar con el pcb recibido desconocida");
                    break;
                }
            sem_post(&sem_planif_exec);
            break;
        case -1:
            log_error(kernel_logger, "Se desconecto CPU - Dispatch");
            control_key = 0;
            break;
        default:
            log_warning(kernel_logger, "Operacion desconocida de CPU - Dispatch");
            control_key = 0;
            break;
        }
    }
}

void procesar_cambio_estado(t_pcb *pcb, estado_proceso estado_nuevo)
{

    switch (estado_nuevo)
    {
    case READY:
    {
        if (strcmp(algoritmo_planificacion, "VRR") == 0)
        {
            pcb->quantum_remanente = quantum; // REINICIO EL QUANTUM SI EL PCB VOLVIO POR FIN DE QUANTUM
        }
        pasar_a_ready(pcb);
        sem_post(&sem_ready);
    }
        break;
    case FINISH_EXIT:
        cambiar_estado(pcb, estado_nuevo);
        pcb->motivo_exit = SUCCESS;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        break;
    case FINISH_ERROR:
        cambiar_estado(pcb, estado_nuevo);
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        break;
    default:
        log_error(kernel_logger, "Cambio de estado no reconocido");
        break;
    }
}

void atender_wait(t_pcb *pcb, char *recurso)
{
	t_recurso *recursobuscado = buscar_recurso(recurso);

    // ! SI DA ERROR EL RECURSO
	if (recursobuscado->id == -1)
	{
		log_error(kernel_logger, "No existe el recurso solicitado: %s", recurso);
		pcb->motivo_exit = INVALID_RESOURCE;
		agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
		sem_post(&sem_exit);
		sem_post(&sem_exec);
	}
	else
	{
		recursobuscado->instancias--;
		log_info(kernel_logger, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, recurso, recursobuscado->instancias);
        string_array_push(&pcb->recursos_usados, recursobuscado->recurso);
        // ! SI NO HAY SUFICIENTES INSTANCIAS DEL RECURSO DISPONIBLES
		if (recursobuscado->instancias < 0)
		{
			cambiar_estado(pcb, BLOCK);
            pcb->motivo_block = RESOURCE_BLOCK;
			log_info(kernel_logger, "PID: %d - Bloqueado por: %s", pcb->pid, recurso);
			agregar_pcb(recursobuscado->cola_block_asignada, pcb, &recursobuscado->mutex_asignado);
			sem_post(&sem_exec);
		}
        // SI SALE TODO BIEN, VUELVE A MANDAR EL PCB AL CPU
		else
		{
			dispatch_pcb(pcb);
		}
	}
}

t_recurso *buscar_recurso(char *recurso)
{
	int longitudLista = list_size(lista_recursos);
	t_recurso *recursobuscado;
	for (int i = 0; i < longitudLista; i++)
	{
		recursobuscado = list_get(lista_recursos, i);
		if (strcmp(recursobuscado->recurso, recurso) == 0)
		{
			return recursobuscado;
		}
	}
    // ! SI NO ENCUENTRA EL RECURSO, PONE ID = -1
	recursobuscado->id = -1;
	return recursobuscado;
}

void atender_signal(t_pcb *pcb, char *recurso)
{
	t_recurso *recursobuscado = buscar_recurso(recurso);

    // ! SI DA ERROR EL RECURSO
	if (recursobuscado->id == -1)
	{
		log_error(kernel_logger, "No existe el recurso solicitado: %s", recurso);
		pcb->motivo_exit = INVALID_RESOURCE;
		agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
		sem_post(&sem_exit);
		sem_post(&sem_exec);
	}
	else
	{
		recursobuscado->instancias++;
		log_info(kernel_logger, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, recurso, recursobuscado->instancias);
        remove_string_from_array(&pcb->recursos_usados, recursobuscado->recurso);
        // DESBLOQUEA AL PRIMER PROCESO DE LA COLA DE BLOQUEADOS DEL RECURSO SI CORRESPONDE
		if (recursobuscado->instancias <= 0)
		{
			t_pcb *pcb_bloqueado = remover_pcb(recursobuscado->cola_block_asignada, &recursobuscado->mutex_asignado);
			agregar_pcb(cola_block, pcb_bloqueado, &mutex_cola_block);
			sem_post(&sem_block_return);
		}

        // MANDA EL PROCESO RECIBIDO A EXEC
		dispatch_pcb(pcb);
	}
}

void atender_io_gen_sleep(t_pcb *pcb, char* nombre_interfaz, u_int32_t unidades) {

    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.
    
    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if(interfaz == NULL) {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if(!operacion_valida(interfaz, OP_IO_GEN_SLEEP)) {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->unidades_de_trabajo = unidades;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_io_gen_sleep, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_io_gen_sleep(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    int unidades = args->unidades_de_trabajo;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer* buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_uint32_a_buffer(buffer, unidades);    
    t_paquete* paquete = crear_super_paquete(GENERICA, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);
}

t_interfaz_kernel* buscar_interfaz(char* nombre_interfaz) {
    int longitudLista = list_size(lista_io_conectadas);
	t_interfaz_kernel* elemento;
	for (int i = 0; i < longitudLista; i++)
	{
		elemento = (t_interfaz_kernel*) list_get(lista_io_conectadas, i);
		if (strcmp(elemento->nombre, nombre_interfaz) == 0)
		{
			return elemento;
		}
	}
    // ! SI NO ENCUENTRA LA INTERFAZ, RETORNA NULL
	return NULL;
}

bool operacion_valida(t_interfaz_kernel* interfaz, op_code operacion) {

    if ((strcmp(interfaz->tipo, "GENERICA") == 0) && operacion == OP_IO_GEN_SLEEP) {
        return true;
    }

    if ((strcmp(interfaz->tipo, "STDIN") == 0) && operacion == OP_IO_STDIN_READ) {
        return true;
    }

    if ((strcmp(interfaz->tipo, "STDOUT") == 0) && operacion == OP_IO_STDOUT_WRITE) {
        return true;
    }

    if ((strcmp(interfaz->tipo, "DIALFS") == 0) && (operacion == OP_IO_FS_CREATE || operacion == OP_IO_FS_DELETE || operacion == OP_IO_FS_READ || operacion == OP_IO_FS_TRUNCATE || operacion == OP_IO_FS_WRITE)) {
        return true;
    }

    return false;
}

void atender_io_stdin_read(t_pcb* pcb, char* nombre_interfaz, t_list* direcciones_fisicas, u_int32_t tamanio) {
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.
    
    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if(interfaz == NULL) {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if(!operacion_valida(interfaz, OP_IO_STDIN_READ)) {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d esta en la cola de block de la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->direcciones_fisicas = direcciones_fisicas;
    args->tamanio = tamanio;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void*)manejar_stdin_read, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_stdin_read(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    t_list* direcciones_fisicas = args->direcciones_fisicas;
    uint32_t tamanio = args->tamanio;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_lista_direcciones_a_buffer(buffer, direcciones_fisicas);   
    agregar_uint32_a_buffer(buffer, tamanio);
    t_paquete* paquete = crear_super_paquete(STDIN, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}

void atender_io_stdout_write(t_pcb* pcb, char* nombre_interfaz, t_list* direcciones_fisicas, u_int32_t tamanio) {
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.
    
    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if(interfaz == NULL) {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if(!operacion_valida(interfaz, OP_IO_STDOUT_WRITE)) {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->direcciones_fisicas = direcciones_fisicas;
    args->tamanio = tamanio;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_stdout_write, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_stdout_write(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    t_list *direcciones_fisicas = args->direcciones_fisicas;
    uint32_t tamanio = args->tamanio;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_lista_direcciones_a_buffer(buffer, direcciones_fisicas);   
    agregar_uint32_a_buffer(buffer, tamanio);
    t_paquete* paquete = crear_super_paquete(STDOUT, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}

void atender_io_fs_create(t_pcb *pcb, char *nombre_interfaz, char *nombre_archivo)
{
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.

    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if (interfaz == NULL)
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if (!operacion_valida(interfaz, OP_IO_FS_CREATE))
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->nombre_archivo = nombre_archivo;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_fs_create, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_fs_create(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    char *nombre_archivo = args->nombre_archivo;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_string_a_buffer(buffer, nombre_archivo);
    t_paquete *paquete = crear_super_paquete(FS_CREATE, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}

void atender_io_fs_delete(t_pcb *pcb, char *nombre_interfaz, char *nombre_archivo)
{
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.

    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if (interfaz == NULL)
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if (!operacion_valida(interfaz, OP_IO_FS_CREATE))
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->nombre_archivo = nombre_archivo;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_fs_delete, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_fs_delete(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    char *nombre_archivo = args->nombre_archivo;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_string_a_buffer(buffer, nombre_archivo);
    t_paquete *paquete = crear_super_paquete(FS_DELETE, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}

bool chequear_quantum(t_pcb *pcb)
{
    return (((strcmp(algoritmo_planificacion, "RR") == 0) || (strcmp(algoritmo_planificacion, "VRR") == 0)) && pcb->motivo_exit != FIN_QUANTUM);
}

void atender_io_fs_truncate(t_pcb *pcb, char *nombre_interfaz, char *nombre_archivo, u_int32_t tamanio)
{
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.

    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if (interfaz == NULL)
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if (!operacion_valida(interfaz, OP_IO_FS_CREATE))
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->nombre_archivo = nombre_archivo;
    args->tamanio = tamanio;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_fs_truncate, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_fs_truncate(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    char *nombre_archivo = args->nombre_archivo;
    uint32_t tamanio = args->tamanio;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_string_a_buffer(buffer, nombre_archivo);
    agregar_uint32_a_buffer(buffer, tamanio);
    t_paquete *paquete = crear_super_paquete(FS_TRUNCATE, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}

void atender_io_fs_write(t_pcb *pcb, char *nombre_interfaz, char *nombre_archivo, t_list *lista_direcciones, u_int32_t tamanio, u_int32_t puntero)
{
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.

    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if (interfaz == NULL)
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if (!operacion_valida(interfaz, OP_IO_FS_CREATE))
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->nombre_archivo = nombre_archivo;
    args->direcciones_fisicas = lista_direcciones;
    args->tamanio = tamanio;
    args->puntero = puntero;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_fs_write, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_fs_write(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    // int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    // char *nombre_archivo = args->nombre_archivo;
    // t_list *lista_direcciones = args->direcciones_fisicas;
    // uint32_t tamanio = args->tamanio;
    // uint32_t puntero = args->puntero;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", args->pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, args->pid);
    agregar_string_a_buffer(buffer, args->nombre_archivo);
    agregar_lista_direcciones_a_buffer(buffer, args->direcciones_fisicas);
    agregar_uint32_a_buffer(buffer, args->tamanio);
    agregar_uint32_a_buffer(buffer, args->puntero);
    t_paquete *paquete = crear_super_paquete(FS_WRITE, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}

void atender_io_fs_read(t_pcb *pcb, char *nombre_interfaz, char *nombre_archivo, t_list *lista_direcciones, u_int32_t tamanio, u_int32_t puntero)
{
    t_interfaz_kernel *interfaz = buscar_interfaz(nombre_interfaz);

    // TODO implementar: En el caso de que exista algún proceso haciendo uso de la Interfaz de I/O, el proceso que acaba de solicitar la operación de I/O deberá esperar la finalización del anterior antes de poder hacer uso de la misma.

    //! EN CASO DE QUE LA INTERFAZ NO EXISTA / NO ESTE CONECTADA
    if (interfaz == NULL)
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    // ! EN CASO DE QUE LA INTERFAZ NO ADMITA LA OPERACION
    if (!operacion_valida(interfaz, OP_IO_FS_CREATE))
    {
        pcb->estado = FINISH_ERROR;
        pcb->motivo_exit = INVALID_INTERFACE;
        agregar_pcb(cola_exit, pcb, &mutex_cola_exit);
        sem_post(&sem_exit);
        sem_post(&sem_exec);
        return;
    }

    if ((strcmp(algoritmo_planificacion, "VRR") == 0) && (pcb->motivo_exit != FIN_QUANTUM))
    {
        int tiempo_exec_proceso = temporal_gettime(tiempo_exec);
        pcb->quantum_remanente -= tiempo_exec_proceso;
        temporal_destroy(tiempo_exec);
    }

    cambiar_estado(pcb, BLOCK);
    pcb->motivo_block = IO_BLOCK;
    list_add(interfaz->cola_block_asignada, pcb);
    log_info(kernel_logger, "PID: %d se bloqueo usando la interfaz %s", pcb->pid, interfaz->nombre);
    sem_post(&sem_exec);

    t_manejo_io *args = malloc(sizeof(t_manejo_io));
    args->pid = pcb->pid;
    args->interfaz = interfaz;
    args->nombre_archivo = nombre_archivo;
    args->direcciones_fisicas = lista_direcciones;
    args->tamanio = tamanio;
    args->puntero = puntero;

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void *)manejar_fs_read, (void *)args);
    pthread_detach(hilo_io);
}

void manejar_fs_read(void *parametros)
{
    t_manejo_io *args = (t_manejo_io *)parametros;
    int pid = args->pid;
    t_interfaz_kernel *interfaz = args->interfaz;
    char *nombre_archivo = args->nombre_archivo;
    t_list *lista_direcciones = args->direcciones_fisicas;
    uint32_t tamanio = args->tamanio;
    uint32_t puntero = args->puntero;

    // Esperar a que la interfaz de I/O esté libre
    sem_wait(&interfaz->interfaz_libre);
    log_info(kernel_logger, "PID: %d - Bloqueado por %s", pid, interfaz->nombre);

    // Realizar la operación de I/O
    t_buffer *buffer = crear_buffer();
    agregar_int_a_buffer(buffer, pid);
    agregar_string_a_buffer(buffer, nombre_archivo);
    agregar_lista_direcciones_a_buffer(buffer, lista_direcciones);
    agregar_uint32_a_buffer(buffer, tamanio);
    agregar_uint32_a_buffer(buffer, puntero);
    t_paquete *paquete = crear_super_paquete(FS_READ, buffer);
    enviar_paquete(paquete, interfaz->socket);
    eliminar_paquete(paquete);

    free(args);
}
