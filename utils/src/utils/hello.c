#include <utils/hello.h>

void decir_hola(char *quien)
{
  printf("Hola desde %s!!\n", quien);
}

t_log *iniciar_logger(char *nombreLog, t_log_level level)
{
  t_log *nuevo_logger;
  nuevo_logger = log_create(nombreLog, "LOG", 1, level);
  if (nuevo_logger == NULL)
  {
    perror("No se creo el logger\n");
    exit(1);
  }
  return nuevo_logger;
}

t_config *iniciar_config(char *nombreConfig)
{
  t_config *nuevo_config;
  nuevo_config = config_create(nombreConfig);
  if (nuevo_config == NULL)
  {
    perror("No se pudo crear  el config\n");
    exit(2);
  }
  return nuevo_config;
}

void terminar_programa(t_log *logger, t_config *config)
{

  if (logger != NULL)
  {
    log_destroy(logger);
  }

  if (config != NULL)
  {
    config_destroy(config);
  }
}

/* ------------------------------------------ FUNCIONES DEL CLIENTE -------------------------------------- */

int crear_conexion(char *ip, char *puerto, char *mensaje, t_log *log)
{
  struct addrinfo hints;
  struct addrinfo *server_info;
  int errores = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  errores = getaddrinfo(ip, puerto, &hints, &server_info);

  if (errores < 0)
  {
    fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(errores));
    exit(1);
  }

  int socketDeConexion = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

  if (socketDeConexion == -1)
  {
    perror("Error en socket");
    exit(1);
  }

  errores = connect(socketDeConexion, server_info->ai_addr, server_info->ai_addrlen);

  if (errores < 0)
  {
    fprintf(stderr, "Error en Conexion: %s\n", gai_strerror(errores));
    exit(1);
  }

  log_info(log, "Se conecto con: %s", mensaje);

  freeaddrinfo(server_info);

  return socketDeConexion;
}


