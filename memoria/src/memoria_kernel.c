#include "memoria_kernel.h"

void atender_kernel()
{

    bool control_key = 1;
    while (control_key)
    {  
        t_buffer* un_buffer;
        int cod_op = recibir_operacion(socket_kernel);
        usleep(retardo_respuesta);
        switch (cod_op)
        {
        case MENSAJE:
            //
            break;
        case PAQUETE:
            //
            break;
        case CREAR_PROCESO:
            un_buffer = recibir_buffer(socket_kernel);
            t_proceso *proceso = atender_crear_proceso(un_buffer);
            pthread_mutex_lock(&mutex_lista_procesos);
            list_add(lista_de_procesos, proceso);
            // log_info(mem_logger, "%d,%d", list_size(lista_de_procesos), procesos_necesarios);
            if (cantidad_procesos_creados >= procesos_necesarios && aux_condicion != 0)
            {
                pthread_cond_signal(&condicion);
                aux_condicion = 0;
            }
            pthread_mutex_unlock(&mutex_lista_procesos);
            // int t = list_size(lista_de_procesos);
            /*for (int i = 0; i < t; i++)
            {
                t_proceso *m = list_get(lista_de_procesos, i);
                log_info(mem_logger, "PID: %d", m->pid);
            }*/
            destruir_buffer(un_buffer);
            break;
        case FINALIZAR_PROCESO:
            un_buffer = recibir_buffer(socket_kernel);
            atender_finalizar_proceso(un_buffer);
            destruir_buffer(un_buffer);
            break;
        case -1:
            log_error(mem_logger, "Se desconecto kernel");
            control_key = 0;
            break;
        default:
            log_warning(mem_logger, "Operacion desconocida de kernel");
            close(socket_kernel);
            control_key = 0;
            break;
        }
    }
};