#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define N_PARADAS 5 // numero de paradas de la ruta
#define EN_RUTA 0 // autobus en ruta
#define EN_PARADA 1 // autobus en la parada
#define MAX_USUARIOS 40 // capacidad del autobus
#define USUARIOS 4 // numero de usuarios

// Mutex
	pthread_mutex_t _mutexBus;
	pthread_cond_t _condBus;
	pthread_cond_t _subirBus;
	pthread_cond_t _bajarBus;

// estado inicial
int estado = EN_RUTA;
int parada_actual = 0; // parada en la que se encuentra el autobus
int n_ocupantes = 0; // ocupantes que tiene el autobus
// personas que desean subir en cada parada
int esperando_parada[N_PARADAS]; //= {0,0,...0};
// personas que desean bajar en cada parada
int esperando_bajar[N_PARADAS]; //= {0,0,...0};
// Otras definiciones globalesparada en la que se encuentra el autobus
int idUsu = 0;

void Autobus_En_Parada(){
	/* Ajustar el estado y bloquear al autobus hasta que no haya pasajeros que
	quieran bajar y/o subir la parada actual. Despues se pone en marcha */
	pthread_mutex_lock(&_mutexBus);
	estado = EN_PARADA;

	printf("El autobus llega a la parada %d \n", parada_actual);

	while(esperando_parada[parada_actual] > 0 || esperando_bajar[parada_actual] > 0){
		pthread_cond_broadcast(&_bajarBus);
		pthread_cond_broadcast(&_subirBus);
		pthread_cond_wait(&_condBus,  &_mutexBus);
	}
	pthread_mutex_unlock(&_mutexBus);
}

void Conducir_Hasta_Siguiente_Parada(){
	/* Establecer un Retardo que simule el trayecto y actualizar numero de parada */
	int retardo = (rand() % 4) + 1; // Retardo entre 1 y 5
	int aux = 0;
	sleep(retardo);
	pthread_mutex_lock(&_mutexBus);
		printf("El autobus deja la parada. \n");
		estado = EN_RUTA;
		aux  = parada_actual;
		parada_actual = (parada_actual + 1) % N_PARADAS; // 0 1 2 3 4
	pthread_mutex_unlock(&_mutexBus);

	printf ("Conduciendo desde %d hasta %d y me cuesta %d segundos. \n", aux, parada_actual, retardo);
}
void Subir_Autobus(int id_usuario, int origen){
	/* El usuario indicara que quiere subir en la parada ’origen’, esperara a que
	el autobus se pare en dicha parada y subira. El id_usuario puede utilizarse para
	proporcionar informacion de depuracion */
	// El autobus debe estar en la parada
	// Debe haber hueco para poder subir -> No superar MAX_USUARIOS
	pthread_mutex_lock(&_mutexBus);
		esperando_parada[origen] = esperando_parada[origen] + 1;
		while(parada_actual != origen || estado != EN_PARADA){
			pthread_cond_wait(&_subirBus,  &_mutexBus);
		}
		//if(n_ocupantes + 1 < MAX_USUARIOS{ 	// Si hay alguien esperando al bus y tiene sitio para subir, sube
			n_ocupantes = n_ocupantes + 1; 											// Incrementamos los ocupantes del bus
			esperando_parada[parada_actual] = esperando_parada[parada_actual] - 1;	// Decrementa las personas que están esperando
			printf("El usuario %d sube en la parada %d \n", id_usuario, origen);
			//signal del bus
		if(esperando_parada[parada_actual] == 0)
			pthread_cond_broadcast(&_condBus);
	pthread_mutex_unlock(&_mutexBus);
}
void Bajar_Autobus(int id_usuario, int destino){
	/* El usuario indicara que quiere bajar en la parada ’destino’, esperara a que
	el autobus se pare en dicha parada y bajara. El id_usuario puede utilizarse para
	proporcionar informacion de depuracion */
	pthread_mutex_lock(&_mutexBus);
		esperando_bajar[destino] = esperando_bajar[destino] + 1;

	while(parada_actual != destino || estado != EN_PARADA){ // El usuario espera a que el autobús 
		pthread_cond_wait(&_bajarBus, &_mutexBus);
	}

		//if(esperando_bajar[parada_actual] > 0){ 									// Si hay alguien esperando para bajar, baja
			n_ocupantes = n_ocupantes - 1; 											// Decrementamos los ocupantes del bus
			esperando_bajar[parada_actual] = esperando_bajar[parada_actual] - 1;	// Decrementa las personas que quieren bajar
			printf("El usuario %d baja en la parada %d \n", id_usuario, destino);
			//signal del bus
		if(esperando_bajar[parada_actual] == 0)
			pthread_cond_signal(&_condBus);
	pthread_mutex_unlock(&_mutexBus);
}

void Usuario(int id_usuario, int origen, int destino) {
	// Esperar a que el autobus este en parada origen para sBajar_Autobus(id_usuario, destino);
	Subir_Autobus(id_usuario, origen);
	// Bajarme en estacion destino
	
	Bajar_Autobus(id_usuario, destino);
}

void * thread_autobus(void * args) {
	while (1) {
		// esperar a que los viajeros suban y bajen
		Autobus_En_Parada();
		// conducir hasta siguiente parada
		Conducir_Hasta_Siguiente_Parada();
	}
}
void * thread_usuario(void * arg) {
	int id_usuario, a, b;
	// obtener el id del usario
	id_usuario = (int ) arg; // Le asignamos el id del usuario
	while (1) {
		a=rand() % N_PARADAS;
		do{
			b=rand() % N_PARADAS;
		} while(a==b);
		printf("Llega el usuario %d y quiere ir de %d a %d \n", id_usuario, a, b);
		Usuario(id_usuario,a,b);
	}
}

int main(int argc, char *argv[]) {
	int i;
	pthread_t _thUsu[USUARIOS];
	pthread_t _thBus;
	// Definicion de variables locales a main
	// Opcional: obtener de los argumentos del programa la capacidad del
	// autobus, el numero de usuarios y el numero de paradas
	// Crear el thread Autobus
	for (i = 0; i < USUARIOS; i++){
		// Crear thread para el usuario i
		// Esperar terminacion de los hilos
		//printf("ITERACIÓN %d \n", i);
		pthread_create(&_thUsu[i], NULL, thread_usuario, (void *)i);
	}

	pthread_create(&_thBus, NULL, thread_autobus, NULL);
	for(int j = 0; j < USUARIOS; i++){
		pthread_join(_thUsu[i], NULL);
	}
	pthread_join(_thBus, NULL);
	return 0;
}
