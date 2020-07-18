/*
 * messages.c
 *
 *  Created on: 19 jun. 2020
 *      Author: utnso
 */
#include "gamecard.h"
#include <errno.h>


int32_t wait_available_file_new(char* pokemon_name){

	//esperar mutex del pokemon metadata
	pthread_mutex_lock(&mutex_pkmetadata);	//TODO unlock
	//abrir metadata del pokemon dado, si no existe, crearlo, setear metadata en ocupado (soltar el mutex y retornar -1)

	//chequear en el metadata si esta ocupado, en ese caso, soltar el mutex y reintentar en TIEMPO_DE_REINTENTO_OPERACION
	//si esta disponible, actualizarlo como en uso, soltar mutex y listo
}

int32_t wait_available_file(char* pokemon_name){

	//esperar mutex del pokemon metadata
	pthread_mutex_lock(&mutex_pkmetadata);	//TODO unlock
	//abrir metadata del pokemon dado, si no existe, soltar el mutex y retornar directamente que no existe (-1)
	//si no existe, esperar en este punto el tiempo de retardo operacion (mirar si aplica en este caso ese tiempo)
	//chequear en el metadata si esta ocupado, en ese caso, soltar el mutex y reintentar en TIEMPO_DE_REINTENTO_OPERACION
	//si esta disponible, actualizarlo como en uso, soltar mutex y listo, (retornar 0)
}

