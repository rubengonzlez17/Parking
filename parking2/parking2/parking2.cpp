#include<stdio.h>
#include<stdlib.h>
#include"stdafx.h"
#include<windows.h>
#include"parking2.h"
#define TIME 30000
#define TAM 80
//#include <stdafx.h>
/*
liberacion de recursos
FreeLibrary(biblioteca);
------
llamada
hCoche ->variable que da la biblioteca
int num;
num = P_getNumero(hCoche);
------
tenemos que hacer una funcion de llegada para cada algoritmo, return -2 bloqueo, return -1 encolar,
y valor entero positivo, posicion inicial donde el coche empieza a aparcar-->creamos hilo,
dicho hilo tiene que ejecutar aparcar y por ultimo devolver la posicion

funcion de salida, lo unico que va a hacer es crear un hilo y ese hilo va a ejecutar desaparcar

padre, comprobar parametros,  crea todos los recursos, despues llamar a la funcion de inicio,
luego esperar con sleep(declarar un define), llamar a parking fin, liberar recursos
*/

/*------Funciones auxiliares-----*/
//FUNCIONES DE AJUSTES
int primer_Ajuste(HCoche hc);
int peor_Ajuste(HCoche hc);
int mejor_Ajuste(HCoche hc);
int siguiente_Ajuste(HCoche hc);
//funcion que reserva la acera en el siguiente ajuste
void reserva_acera(int pos, int tam);
//funcion aparcar
void hilo_Aparcar(HCoche hc);
DWORD WINAPI funcionHiloAparcar(LPVOID param);
//funcion desaparcar
void hilo_Desaparcar(HCoche hc);
DWORD WINAPI funcionHiloDesparcar(LPVOID param);
/*-----------------------------------*/
int primer_Ajuste_Salida(HCoche hc);
int peor_Ajuste_Salida(HCoche hc);
int mejor_Ajuste_Salida(HCoche hc);
int siguiente_Ajuste_Salida(HCoche hc);
//FUNCION PARA LA LINEA DE ORDENES
int parametros(int argc, char *argv[]);
int cuenta_Carretera(int ajuste, int pos);
/*-------------------------------*/
//VECTOR DONDE SE ALMACENAN TODOS LOS ALGORITMOS DE AJUSTE
TIPO_FUNCION_LLEGADA funciones_llegada[4] = { primer_Ajuste,siguiente_Ajuste,mejor_Ajuste,peor_Ajuste };
//VECTOR DONDE SE ALMACENAN TODOS LOS ALGORITMOS DE AJUSTE DE SALIDA
TIPO_FUNCION_SALIDA funciones_salida[4] = { primer_Ajuste_Salida,siguiente_Ajuste_Salida,mejor_Ajuste_Salida,peor_Ajuste_Salida };

//Punteros a funciones
typedef struct datos {
	int(*PARKING2_inicio) (TIPO_FUNCION_LLEGADA *, TIPO_FUNCION_SALIDA *, LONG, BOOL);
	void(*PARKING2_aparcar)(HCoche, void *datos, TIPO_FUNCION_APARCAR_COMMIT, TIPO_FUNCION_PERMISO_AVANCE, TIPO_FUNCION_PERMISO_AVANCE_COMMIT);
	int(*PARKING2_desaparcar)(HCoche, void *datos, TIPO_FUNCION_PERMISO_AVANCE, TIPO_FUNCION_PERMISO_AVANCE_COMMIT);
	int(*PARKING2_fin)();
	int(*PARKING2_getNUmero)(HCoche);
	int(*PARKING2_getLongitud)(HCoche);
	int(*PARKING2_getPosiciOnEnAcera)(HCoche);
	unsigned long(*PARKING2_getTServ)(HCoche);
	int(*PARKING2_getColor)(HCoche);
	void(*(*PARKING2_getDatos))(HCoche);
	int(*PARKING2_getX)(HCoche);
	int(*PARKING2_getY)(HCoche);
	int(*PARKING2_getX2)(HCoche);
	int(*PARKING2_getY2)(HCoche);
	int(*PARKING2_getAlgoritmo)(HCoche);
	int(*PARKING2_isAceraOcupada)(int algoritmo, int pos);
	int ultimo;
	int anterior;
	int longitudAnterior;
}datos;
typedef struct memoria {
	char primerAjuste[80];
	char mejorAjuste[80];
	char peorAjuste[80];
	double siguienteAjuste[80];
}memoria;
typedef struct handle {
	HANDLE carreteras[320];
	HANDLE liberar_acera;
	HANDLE salir_acera;
	HANDLE orden;
	HANDLE orden0[3000];
	HANDLE orden1[3000];
	HANDLE orden2[3000];
	HANDLE orden3[3000];
	HANDLE algoritmo[4];
}handle;
memoria mem;
datos m;
handle h;

