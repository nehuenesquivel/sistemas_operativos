/*
 * broker.h
 *
 *  Created on: 6 may. 2020
 *      Author: utnso
 */

#ifndef BROKER_H_
#define BROKER_H_
#include<commons/config.h>
#include<commons/collections/queue.h>
#include <conexion.h>
#include <mensajes.h>
#include <math.h>

char* IP;
char* PORT;
uint32_t memory_size;
uint32_t min_partition_size;
char* memory_algorithm;
char* replacement_algorithm;
char* free_partition_algorithm; //rename
int32_t compaction_frequency;

void* mem;

typedef struct
{
	uint32_t ID_mensaje;
	uint32_t ID_correlativo; //para new, catch y get siempre en null //el broker nunca modifica este dato
	t_list* subs_enviados;	//suscriptores a los que ya envie este mensaje
	t_list* subs_confirmados;
	void* datos_mensaje;
	uint32_t bytes;

} t_pending;

typedef struct
{
	uint32_t ID_suscriber;
	queue_code suscribed_queue;
	bool connected;
	int32_t socket;
	t_list* sent_messages; //mensajes que ya le envie a este suscriptor
	t_log* log;

} t_suscriber;

#include "listas.h"

typedef struct
{
	t_list* NEW_POKEMON;			//listas de t_pending* encolados
	t_list* APPEARED_POKEMON;
	t_list* GET_POKEMON;
	t_list* LOCALIZED_POKEMON;
	t_list* CATCH_POKEMON;
	t_list* CAUGHT_POKEMON;

	t_list* NEW_POKEMON_IDS;		//listas de ids encolados
	t_list* APPEARED_POKEMON_IDS;
	t_list* GET_POKEMON_IDS;
	t_list* LOCALIZED_POKEMON_IDS;
	t_list* CATCH_POKEMON_IDS;
	t_list* CAUGHT_POKEMON_IDS;

} t_queues;

typedef struct
{
	t_list* NEW;
	t_list* APPEARED;
	t_list* GET;
	t_list* LOCALIZED;
	t_list* CATCH;
	t_list* CAUGHT;

} t_suscribers;

struct broker_thread_args {
    int32_t socket;
    t_log* logger;
    t_queues* colas;
    t_suscribers* suscriptores;
};

pthread_mutex_t mutex_ID_global;
uint32_t total_new_messages;
uint32_t total_appeared_messages;
uint32_t total_catch_messages;
uint32_t total_caught_messages;
uint32_t total_get_messages;
uint32_t total_localized_messages;

typedef struct
{

	pthread_mutex_t mutex_cola;
	pthread_mutex_t mutex_subs;

	pthread_cond_t broadcast;

} t_semaforos;

typedef struct {
	uint32_t ID_message;
	queue_code queue_code;
	bool available;
	void* final_position;
	void* initial_position;
	uint32_t size;
	int32_t lru;
} t_partition;

t_list* partitions;

pthread_mutex_t mutex_cache;

uint32_t ID_GLOBAL;
uint32_t size_subs_new;			//TODO para mantener la cantidad de suscriptores tambien hay que usar semaforos?
uint32_t size_subs_appeared;
uint32_t size_subs_get;
uint32_t size_subs_localized;
uint32_t size_subs_catch;
uint32_t size_subs_caught;

bool log_debug_console;
t_log* obligatorio;
t_log* logger;
t_log* helper;
t_config* config;

t_queues* queues;
t_suscribers* suscribers;
t_semaforos* semaphores_new;
t_semaforos* semaphores_appeared;
t_semaforos* semaphores_get;
t_semaforos* semaphores_localized;
t_semaforos* semaphores_catch;
t_semaforos* semaphores_caught;


pthread_t listening_thread;

void cola_NEW();
void cola_APPEARED();
void cola_GET();
void cola_LOCALIZED();
void cola_CATCH();
void cola_CAUGHT();

//inicia el servidor del broker con las referencias a colas y semaforos necesarias
void iniciar_servidor_broker();

//atiende clientes en un nuevo hilo con la funcion broker_server_client()
void esperar_clientes(int32_t socket_servidor, t_log* logger, t_queues* colas, t_suscribers* suscriptores);

