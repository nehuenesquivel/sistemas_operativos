/*
 * suscription.c
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#include "broker.h"

void agregar_Asubs(t_suscriber* suscriber, int32_t socket, queue_code cola, t_list* lista_subs, pthread_mutex_t mutex, t_log* logger){

	suscriber->suscribed_queue = cola;
	suscriber->connected = true;
	suscriber->socket = socket;

	pthread_mutex_lock(&mutex);
		list_add(lista_subs, suscriber);
		pthread_mutex_unlock(&mutex);
	log_info(logger, "Se agrego el socket '%d' a suscriptores\n", socket);

}

void process_suscripcion(operation_code cod_op, int32_t socket_cliente, t_log* logger, t_suscribers* suscriptores) {

	//ya recibi la cod_op
	//recibir el size del stream
	uint32_t size;
	size = receive_size(socket_cliente, logger);

	//recibir el ID del proceso
	uint32_t ID_proceso;
	ID_proceso = receive_ID_proceso(socket_cliente, logger);


	//recibir la cola a suscribirse
	queue_code cola;
	cola = receive_cola(socket_cliente, logger);

//TODO verificar si es un proceso que ya se habia suscrito antes,
	switch(cola){
	case COLA_NEW:
		//buscar sub en la lista de subs de la cola elegida. (un ID de proceso se puede suscribir a todas las colas independientemente)
		//si se encuentra y esta conectado, ERROR. Responder con ack de error
		//si se encuentra, y no conectado, actualizar. (socket y flag)
		//si no se encuentra, camino normal
		break;
	default:
		break;
	}

	//crear el t_suscriber
	t_suscriber* suscriber = malloc(sizeof(t_suscriber));
	suscriber->ID_suscriber = ID_proceso;
	suscriber->sent_messages = list_create();

	//asignar la cola al t_suscriber y guardar en la cola de suscritos
	t_semaforos* my_semaphores;
	t_list* my_queue;
	t_list* my_queueIDs;
	uint32_t* count;
	switch(cola){

	case COLA_NEW:
		my_semaphores = semaphores_new;
		my_queue = queues->NEW_POKEMON;
		my_queueIDs = queues->NEW_POKEMON_IDS;
		count = &total_new_messages;
		log_info(logger, "Por suscribir al socket '%d' a la cola de NEW", socket_cliente);
		agregar_Asubs(suscriber, socket_cliente, cola, suscriptores->NEW, my_semaphores->mutex_subs, logger);
		break;

	case COLA_APPEARED:
		my_semaphores = semaphores_appeared;
		my_queue = queues->APPEARED_POKEMON;
		my_queueIDs = queues->APPEARED_POKEMON_IDS;
		count = &total_appeared_messages;
		log_info(logger, "Por suscribir al socket '%d' a la cola de APPEARED", socket_cliente);
		agregar_Asubs(suscriber, socket_cliente, cola, suscriptores->APPEARED, my_semaphores->mutex_subs, logger);
		break;

	case COLA_CATCH:
		my_semaphores = semaphores_catch;
		my_queue = queues->CATCH_POKEMON;
		my_queueIDs = queues->CATCH_POKEMON_IDS;
		count = &total_catch_messages;
		log_info(logger, "Por suscribir al socket '%d' a la cola de CATCH", socket_cliente);
		agregar_Asubs(suscriber, socket_cliente, cola, suscriptores->CATCH, my_semaphores->mutex_subs, logger);
		break;

	case COLA_CAUGHT:
		my_semaphores = semaphores_caught;
		my_queue = queues->CAUGHT_POKEMON;
		my_queueIDs = queues->CAUGHT_POKEMON_IDS;
		count = &total_caught_messages;
		log_info(logger, "Por suscribir al socket '%d' a la cola de CAUGHT", socket_cliente);
		agregar_Asubs(suscriber, socket_cliente, cola, suscriptores->CAUGHT, my_semaphores->mutex_subs, logger);
		break;

	case COLA_GET:
		my_semaphores = semaphores_get;
		my_queue = queues->GET_POKEMON;
		my_queueIDs = queues->GET_POKEMON_IDS;
		count = &total_get_messages;
		log_info(logger, "Por suscribir al socket '%d' a la cola de GET", socket_cliente);
		agregar_Asubs(suscriber, socket_cliente, cola, suscriptores->GET, my_semaphores->mutex_subs, logger);
		break;

	case COLA_LOCALIZED:
		my_semaphores = semaphores_localized;
		my_queue = queues->LOCALIZED_POKEMON;
		my_queueIDs = queues->LOCALIZED_POKEMON_IDS;
		count = &total_localized_messages;
		log_info(logger, "Por suscribir al socket '%d' a la cola de LOCALIZED", socket_cliente);
		agregar_Asubs(suscriber, socket_cliente, cola, suscriptores->LOCALIZED, my_semaphores->mutex_subs, logger);
		break;

	default:
		log_error(logger, "Aun no puedo suscribir a nadie en ese tipo de cola\n");
		return;
	}

	//responder al proceso que ya esta suscrito
	send_ACK(socket_cliente, logger);


	//crear logger para el suscriptor
	char* nombre = string_new();
	string_append(&nombre, "sub-");
	string_append(&nombre, string_itoa(suscriber->ID_suscriber));
	char* file = string_duplicate(nombre);
	string_append(&nombre, ".log");
	suscriber->log = log_create(nombre, file, LOG_CONSOLE, LOG_LEVEL_TRACE);



	send_received_message(suscriber, my_semaphores, my_queue, my_queueIDs, count);	//loop infinito

}

void send_received_message(t_suscriber* suscriber, t_semaforos* semaforos, t_list* cola, t_list* colaIDs, uint32_t* total_queue_messages){

	t_list* current_global_message_ids = list_create();
	t_list* not_sent_ids = list_create();
	void* elemento;
	uint32_t current_total_count;
	t_pending* mensaje;
	void* message_data;
	uint32_t bytes;
	t_package* paquete;
	int32_t result;
	uint32_t not_sent_size;

	log_debug(suscriber->log, "Empieza el envio de mensajes al proceso: %d", suscriber->ID_suscriber);

	while(1){

		list_clean(current_global_message_ids);
		//--------------Chequeo de mensajes nuevos (siempre se envian mesajes cacheados, la primera vez, y las siguientes)

		log_debug(suscriber->log, "Nuevo chequeo de envio");
		//obtener lista global de ids
		pthread_mutex_lock(&(semaforos->mutex_cola));
			list_add_all(current_global_message_ids, colaIDs);
			current_total_count = (*total_queue_messages);
		pthread_mutex_unlock(&(semaforos->mutex_cola));
		log_debug(suscriber->log, "Hay un total de %d mensajes actualmente en la cola", current_total_count);

		no_enviados_lista(current_global_message_ids, suscriber->sent_messages, &not_sent_ids);
		not_sent_size = list_size(not_sent_ids);
		log_debug(suscriber->log, "Hay %d mensajes pendientes a enviar", not_sent_size);

		//-----------------------------------------------------------------------------------------------------------

		log_debug(suscriber->log, "Entrando al semaforo");
		pthread_mutex_lock(&(semaforos->mutex_cola));
			while (((*total_queue_messages) == current_total_count) && not_sent_size == 0)	//si cumple la condicion, pasa de largo, y sigue ejecutando el programa. Si no cumple, se bloquea.
				pthread_cond_wait(&(semaforos->broadcast), &(semaforos->mutex_cola));
		/* do something that requires holding the mutex and condition is true */
		pthread_mutex_unlock(&(semaforos->mutex_cola));
		log_debug(suscriber->log, "Aparecieron nuevos mensajes a enviar");

		if((*total_queue_messages) < current_total_count)
			printf("ERROR imposible, nunca puede ser menor\n");

		//-----------------------------------------------------------------------------------------------

		while(!list_is_empty(not_sent_ids)){
			//tomar el primer ID de mensaje que falte enviar, y sacarlo de la lista
			elemento = list_remove(not_sent_ids, 0);
			log_debug(suscriber->log, "Se va a enviar el mensaje de ID: %d", (int)elemento);

			//obtener el mensaje con ese ID
	//TODO implementar funcion de busqueda segun algoritmo
			mensaje = find_element_given_ID(elemento, cola, semaforos->mutex_cola, &bytes, &message_data, suscriber->log);
			if(mensaje != NULL){	//en modo sin memoria, siempre va a encontrar el mensaje
				paquete = broker_serialize(suscriber->suscribed_queue, (uint32_t) elemento, &message_data, bytes);

				//enviar el mensaje
				result = send_paquete(suscriber->socket, paquete);

				//si falla el envio, cambiar el flag a desconectado, y cerrar el hilo.
				if(result == -1){
					suscriber->connected = false;
					log_info(suscriber->log, "No se encuentra conectado el suscriptor %d\n", suscriber->ID_suscriber);
					suscriber->socket = 0;
					pthread_exit(NULL);
				}
				else
					log_debug(suscriber->log, "Se envio el mensaje de ID %d al suscriptor %d", (uint32_t) elemento, (uint32_t) suscriber->ID_suscriber);
				//agregar el ID del mensaje como enviado en suscriber->sent_messages
				list_add(suscriber->sent_messages, elemento);

				//Agregar el ID suscriptor en el mensaje de la cola, como que ya fue enviado a este
				pthread_mutex_lock(&(semaforos->mutex_cola));
				list_add(mensaje->subs_enviados, (void*)suscriber->ID_suscriber);
				//verificar si el mensaje ya fue enviado a todos para borrar de la cola. (sigue en la cache)
				pthread_mutex_unlock(&(semaforos->mutex_cola));

				//esperar confirmacion del mensaje
				receive_ACK(suscriber->socket, suscriber->log);

				//Agregar el ID suscriptor en el mensaje de la cola, como que ya fue confirmado para este
				pthread_mutex_lock(&(semaforos->mutex_cola));
				list_add(mensaje->subs_confirmados, (void*)suscriber->ID_suscriber);
				pthread_mutex_unlock(&(semaforos->mutex_cola));

				free(message_data);
			}
			else{
				log_info(suscriber->log, "El mensaje de ID %d ya no se encuentra en la memoria", (uint32_t)elemento);
			}
		}



	}
}

queue_code receive_cola(uint32_t socket, t_log* logger){

	queue_code cola;
	int32_t resultado;
	if((resultado = recv(socket, &cola, sizeof(queue_code), MSG_WAITALL)) == -1){
		log_error(logger, "Error al recibir la cola a suscribirse\n");
		return -1; //failure
	}
	else
		log_info(logger, "Se recibio la cola a suscribirse: %d\n", cola);

	//verificar que sea una cola valida

	return cola;

}