HCoche hc;

//main
int main(int argc, char* argv[]) {
	m.ultimo = 0;
	//Comprobamos que son correctos el numero de parametros 
	if (0 != parametros(argc, argv)) {
		exit(0);
	}
	//variables
	int i;
	HINSTANCE controladorDll;
	//creamos semaforo

	h.orden = CreateSemaphore(NULL, 1, 1, NULL);
	if (h.orden == NULL) {
		PERROR("ERROR al crear semaforo para orden");
	}


	for (i = 0; i<4; i++) {
		h.algoritmo[i] = CreateSemaphore(NULL, 1, 1, NULL);
		if (h.algoritmo[i] == NULL) {
			PERROR("error al crear semaforo para carretera");
			exit(1);
		}
	}

	h.orden0[0] = CreateSemaphore(NULL, 1, 1, NULL);
	for (i = 1; i<3000; i++) {
		h.orden0[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (h.orden0[i] == NULL) {
			PERROR("error al crear semaforo para carretera");
			exit(1);
		}
	}
	h.orden1[0] = CreateSemaphore(NULL, 1, 1, NULL);
	for (i = 1; i<3000; i++) {
		h.orden1[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (h.orden1[i] == NULL) {
			PERROR("error al crear semaforo para carretera");
			exit(1);
		}
	}
	h.orden2[0] = CreateSemaphore(NULL, 1, 1, NULL);
	for (i = 1; i<3000; i++) {
		h.orden2[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (h.orden2[i] == NULL) {
			PERROR("error al crear semaforo para carretera");
			exit(1);
		}
	}
	h.orden3[0] = CreateSemaphore(NULL, 1, 1, NULL);
	for (i = 1; i<3000; i++) {
		h.orden3[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (h.orden3[i] == NULL) {
			PERROR("error al crear semaforo para carretera");
			exit(1);
		}
	}
	//inicializamos los semaforos para la sincronizacion
	for (i = 0; i<320; i++) {
		h.carreteras[i] = CreateSemaphore(NULL, 1, 1, NULL);//valor inicial 1, valor maximo 1
		if (h.carreteras[i] == NULL) {
			PERROR("error al crear semaforo para carretera");
		}
	}
	//inicializamos el semaforo para liberar
	h.liberar_acera = CreateSemaphore(NULL, 1, 1, NULL);//valor inicial 1, valor maximo 1
	if (h.liberar_acera == NULL) {
		PERROR("error al crear semaforo para orden de entrada");
	}

	h.salir_acera = CreateSemaphore(NULL, 1, 1, NULL);//valor inicial 1, valor maximo 1
	if (h.salir_acera == NULL) {
		PERROR("error al crear semaforo para orden de entrada");
	}
	//cargar biblioteca
	controladorDll = LoadLibrary("parking2.dll");
	//comprobacion de errores 
	if (controladorDll == NULL) {
		PERROR("error LoadLibrary");
	}
	//obtencion de puntero a la funcion*********
	m.PARKING2_inicio = (int(*) (TIPO_FUNCION_LLEGADA *, TIPO_FUNCION_SALIDA *, LONG, BOOL)) GetProcAddress(controladorDll, "PARKING2_inicio");
	if (m.PARKING2_inicio == NULL) {
		PERROR("ERROR getprocaddress inicio");
		return -1;
	}

	m.PARKING2_aparcar = (void(*) (HCoche, void *datos, TIPO_FUNCION_APARCAR_COMMIT, TIPO_FUNCION_PERMISO_AVANCE, TIPO_FUNCION_PERMISO_AVANCE_COMMIT)) GetProcAddress(controladorDll, "PARKING2_aparcar");
	if (m.PARKING2_aparcar == NULL) {
		PERROR("ERROR getprocaddress aparcar");
		return -1;
	}

	m.PARKING2_desaparcar = (int(*) (HCoche, void *datos, TIPO_FUNCION_PERMISO_AVANCE, TIPO_FUNCION_PERMISO_AVANCE_COMMIT)) GetProcAddress(controladorDll, "PARKING2_desaparcar");
	if (m.PARKING2_desaparcar == NULL) {
		PERROR("ERROR getprocaddress desaparcar");
		return -1;
	}

	m.PARKING2_fin = (int(*) ()) GetProcAddress(controladorDll, "PARKING2_fin");
	if (m.PARKING2_fin == NULL) {
		PERROR("ERROR getprocaddress fin");
		return -1;
	}

	m.PARKING2_getNUmero = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getNUmero");
	if (m.PARKING2_getNUmero == NULL) {
		PERROR("ERROR getprocaddress getNUmero");
		return -1;
	}

	m.PARKING2_getLongitud = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getLongitud");
	if (m.PARKING2_getLongitud == NULL) {
		PERROR("ERROR getprocaddress getLongitud");
		return -1;
	}

	m.PARKING2_getPosiciOnEnAcera = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getPosiciOnEnAcera");
	if (m.PARKING2_getPosiciOnEnAcera == NULL) {
		PERROR("ERROR getprocaddress getPosiciOnEnAcera");
		return -1;
	}

	m.PARKING2_getTServ = (unsigned long(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getTServ");
	if (m.PARKING2_getTServ == NULL) {
		PERROR("ERROR getprocaddress getTServ");
		return -1;
	}

	m.PARKING2_getColor = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getColor");
	if (m.PARKING2_getColor == NULL) {
		PERROR("ERROR getprocaddress getColor");
		return -1;
	}

	m.PARKING2_getDatos = (void(**) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getDatos");
	if (m.PARKING2_getDatos == NULL) {
		PERROR("ERROR getprocaddress getDatos");
		return -1;
	}

	m.PARKING2_getX = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getX");
	if (m.PARKING2_getX == NULL) {
		PERROR("ERROR getprocaddress getX");
		return -1;
	}

	m.PARKING2_getY = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getY");
	if (m.PARKING2_getY == NULL) {
		PERROR("ERROR getprocaddress getY");
		return -1;
	}

	m.PARKING2_getX2 = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getX2");
	if (m.PARKING2_getX2 == NULL) {
		PERROR("ERROR getprocaddress getX2");
		return -1;
	}

	m.PARKING2_getY2 = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getY2");
	if (m.PARKING2_getY2 == NULL) {
		PERROR("ERROR getprocaddress getY2");
		return -1;
	}

	m.PARKING2_getAlgoritmo = (int(*) (HCoche)) GetProcAddress(controladorDll, "PARKING2_getAlgoritmo");
	if (m.PARKING2_getAlgoritmo == NULL) {
		PERROR("ERROR getprocaddress getAlgoritmo");
		return -1;
	}

	m.PARKING2_isAceraOcupada = (int(*) (int algoritmo, int pos)) GetProcAddress(controladorDll, "PARKING2_isAceraOcupada");
	if (m.PARKING2_isAceraOcupada == NULL) {
		PERROR("ERROR getprocaddress isAceraOcupada");
		return -1;
	}
	//********************************************************************
	//Llamamos a parking_inicio
	int argumento1 = atoi(argv[1]);
	if (m.PARKING2_inicio(funciones_llegada, funciones_salida, argumento1, 0) == -1) {
		PERROR("ERROR parking2_inicio");
		return -1;
	}
	//El hilo padre se queda esperando durante 30 seg
	Sleep(TIME);
	//liberar recurso, no visto en clase
	//FreeLibrary(controladorDll);
	//ahora llamamos a parking_fin y el programa acaba
	m.PARKING2_fin();
	FreeLibrary(controladorDll);
	return 0;
}

/*ALGORITMOS DE AJUSTE*/
/*Algoritmo del primer ajuste, consiste en colocar el coche en el primer lugar que se encuentre,
siempre y cuando exista espacio suficiente para que el chofer aparque el coche*/
//funcion del primer ajuste
//devolver -1 si no hay sitio para aparcar
//devolver un valor del 0 al 79, que es la posicion inicial para aparcar el coche
//devolver -2, para bloquear el algoritmo
int primer_Ajuste(HCoche hc) {
	int i, j, contador, pos = 0;
	int longitud;
	longitud = m.PARKING2_getLongitud(hc);
	//WaitForSingleObject(h.orden[m.PARKING2_getAlgoritmo(hc)], INFINITE);
	for (i = 0; i < 80; i++) {
		if (mem.primerAjuste[i] == 0) {
			pos = i;
			j = 0;
			contador = 0;
			while (j < longitud && contador != -1) {
				if (mem.primerAjuste[i] == 0) {
					j++;
					i++;
				}
				else {
					contador = -1;;
				}
			}

			if (pos + longitud > 80) {
				return -1;
			}

			if (j >= longitud) {

				for (int k = pos; k < pos + longitud; k++) {
					mem.primerAjuste[k] = 1;
				}
				i = i + 1;

				// WaitForSingleObject(h.orden[m.PARKING2_getAlgoritmo(hc)], INFINITE);
				hilo_Aparcar(hc);
				return pos;
			}
		}
	}
	return -1;
	//return -2;
}

int mejor_Ajuste(HCoche hc) {
	int tam; // Tamaño del coche
			 //char *ocup = disponible + 80 * 2; // Array de disponibilidad
	int i, j; // Iteradores
	int mejPos = -1; // Mejor posicion
	int mejEspacio = 10000; // Mejor espacio

	tam = m.PARKING2_getLongitud(hc);
	//WaitForSingleObject(h.orden[m.PARKING2_getAlgoritmo(hc)], INFINITE);
	// Comprobar cada posición para aparcar
	for (i = 0; i < 80; i++)
	{
		// Si se encuentra un sitio sin ocupar
		if (!(mem.mejorAjuste[i]))
		{
			// Se calcula el espacio del hueco
			for (j = 0; (j + i < 80); j++)
			{
				// Si la posición esta ocupada, se finaliza el calculo
				if (mem.mejorAjuste[i + j]) break;
			}

			// Si se encontró el espacio necesario, se compara con el mejor espacio
			if (j >= tam && i + j <= 80)
			{
				if (mejEspacio > j)
				{
					mejEspacio = j;
					mejPos = i;
				}
			}

			// Se avanza lo comprobado
			i += j;
		}
	}

	// Si se ha encontrado una posición válida, se devuelve la mejor
	if (mejPos != -1)
	{
		for (int k = mejPos; k<mejPos + tam; k++) {
			mem.mejorAjuste[k] = 1;
		}
		//WaitForSingleObject(h.orden[m.PARKING2_getAlgoritmo(hc)], INFINITE);
		hilo_Aparcar(hc);

		return mejPos;
	}
	// Si no, se pone a la cola
	return -1;
	//return -2;
}

int peor_Ajuste(HCoche hc) {
	int tam; // Tamaño del coche
			 //char *ocup = disponible + 80 * 3; // Array de disponibilidad
	int i, j; // Iteradores
	int peorPos = -1; // Peor posicion
	int mejEspacio = -1; // Peor espacio

	tam = m.PARKING2_getLongitud(hc);
	// Comprobar cada posición para aparcar
	for (i = 0; i < 80; i++)
	{
		// Si se encuentra un sitio sin ocupar
		if (!(mem.peorAjuste[i]))
		{
			// Se calcula el espacio del hueco
			for (j = 0; (j + i < 80); j++)
			{
				// Si la posición esta ocupada, se finaliza el calculo
				if (mem.peorAjuste[i + j]) break;
			}

			// Si se encontró el espacio necesario, se compara con el mejor espacio
			if (j >= tam && i + j <= 80)
			{
				if (mejEspacio < j)
				{
					mejEspacio = j;
					peorPos = i;
				}
			}

			// Se avanza lo comprobado
			i += j;
		}
	}

	// Si se ha encontrado una posición válida, se devuelve la mejor
	if (peorPos != -1)
	{
		for (int k = peorPos; k<peorPos + tam; k++) {
			mem.peorAjuste[k] = 1;
		}
		//WaitForSingleObject(h.orden[m.PARKING2_getAlgoritmo(hc)], INFINITE);
		hilo_Aparcar(hc);
		return peorPos;
	}
	// Si no, se pone a la cola
	return -1;
	//return  -2;
}

int siguiente_Ajuste(HCoche hc) {
	int pos = 0;
	int libre = 0;
	int i = 0;
	int j = 0;
	int numero = m.PARKING2_getNUmero(hc);
	int longitud = m.PARKING2_getLongitud(hc);

	WaitForSingleObject(h.orden, INFINITE);
	i = m.ultimo;
	if (mem.siguienteAjuste[m.ultimo] == 0 && m.ultimo>0) {
		while (mem.siguienteAjuste[i] == 0 && i >= 0) {
			m.ultimo = i;
			i--;
		}
	}

	for (i = m.ultimo; i < 80; i++) {
		if (mem.siguienteAjuste[i] == 0) {
			libre++;
		}
		else {
			libre = 0;
		}

		if (libre == longitud) {
			pos = i - longitud + 1;

			for (j = pos; j <= i; j++) {
				mem.siguienteAjuste[j] = 1;
			}

			hilo_Aparcar(hc);
			if (pos >= 0) {
				m.ultimo = pos;
				ReleaseSemaphore(h.orden, 1, NULL);
				return pos;
			}
			else {
				ReleaseSemaphore(h.orden, 1, NULL);
				return -1;
			}
		}
	}

	libre = 0;
	for (i = 0; i < m.ultimo; i++) {

		if (mem.siguienteAjuste[i] == 0) {
			libre++;
		}
		else {
			libre = 0;
		}

		if (libre == longitud) {
			pos = i - longitud + 1;
			for (j = pos; j <= i; j++) {
				mem.siguienteAjuste[j] = 1;
			}

			hilo_Aparcar(hc);
			if (pos >= 0) {
				m.ultimo = pos;
				ReleaseSemaphore(h.orden, 1, NULL);
				return pos;
			}
			else {
				ReleaseSemaphore(h.orden, 1, NULL);
				return -1;
			}
		}
	}
	ReleaseSemaphore(h.orden, 1, NULL);
	return -1;
}

int primer_Ajuste_Salida(HCoche hc) {
	hilo_Desaparcar(hc);
	return 0;
}

int mejor_Ajuste_Salida(HCoche hc) {
	hilo_Desaparcar(hc);
	return 0;
}

int peor_Ajuste_Salida(HCoche hc) {
	hilo_Desaparcar(hc);
	return  0;
}

int siguiente_Ajuste_Salida(HCoche hc) {
	hilo_Desaparcar(hc);
	return 0;
}
void reserva_acera(int pos, int tam) {
	int i;
	for (i = 0; i<tam; i++) {
		mem.siguienteAjuste[i] = 1;
	}
}
//Funciones de salida
int parametros(int argc, char *argv[]) {

	//Variables auxiliares
	int argumento1;
	//Comprobamos que no se introduzcan mas de 3 argumentos y menos de 2, que es el mÃ¡ximo y el mÃnimo permitido teniendo en cuenta que el argumento 0 es el nombre del programa
	if (argc > 3 || argc < 0) {
		PERROR("Numero de argumentos invalido, por favor introduzca minimo 1 argumento y maximo 2 aparte del nombre del programa");
		PERROR("Prueba con './nombre_del_ejecutable argumento1 D'.\n 'D' es opcional: depurar por el canal de errores estandar.\n");
		return -1;
	}
	//ConversiÃ³n a int
	argumento1 = atoi(argv[1]);
	//argumento2 = atoi(argv[2]);
	//Fin de conversiÃ³n

	if (argc == 2) {
		if (argumento1 < 0) {
			PERROR("El primer argumento debe ser un numero entero mayor o igual que 0\n");
			return -2;
		}
	}
	if (argc == 3) {
		if (argumento1 < 0) {
			PERROR("El primer argumento debe ser un numero entero mayor o igual que 0\n");
			return -2;
		}
		if (strcmp("D", argv[2]) != 0) {
			PERROR("El ultimo argumento debe ser una D mayuscula\n");
			return -3;
		}
	}
	//Si todo ha sido realizado correctamente

	return 0;
}
void fCommit(HCoche hc) {
	//SINCRONIZACION PARA EL ORDEN DE ENTRADA
	switch (m.PARKING2_getAlgoritmo(hc)) {
	case 0:
		if ((ReleaseSemaphore(h.orden0[m.PARKING2_getNUmero(hc)], 1, NULL)) == 0) {
			PERROR("Error orden");
			exit(1);
		}
		break;
	case 1:
		if ((ReleaseSemaphore(h.orden1[m.PARKING2_getNUmero(hc)], 1, NULL)) == 0) {
			PERROR("Error orden");
			exit(1);
		}
		break;
	case 2:
		if ((ReleaseSemaphore(h.orden2[m.PARKING2_getNUmero(hc)], 1, NULL)) == 0) {
			PERROR("Error orden");
			exit(1);
		}
		break;
	case 3:
		if ((ReleaseSemaphore(h.orden3[m.PARKING2_getNUmero(hc)], 1, NULL)) == 0) {
			PERROR("Error orden");
			exit(1);
		}
		break;
	default:
		break;
	}//end of switch
}

void fPermisoAvance(HCoche hc) {

	int longitud = m.PARKING2_getLongitud(hc);
	int i, bandera = 0;
	if (m.PARKING2_getY(hc) == m.PARKING2_getY2(hc)) {//nos movemos horizontalmente
		if (m.PARKING2_getX2(hc) < m.PARKING2_getX(hc) && m.PARKING2_getX2(hc) >= 0) {
			WaitForSingleObject(h.carreteras[cuenta_Carretera(m.PARKING2_getAlgoritmo(hc), m.PARKING2_getX2(hc))], INFINITE);//ocupamos la posicion a la que nos dirigimos
		}

	}
	else if (m.PARKING2_getY(hc) == 1 && m.PARKING2_getY2(hc) == 2) {
		if (m.PARKING2_getX(hc) == m.PARKING2_getX2(hc)) {
			WaitForSingleObject(h.carreteras[cuenta_Carretera(m.PARKING2_getAlgoritmo(hc), m.PARKING2_getX(hc) + longitud - 1)], INFINITE);
			for (i = m.PARKING2_getX(hc) + longitud - 2; i >= m.PARKING2_getX(hc); i--) {
				WaitForSingleObject(h.carreteras[cuenta_Carretera(m.PARKING2_getAlgoritmo(hc), i)], INFINITE);
			}
		}
	}
}

void fPermisoAvanceCommit(HCoche hc) {

	int longitud = m.PARKING2_getLongitud(hc);
	int i;
	int bandera = 1;

	if (m.PARKING2_getY(hc) == m.PARKING2_getY2(hc)) {//nos movemos horizontalmente
		if (m.PARKING2_getX(hc) < m.PARKING2_getX2(hc)) {
			if (m.PARKING2_getX(hc) + longitud<80) {
				if ((ReleaseSemaphore(h.carreteras[cuenta_Carretera(m.PARKING2_getAlgoritmo(hc), m.PARKING2_getX(hc)) + longitud], 1, NULL)) == 0) {
					PERROR("Error liberar carretera al avanzar");
					exit(1);
				}
			}
		}
	}
	else if (m.PARKING2_getY(hc) == 1 && m.PARKING2_getY2(hc) == 2) {
		if (m.PARKING2_getX(hc) == m.PARKING2_getX2(hc)) {
			for (i = m.PARKING2_getX(hc); i<m.PARKING2_getX(hc) + longitud; i++) {
				if ((ReleaseSemaphore(h.carreteras[cuenta_Carretera(m.PARKING2_getAlgoritmo(hc), i)], 1, NULL)) == 0) {
					PERROR("Error liberar carretera al aparcar");
					exit(1);
				}
			}
		}
	}
	else if (m.PARKING2_getY(hc) == 2 && m.PARKING2_getY2(hc) == 1) {
		if (m.PARKING2_getX(hc) == m.PARKING2_getX2(hc)) {
			WaitForSingleObject(h.liberar_acera, INFINITE);
			if (m.PARKING2_getAlgoritmo(hc) == 0) {
				for (i = m.PARKING2_getX(hc); i<m.PARKING2_getX(hc) + longitud; i++) {
					mem.primerAjuste[i] = 0;
				}
			}
			else if (m.PARKING2_getAlgoritmo(hc) == 2) {
				for (i = m.PARKING2_getX(hc); i<m.PARKING2_getX(hc) + longitud; i++) {
					mem.mejorAjuste[i] = 0;
				}
			}
			else if (m.PARKING2_getAlgoritmo(hc) == 3) {
				for (i = m.PARKING2_getX(hc); i<m.PARKING2_getX(hc) + longitud; i++) {
					mem.peorAjuste[i] = 0;
				}
			}
			else if (m.PARKING2_getAlgoritmo(hc) == 1) {
				for (i = m.PARKING2_getX(hc); i<m.PARKING2_getX(hc) + longitud; i++) {
					mem.siguienteAjuste[i] = 0;
				}
			}

		}
		if ((ReleaseSemaphore(h.liberar_acera, 1, NULL)) == 0) {
			PERROR("Error liberar carretera al aparcar");
			exit(1);
		}
	}
	//fin de sincronizacion
}
DWORD WINAPI funcionHiloAparcar(LPVOID param) {
	HCoche hc = (HCoche)param;
	switch (m.PARKING2_getAlgoritmo(hc)) {
	case 0:
		WaitForSingleObject(h.orden0[m.PARKING2_getNUmero(hc) - 1], INFINITE);
		m.PARKING2_aparcar(hc, NULL, fCommit, fPermisoAvance, fPermisoAvanceCommit);
		break;
	case 1:
		WaitForSingleObject(h.orden1[m.PARKING2_getNUmero(hc) - 1], INFINITE);
		m.PARKING2_aparcar(hc, NULL, fCommit, fPermisoAvance, fPermisoAvanceCommit);
		break;
	case 2:
		WaitForSingleObject(h.orden2[m.PARKING2_getNUmero(hc) - 1], INFINITE);
		m.PARKING2_aparcar(hc, NULL, fCommit, fPermisoAvance, fPermisoAvanceCommit);
		break;
	case 3:
		WaitForSingleObject(h.orden3[m.PARKING2_getNUmero(hc) - 1], INFINITE);
		m.PARKING2_aparcar(hc, NULL, fCommit, fPermisoAvance, fPermisoAvanceCommit);
		break;
	default:
		break;
	}//end of switch
	return 0;

}
DWORD WINAPI funcionHiloDesaparcar(LPVOID param) {
	HCoche hc = (HCoche)param;
	m.PARKING2_desaparcar(hc, NULL, fPermisoAvance, fPermisoAvanceCommit);

	return 0;
}
void hilo_Aparcar(HCoche hc) {
	HANDLE h;
	h = CreateThread(NULL, 0, funcionHiloAparcar, (LPVOID)hc, 0, NULL);
	if (h == NULL) {
		PERROR("error creando hilo");
	}
}
void hilo_Desaparcar(HCoche hc) {
	HANDLE h;
	h = CreateThread(NULL, 0, funcionHiloDesaparcar, (LPVOID)hc, 0, NULL);
	if (h == NULL) {
		PERROR("error creando hilo");
	}

}
int cuenta_Carretera(int ajuste, int pos) {
	return ((ajuste * 80) + pos);
}