void release_pokemon_file(char* pokemon_name){
	//esperar mutex del pokemon metadata
	pthread_mutex_lock(&mutex_pkmetadata);
	//abrir metadata del pokemon dado
	//actualizar el metadata del pokemon a libre
	//soltar el mutex
	pthread_mutex_unlock(&mutex_pkmetadata);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

t_message_appeared* process_new(t_message_new* message_new){

	t_message_appeared* message_appeared;
	int32_t exists;

	exists = wait_available_file_new(message_new->pokemon_name);
	//si no existia el archivo metadata, crear directorio y archivo pokemon manualmente
	if(exists != -1){
	//pasar los bloques del archivo a memoria
	//crear diccionario con el archivo
	//verificar si existe en el archivo la posicion recibida, si no existe, crearla
	//sumar la cantidad de pokemonos nuevos
	//convertir el diccionario a void*
	//grabar el void* en los bloques
	//esperar el tiempo de retardo  operacion
	release_pokemon_file(message_new->pokemon_name);
	}
	else{
		//TODO crear metadata y setear en open antes de soltar el mutex
		create_file_directory(message_new->pokemon_name, message_new->location);
		sleep(TIEMPO_RETARDO_OPERACION);
	}

	//generar mensaje appeared y destruir el mensaje new
	return message_appeared;
}

t_message_caught* process_catch(t_message_catch* message_catch){

	t_message_caught* message_caught;
	int32_t exists;
	uint32_t caught_result;

		//si no existia el archivo metadata, saltar directamente a generar la respuesta que no se pudo atrapar

	//esperar mutex del pokemon metadata
	//pthread_mutex_lock(&mutex_pkmetadata);	//TODO unlock
	//aca hay que hacer un fopen y ver que retorna
	char* pokemon_metadata = string_new();
	string_append(&pokemon_metadata, files_directory);
	string_append(&pokemon_metadata, message_catch->pokemon_name);
	string_append(&pokemon_metadata, "/Metadata.bin");
	t_config* file;
	printf("aca estamos bien1\n");
	if((file = config_create(pokemon_metadata)) == NULL){
		log_warning(logger, "no se pudo leer %s/Metadata.bin", message_catch->pokemon_name);
		log_info(logger, "no existe entonces el caught dice que no se puede atrapar");
		pthread_mutex_unlock(&mutex_pkmetadata);
		sleep(TIEMPO_RETARDO_OPERACION); //TODO ver donde va
		//message_caught = generate_caught();
		caught_result =0;
	}
	else{
		char* open;
		//CANTINDAD DE REINTENTOS
		uint32_t retry = 1;
		while(retry){
			printf("aca estamos bien4\n");
			open = config_get_string_value(file, "OPEN");
			if(strcmp(open, "N") == 0){
				//editar el metada.bin -> OPEN=Y
				config_set_value(file, "OPEN", "Y");
				printf("aca estamos bien5\n");
				config_save(file);
				//pthread_mutex_unlock(&mutex_pkmetadata);
				retry = 0;
			}
			else{
				//pthread_mutex_unlock(&mutex_pkmetadata);
				config_destroy(file);
				printf("SLEEP\n");
				sleep(TIEMPO_DE_REINTENTO_OPERACION);
				//pthread_mutex_lock(&mutex_pkmetadata);
			}
		}

		//ARMAR LISTA DE BLOQUES
		char* blocks_string = config_get_string_value(file,"BLOCKS");
		char** blocks_array = string_get_string_as_array(blocks_string);

		//convertir char** en t_list*
		printf("aca estamos bien6\n");
		t_list* blocks_list = list_create();
		t_list* blocks_list_int = list_create();
		uint32_t blocks_count = 0;
		while(blocks_array[blocks_count]!=NULL){
			printf("any block is %s \n",blocks_array[blocks_count]);
			list_add(blocks_list,blocks_array[blocks_count]);
			list_add(blocks_list_int,atoi(blocks_array[blocks_count]));
			blocks_count++;
		}

		printf("cargamos bien los blockes\n");
		//el 24 cambiarlo por lectura de config

		void* pokemon_file = open_file_blocks(blocks_list, 24);
		//DICCIONARIO CON POSITION(KEY)->CANT(VALUE)
		t_dictionary* pokemon_dictionary =  void_to_dictionary(pokemon_file);

		printf("dictionary get en posicion 8-6 value: %s \n",dictionary_get(pokemon_dictionary, "8-6"));
		printf("dictionary get en posicion 5-5 value: %s \n",dictionary_get(pokemon_dictionary, "5-5"));
		printf("dictionary get en posicion 3-2 value: %s \n",dictionary_get(pokemon_dictionary, "3-2"));
		printf("dictionary get en posicion 1-9 value: %s \n",dictionary_get(pokemon_dictionary, "1-9"));
		/*
		DIRECTORY=N
		SIZE=250
		BLOCKS=[40,21,82,3]
		OPEN=Y
		*/
		//1-9=23-2=55-5=28-6=5 -> ESTO PASAR A DICCIONARIO
		char* key = string_new();
		char* x = string_itoa(message_catch->position->x);
		char* y = string_itoa(message_catch->position->y);
		string_append(&key, x);
		string_append(&key, "-");
		string_append(&key, y);
		free(x);
		free(y);
		printf("the key for pokemon map is %s\n",key);
		if(dictionary_has_key(pokemon_dictionary,key)){
			//MODIFICA VALUE EN EL MAPA
			char * value_str = dictionary_get(pokemon_dictionary,key);
			uint32_t value = atoi(value_str);
			if(value>1){
				value--;
				free(value_str);
				char* new_value_str = string_itoa(value);
				dictionary_put(pokemon_dictionary,key,new_value_str);
				//misma cantidad de bloques
			}
			else
			{
				char* new_value_str = dictionary_remove(pokemon_dictionary, key);
				free(new_value_str);
				//aca puede ser  que sea 1 bloque menos o mas
			}
			caught_result = 1;
			//PISA LA LISTA DE BLOQUES DEL FILE SYSTEM CON LOS DATOS DEL DICCIONARIO
			//MAPEA EN DISCO DE NUEVO CON LOS CAMBIOS Y HACE EL CAUGHT TRUE.
			uint32_t new_size;
			void* new_pokemon_file = dictionary_to_void(pokemon_dictionary, &new_size);

			double aux = ((double)new_size/(double)block_size);
			uint32_t new_blocks_count = (uint32_t) ceil(aux);
			printf("a ver el numero%d\n",new_blocks_count);

			//RECORRER LISTA DE BLOQUES Y CREAR ARCHIVOS
			if(new_blocks_count != blocks_count){
				//sacar algun bloque al azar de la lista y actualizar el bitmap y el metadabata.bin
				uint32_t diff = blocks_count - new_blocks_count;
				for(uint32_t c = 0; c< diff; c++){
					uint32_t block_number = list_remove(blocks_list_int, 0);
					//ACTUALIZO BIRMAP
					//ACA VA UN MUTEX NO? TODO
					bitarray_clean_bit(bitmap, block_number);
					msync(bmap, blocks/8, MS_SYNC);
				}
			}
			printf("tamaño de lista %d\n",list_size(blocks_list_int));
			void imprimir(uint32_t elemt){
				printf("elemento de los blocks son %d\n", elemt);
			}
			list_iterate(blocks_list_int,&imprimir);
			if(list_size(blocks_list_int)>0)
				write_file_blocks((void*)new_pokemon_file, blocks_list_int, new_size, message_catch->pokemon_name);


			printf("ACA ESTAMOS FUERA DEL WRITE \n");
			//ACTUALIZAR EL METADATA.BIN DEL POKEMON, BLOCKS_LIST Y SIZE Y OPEN EN N.
			//BLOCKS=[40,21,82,12]
			//SIZE=250
			char* blocks_to_write = string_new();
			uint32_t current_block = 1;
			string_append(&blocks_to_write,"[");

			void list_to_string(uint32_t element){
				char * element_str = string_itoa(element);
				string_append(&blocks_to_write,element_str);
				if(new_blocks_count != current_block)
					string_append(&blocks_to_write,",");
				current_block++;
				free(element_str);
			}
			list_iterate(blocks_list_int,&list_to_string);
			string_append(&blocks_to_write,"]");
			char* new_size_to_metadata = string_itoa(new_size);
			printf("ACA VEMOS SI FUNCIONA %s\n",blocks_to_write);
			config_set_value(file, "BLOCKS", blocks_to_write);
			config_set_value(file, "SIZE", new_size_to_metadata);
			config_set_value(file, "OPEN","N");
			printf("aca estamos bien5\n");
			config_save(file);

			free(new_size_to_metadata);

		}
		else
		{
			caught_result = 0;
			//SI NO LO ENCUENTRA ES QUE NO HAY POKEMON EN ESA POSICION CAUGHT FALSE.
		}
	}




	//generar mensaje caught y destruir el mensaje catch

	//Generar mensaje CAUGHT
	message_caught = create_message_caught(message_catch->id, caught_result);
	log_info(logger, "Se genero el mensaje caught");
	destroy_message_catch(message_catch);


	return message_caught;
}

t_message_localized* process_get(t_message_get* message_get){

	t_message_localized* message_localized;
	int32_t exists;

	exists = wait_available_file(message_get->pokemon_name);
		//si no existia el archivo metadata, saltar directamente a generar la respuesta localized sin posiciones
	if(exists != -1){
	//pasar los bloques del archivo a memoria
	//crear diccionario con el archivo
	//Obtener todas las posiciones y cantidades de Pokemon pedido.
	//no se modifica el archivo, entonces con destruir el diccionario alcanza
	//esperar el tiempo de retardo operacion
	release_pokemon_file(message_get->pokemon_name);
	}

	//generar mensaje localized y destruir el mensaje get
	return message_localized;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void serve_new(void* input){
	operation_code op_code = ((serve_thread_args*)input)->op_code;
	void* message = ((serve_thread_args*)input)->message;
	free(input);

	if(op_code != OPERATION_NEW)
		log_error(logger, "Aca nunca llego");

	t_message_new* message_new = (t_message_new*) message;

	t_message_appeared* message_appeared;
	//message_appeared =  process_new(message_new); TODO

//-----------------------------------------------------//TODO remover harcodeo, se hace esto en la funcion de arriba
	//Generar mensaje APPEARED
	message_appeared = create_message_appeared_long(message_new->id, message_new->pokemon_name, message_new->location->position->x, message_new->location->position->y);
	log_info(logger, "Se genero el mensaje appeared");
	destroy_message_new(message_new);

//------------------------------------------------------

	t_package* package = serialize_appeared(message_appeared);
	destroy_message_appeared(message_appeared);


	send_to_broker(package);
}

void serve_catch(void* input){
	operation_code op_code = ((serve_thread_args*)input)->op_code;
	void* message = ((serve_thread_args*)input)->message;
	free(input);

	if(op_code != OPERATION_CATCH)
		log_error(logger, "Aca nunca llego");

	t_message_catch* message_catch = (t_message_catch*) message;

	t_message_caught* message_caught;
	message_caught = process_catch(message_catch);



	t_package* package = serialize_caught(message_caught);
	destroy_message_caught(message_caught);


	send_to_broker(package);
}

void serve_get(void* input){
	operation_code op_code = ((serve_thread_args*)input)->op_code;
	void* message = ((serve_thread_args*)input)->message;
	free(input);

	if(op_code != OPERATION_GET)
		log_error(logger, "Aca nunca llego");

	t_message_get* message_get = (t_message_get*) message;

	t_message_localized* message_localized;
	//message_localized = process_get(message_get); TODO

//-----------------------------------------------------//TODO remover harcodeo, se hace esto en la funcion de arriba
	//Generar mensaje LOCALIZED
			t_position* positions = malloc(3 * sizeof(t_position));
			positions->x = 1;
			positions->y = 1;
			(positions + 1)->x = 2;
			(positions + 1)->y = 2;
			(positions + 2)->x = 3;
			(positions + 2)->y = 3;

	message_localized = create_message_localized(message_get->id, message_get->pokemon_name, 3, positions);
	log_info(logger, "Se genero el mensaje localized");
	destroy_message_get(message_get);

//-----------------------------------------------------

	t_package* package = serialize_localized(message_localized);
	destroy_message_localized(message_localized);


	send_to_broker(package);
}


//---------------------------------------------No va------------------------------------------------------------------------------------------------------------------



t_dictionary* semaphores;
pthread_mutex_t semaforo_del_diccionario_de_semaforos_JAJAJA;
//un semaforo para cada pokemon_metadata. En el caso que rompa t0do, se puede usar "sin problema" un unico mutex global para todos los pokemon_metadata
pthread_mutex_t* get_pokemon_mutex(char* pokemon_name){
	pthread_mutex_lock(&semaforo_del_diccionario_de_semaforos_JAJAJA);
	if(dictionary_has_key(semaphores, pokemon_name)){
		pthread_mutex_t* found_semaphore = dictionary_get(semaphores, pokemon_name);
		pthread_mutex_unlock(&semaforo_del_diccionario_de_semaforos_JAJAJA);
		return found_semaphore;
	}
	else{
		pthread_mutex_t new_semaphore;
		pthread_mutex_init(&new_semaphore, NULL);
		dictionary_put(semaphores, pokemon_name, &new_semaphore);
		pthread_mutex_unlock(&semaforo_del_diccionario_de_semaforos_JAJAJA);
		return &new_semaphore;
	}
}