void *serializar_paquete(t_paquete *paquete)
{

  int tamanio_stream = paquete->buffer->size + 2*sizeof(int);
  void *stream = malloc(tamanio_stream);
  int desplazamiento = 0;

  memcpy(stream + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
  desplazamiento += sizeof(int);
  memcpy(stream + desplazamiento, &(paquete->buffer->size), sizeof(int));
  desplazamiento += sizeof(int);
  memcpy(stream + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
  desplazamiento += paquete->buffer->size;

  return stream;
}

void enviar_mensaje(char *mensaje, int socket_cliente)
{
  t_paquete *paquete = malloc(sizeof(t_paquete));

  paquete->codigo_operacion = MENSAJE;
  paquete->buffer = malloc(sizeof(t_buffer));
  paquete->buffer->size = strlen(mensaje) + 1;
  paquete->buffer->stream = malloc(paquete->buffer->size);
  memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

  int bytes = paquete->buffer->size + 2 * sizeof(int);

  void *a_enviar = serializar_paquete(paquete);

  send(socket_cliente, a_enviar, bytes, 0);

  free(a_enviar);
  eliminar_paquete(paquete);
}

t_buffer *crear_buffer(void)
{
  t_buffer *buffer = malloc(sizeof(t_buffer));
  buffer->size = 0;
  buffer->stream = NULL;
  return buffer;
}

void destruir_buffer(t_buffer *buffer)
{
  if (buffer->stream != NULL)
  {
    free(buffer->stream);
  }
  free(buffer);
}

void agregar_a_buffer(t_buffer *buffer, void *datos, int tamanio_datos)
{
  // SI EL BUFFER ESTA VACIO, AGREGO AL PRINCIPIO
  if (buffer->size == 0)
  {
    buffer->stream = malloc(sizeof(int) + tamanio_datos);
    memcpy(buffer->stream, &tamanio_datos, sizeof(int));
    memcpy(buffer->stream + sizeof(int), datos, tamanio_datos);
  }
  // SI NO ESTA VACIO, SE AGREGA DESPUES DE LO QUE YA TIENE
  else
  {
    buffer->stream = realloc(buffer->stream, buffer->size + sizeof(int) + tamanio_datos);
    memcpy(buffer->stream + buffer->size, &tamanio_datos, sizeof(int));
    memcpy(buffer->stream + buffer->size + sizeof(int), datos, tamanio_datos);
  }

  buffer->size += sizeof(int);
  buffer->size += tamanio_datos;
}

void agregar_int_a_buffer(t_buffer *buffer, int valor)
{
  agregar_a_buffer(buffer, &valor, sizeof(int));
}

void agregar_uint32_a_buffer(t_buffer *buffer, u_int32_t valor)
{
  agregar_a_buffer(buffer, &valor, sizeof(u_int32_t));
}

void agregar_string_a_buffer(t_buffer *buffer, char *string)
{
  agregar_a_buffer(buffer, string, strlen(string) + 1);
}

t_paquete *crear_paquete(void)
{
  t_paquete *paquete = malloc(sizeof(t_paquete));
  paquete->codigo_operacion = PAQUETE;
  crear_buffer();
  return paquete;
}

t_paquete *crear_super_paquete(op_code cop, t_buffer *buffer){
  t_paquete *paquete = malloc(sizeof(t_paquete));
  paquete->codigo_operacion = cop;
  paquete->buffer = buffer;
  return paquete;
}

    void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
  paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

  memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
  memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

  paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
  int bytes = paquete->buffer->size + 2 * sizeof(int);
  void *a_enviar = serializar_paquete(paquete);

  send(socket_cliente, a_enviar, bytes, 0);

  free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
  free(paquete->buffer->stream);
  free(paquete->buffer);
  free(paquete);
}

/* ------------------------------------- FUNCIONES DEL SERVER ------------------------------------------------ */

int iniciar_escucha(char *PUERTO, char *mensaje, t_log *log)
{

  struct addrinfo hints, *servinfo;
  int errores = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  errores = getaddrinfo(NULL, PUERTO, &hints, &servinfo);

  if (errores < 0)
  {
    fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(errores));
    exit(1);
  }

  int socketEscucha = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  if (socketEscucha == -1)
  {
    perror("Error en socket");
    exit(1);
  }

  errores = bind(socketEscucha, servinfo->ai_addr, servinfo->ai_addrlen);

  if (errores < 0)
  {
    fprintf(stderr, "Error en Bind: %s\n", gai_strerror(errores));
    exit(1);
  }

  errores = listen(socketEscucha, SOMAXCONN);

  if (errores < 0)
  {
    fprintf(stderr, "Error en Listen: %s\n", gai_strerror(errores));
    exit(1);
  }

  log_info(log, "Se puso a escuchar: %s", mensaje);

  freeaddrinfo(servinfo);

  return socketEscucha;
}

int esperar_conexion(int socket_conexion, char *mensaje, t_log *log)
{
  int socket = accept(socket_conexion, NULL, NULL);

  if (socket < 0)
  {
    fprintf(stderr, "Error en Aceptacion: %s\n", strerror(socket));
    exit(1);
  }

  log_info(log, "Se acepto a: %s", mensaje);

  return socket;
}

void handshake_servidor(int socket_conexion)
{
  int32_t handshake;
  int32_t resultOk = 0;
  int32_t resultError = -1;

  recv(socket_conexion, &handshake, sizeof(int32_t), MSG_WAITALL);
  if (handshake == 1)
  {
    send(socket_conexion, &resultOk, sizeof(int32_t), 0);
  }
  else
  {
    send(socket_conexion, &resultError, sizeof(int32_t), 0);
  }
}

void handshake_cliente(int socket_conexion, t_log *log)
{

  int32_t handshake = 1;
  int32_t result;

  send(socket_conexion, &handshake, sizeof(int32_t), 0);
  recv(socket_conexion, &result, sizeof(int32_t), MSG_WAITALL);

  if (result == 0)
  {
    log_info(log, "Handshake OK");
  }
  else
  {
    perror("Error al realizar Handshake");
    exit(1);
  }
}