//verifica si lo que se recibe es una suscripcion o un mensaje, y lo procesa segun corresponda
void broker_serves_client(void* input);


void process_suscripcion(operation_code cod_op, int32_t socket_cliente, t_log* logger, t_suscribers* suscriptores);


void first_process(operation_code cod_op, int32_t socket_cliente, t_log* logger, t_queues* colas);


void send_received_message(t_suscriber* suscriber, t_semaforos* semaforos, t_list* cola, t_list* colaIDs, uint32_t* total_queue_messages);

//mutex funciona sin pasarlo como puntero??
void agregar_Asubs(t_suscriber* suscriber, int32_t socket, queue_code cola, t_list* lista_subs, pthread_mutex_t* mutex, t_log* logger);

//agrega a una cola del broker un t_pending, dada una estructura t_mensaje (new, catch, etc..)
//mutex funciona sin pasarlo como puntero??
void agregar_Acola(t_list* cola, t_list* colaIds, t_pending* t_mensaje, pthread_mutex_t* mutex, t_log* logger, t_semaforos* semaforos, uint32_t* total_queue_messages);

void close_suscriber_thread(t_suscriber* suscriber, t_list* list1, t_list* list2);

//Recibe el size del stream. Recibe un queue_code.
queue_code receive_cola(uint32_t socket, t_log* logger);

t_pending* broker_receive_mensaje(uint32_t socket_cliente, uint32_t* size, bool response, t_log* logger);

void process_receive_message(int32_t socket_cliente, t_log* logger, t_list* queue_NEW, t_list* queueIds, t_semaforos* semaforos, uint32_t* total_queue_messages, bool response, queue_code queue_code);


t_list* obtener_ids_pendientes(t_list* colaEnviados, t_list* colaAEnviar);
bool falta_enviar_msj(t_list* cola_enviados, uint32_t idMensaje);

void initialization();

void generic_initialization();

void specific_initialization();

void initialize_queues();

void config_init();
void semaphores_init();

void behavior();

void listening();

void sending();

void termination();

void specific_termination();

void memory_init();
void create_first_partition(void* memory_initial_position, uint32_t memory_size);
void update_LRU(t_partition* touched_partition);

t_package* broker_serialize(queue_code queue_code, uint32_t id_message, uint32_t id_co, void** message, uint32_t bytes);

void delete_messages_from_queue(t_list* deleted_messages);
void store_message_partition(uint32_t message_id, uint32_t size_message, void* message_data, queue_code queue_code);
void store_message_buddy(uint32_t message_id, uint32_t size_message, void* message_data, queue_code queue_code);
int32_t find_free_position();
void free_some_space();

bool is_free_partition(void *partition);
void free_partition();
uint32_t get_partition_number_to_delete(uint32_t* message_id);

void dump_cache(void);
pthread_mutex_t* get_mutex_and_queues_by_id(queue_code queue_id, t_list** queue, t_list** queueIDS);
void create_dynamic_partition(uint32_t size);
void create_fixed_partition(uint32_t size);
void create_partition(uint32_t size);
void delete_dynamic_partition(t_list** deleted_messages);
void delete_fixed_partition();
void delete_partition();
int32_t __find_available_dynamic_partition(uint32_t size, t_list** deleted_messages);
int32_t find_available_buddy_partition(uint32_t size, t_list** deleted_messages);
int32_t find_available_dynamic_partition(uint32_t size, t_list** deleted_messages);
int32_t get_available_partition_number(uint32_t size);
uint32_t get_available_partition_number_buddy(uint32_t size);
uint32_t get_buddy_partition_size(uint32_t size);
int32_t create_and_search_partition_buddy(uint32_t searched_size, int32_t partition_location);
void delete_buddy_partition(t_list** deleted_messages);
bool is_buddy(int32_t partition_index, int32_t buddy_index);
bool is_free_partition_by_index(int32_t partition_index);
int32_t compact_buddy_to_right(int32_t partition_index);
int32_t compact_buddy_to_left(int32_t partition_index);
int32_t get_size_partition_by_index(int32_t index_partition);
void _memory_compaction();
void memory_compaction();
void merge_partitions(uint32_t initial_partition_number, uint32_t final_partition_number);

#endif /* BROKER_H_ */
