/*
 ============================================================================
 Name        : broker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <pthread.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include <conexion.h>
#include <mensajes.h>
#include "routines.c"


#define LOG_CONSOLE true /*pending2*/

#define IP "127.0.0.1" /*pending2*/

/*pending3*/
int main(int argc, char* argv[]) {
	puts("starting gameboy");

	logger = log_create("gameboy.log", "gameboy", LOG_CONSOLE, LOG_LEVEL_INFO);
	log_info(logger, "log created");
	config = config_create("gameboy.config");
	log_info(logger, "config created");

	if (strcmp(argv[1], "SUSCRIPTOR") == 0 && argc == 4) {
		//gameboy SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO]
		queue_code queue_code;
		if (strcmp(argv[2], "APPEARED_POKEMON") == 0) {
			queue_code = COLA_APPEARED;
		} else if (strcmp(argv[2], "CATCH_POKEMON") == 0) {
			queue_code = COLA_CATCH;
		} else if (strcmp(argv[2], "CAUGHT_POKEMON") == 0) {
			queue_code = COLA_CAUGHT;
		} else if (strcmp(argv[2], "GET_POKEMON") == 0) {
			queue_code = COLA_GET;
		} else if (strcmp(argv[2], "LOCALIZED_POKEMON") == 0) {
			queue_code = COLA_LOCALIZED;
		} else if (strcmp(argv[2], "NEW_POKEMON") == 0) {
			queue_code = COLA_NEW;
		} else {
			exit(EXIT_FAILURE);
		}
		subscribe(queue_code, argv[3]);

	} else {

		t_package* package; /*pending1*/
		if (strcmp(argv[1], "BROKER") == 0) {

			ip = config_get_string_value(config, "BROKER_IP");
			port = config_get_string_value(config, "BROKER_PORT");
			if (argc == 3 && strcmp(argv[2], "GET_POKEMON") == 0) {
				//gameboy BROKER GET_POKEMON [POKEMON]

			} else if (argc == 5 && strcmp(argv[2], "CAUGHT_POKEMON") == 0) {
				//gameboy BROKER CAUGHT_POKEMON [ID_CORRELATIVO] [OK/FAIL]

			} else if (argc == 6 && strcmp(argv[2], "CATCH_POKEMON") == 0) {
				//gameboy BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]

			} else if (argc == 7) {

				if (strcmp(argv[2], "APPEARED_POKEMON") == 0) {
					//gameboy BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_CORRELATIVO]

				} else if (strcmp(argv[2], "NEW_POKEMON") == 0) {
					//gameboy BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]
					t_message_new* new;
					new = create_message_new_long(argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
					package = serialize_message_new(new);
					destroy_message_new(new);
				} else {
					exit(EXIT_FAILURE);
				}
			} else {
				exit(EXIT_FAILURE);
			}

		} else if (strcmp(argv[1], "GAMECARD") == 0) {

			ip = config_get_string_value(config, "GAMECARD_IP");
			port = config_get_string_value(config, "GAMECARD_PORT");
			if (argc == 5 && strcmp(argv[2], "GET_POKEMON") == 0) {
				//gameboy GAMECARD GET_POKEMON [POKEMON] [ID_MENSAJE]

			} else if (argc == 7 && strcmp(argv[2], "CATCH_POKEMON") == 0) {
				//gameboy GAMECARD CATCH_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]

			} else if (argc == 8 && strcmp(argv[2], "NEW_POKEMON") == 0) {
				//gameboy GAMECARD NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD] [ID_MENSAJE]

			} else {
				exit(EXIT_FAILURE);
			}

		} else if (strcmp(argv[1], "TEAM") == 0 && argc == 6 && strcmp(argv[2], "APPEARED_POKEMON") == 0) {
			//gameboy TEAM APPEARED_POKEMON [POKEMON] [POSX] [POSY]

			ip = config_get_string_value(config, "TEAM_IP");
			port = config_get_string_value(config, "TEAM_PORT");

		} else {
			exit(EXIT_FAILURE);
		}
		send_message(ip, port, package);

	}

	puts("ending gameboy");
	log_destroy(logger);
	config_destroy(config);
	return EXIT_SUCCESS;
}

/*int32_t posX = atoi(argv[4]);	//convierte a int el argumento de main

	int32_t posY = atoi(argv[5]);
	int32_t cantidad = atoi(argv[6]);

	t_new* new = malloc(sizeof(t_new));
	new->id = 0;
	strcpy(new->nombre, argv[3]);				//valgrind dice "use of uninitialized value of size 4 en strcpy." Pero no aclara si es esta linea
	uint32_t size_name = strlen(new->nombre)+1;
	new->size_nombre = size_name;
	new->posX = posX;				//convertir string a int?
	new->posY = posY;
	new->cantidad = cantidad;
	printf("Se guardaron todos los datos en la estructura a serializar\n");

	//serializar
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete = serialize_new(new); /*pending1

	free(new->nombre);
	free(new);*/