int recibir_operacion(int socket_cliente)
{
  int cod_op;
  if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
    return cod_op;
  else
  {
    close(socket_cliente);
    return -1;
  }
}

t_buffer *recibir_buffer(int socket_cliente)
{
  t_buffer *buffer = malloc(sizeof(t_buffer));

  if (recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL) > 0)
  {
    buffer->stream = malloc(buffer->size);
    if (recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL) > 0)
    {
      return buffer;
    }
    else
    {
      perror("Error al recibir el void* del buffer del socket cliente");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    perror("Error al recibir el tamanio del buffer del socket_cliente");
    exit(EXIT_FAILURE);
  }
  return buffer;
}

void *extraer_de_buffer(t_buffer *buffer)
{
  if (buffer->size == 0)
  {
    perror("\n Error al intentar extraer el contenido de un t_buffer vacio");
    exit(EXIT_FAILURE);
  }
  if (buffer->size < 0)
  {
    perror("\n Error, el size del t_buffer es negativo");
    exit(EXIT_FAILURE);
  }

  int tamanio_buffer;
  memcpy(&tamanio_buffer, buffer->stream, sizeof(int));
  void *stream = malloc(tamanio_buffer);
  memcpy(stream, buffer->stream + sizeof(int), tamanio_buffer);

  int nuevo_tamanio = buffer->size - sizeof(int) - tamanio_buffer;

  if (nuevo_tamanio == 0)
  {
    buffer->size = 0;
    free(buffer->stream);
    buffer->stream = NULL;
    return stream;
  }

  if (nuevo_tamanio < 0)
  {
    perror("\n Error, buffer con tamanio negativo");
    exit(EXIT_FAILURE);
  }

  void *nuevo_stream = malloc(nuevo_tamanio);
  memcpy(nuevo_stream, buffer->stream + sizeof(int) + tamanio_buffer, nuevo_tamanio);
  free(buffer->stream);
  buffer->size = nuevo_tamanio;
  buffer->stream = nuevo_stream;

  return stream;
}

int extraer_int_de_buffer(t_buffer *buffer)
{
  int *entero = extraer_de_buffer(buffer);
  int valor = *entero;
  free(entero);
  return valor;
}

u_int32_t extraer_uint32_de_buffer(t_buffer *buffer)
{
  u_int32_t *int32 = extraer_de_buffer(buffer);
  u_int32_t valor = *int32;
  free(int32);
  return valor;
}

char *extraer_string_de_buffer(t_buffer *buffer)
{
  char *string = extraer_de_buffer(buffer);
  return string;
}
// void *recibir_buffer(int *size, int socket_cliente)
// {
//   void *buffer;

//   recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
//   buffer = malloc(*size);
//   recv(socket_cliente, buffer, *size, MSG_WAITALL);

//   return buffer;
// }

// void recibir_mensaje(int socket_cliente)
// {
// int size;
// char *buffer = recibir_buffer(&size, socket_cliente);
// TODO: MODIFICAR FUNCION PARA QUE RECIBA UN LOGGER Y ESCRIBA EN ESE MISMO
// log_info(logger, "Me llego el mensaje %s", buffer);
//   free(buffer);
// }

// t_list *recibir_paquete(int socket_cliente)
// {
//   int size;
//   int desplazamiento = 0;
//   void *buffer;
//   t_list *valores = list_create();
//   int tamanio;

//   // buffer = recibir_buffer(&size, socket_cliente);
//   while (desplazamiento < size)
//   {
//     memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
//     desplazamiento += sizeof(int);
//     char *valor = malloc(tamanio);
//     memcpy(valor, buffer + desplazamiento, tamanio);
//     desplazamiento += tamanio;
//     list_add(valores, valor);
//   }
//   free(buffer);
//   return valores;
// }