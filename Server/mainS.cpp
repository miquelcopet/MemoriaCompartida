#include <string.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <iostream>

#ifdef OS_WINDOWS
    #include <windows.h>
    #include <tchar.h>
    #include <iostream>
    #include <cstdlib>

#else

    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <stdio.h>
    #include <stdio.h>

#endif

using namespace std;

struct Mensaje {
    int id = 0;
    int ack = 0;
    char message[256];
};

string to_String(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

#ifdef OS_WINDOWS

#else
	int crearSegmento (int id_memoria, int key, int BUFFER_SIZE) {
    #ifdef OS_WINDOWS

    #else
        id_memoria = shmget(key, BUFFER_SIZE, 0777 | IPC_CREAT);
        if (id_memoria == 1) {
              cout << "ERROR: Segmento de memoria" << endl;
        }

    #endif
    return id_memoria;
	}

	void * mapearMemoria (void* pBuf, int id_memoria) {
		#ifdef OS_WINDOWS

		#else
			pBuf = shmat (id_memoria, (char *)0, 0);
			if (pBuf == NULL) {
				cout << "ERROR: Reservar la memoria" << endl;
			}
		#endif
		return pBuf;
	}
#endif


int main() {
	
	#ifdef OS_WINDOWS
		string sharedName = "Global\\MyFileMappingObject";
		string message = "";
		HANDLE hMapFile;

		hMapFile = CreateFileMapping(
					 INVALID_HANDLE_VALUE,    // use paging file
					 NULL,                    // default security
					 PAGE_READWRITE,          // read/write access
					 0,                       // maximum object size (high-order DWORD)
					 BUF_SIZE,                // maximum object size (low-order DWORD)
					 sharedName.c_str());     // name of mapping object

		if (hMapFile == NULL)
		{
		cout << "No se ha podido crear el segmento de memoria (" << GetLastError() << ").\n";
		return 1;
		} // end if

		void *pBuf = (void *) MapViewOfFile(hMapFile,    // handle to map object
									FILE_MAP_ALL_ACCESS, // read/write permission
										0,
									0,
										BUF_SIZE);

		if (pBuf == NULL) {
		cout << "No se ha podido completar el mapeo (" << GetLastError() << ").\n";
		CloseHandle(hMapFile);
		return 1;
		} // end if
	#else
	    //Creamos una clave para identificar el espacio de memória
		int key = 7680;
		int BUFFER_SIZE = sizeof(Mensaje) * 2;
		int id_memoria = 0;
		void * pBuf;
		string message="";	
	
		//Creamos el segmento de memória
		id_memoria = crearSegmento(id_memoria, key, BUFFER_SIZE);

		//Se mapea la memoria
		pBuf = mapearMemoria(pBuf, id_memoria);
	#endif


    cout << ".:SERVIDOR:." << endl;
    cout << "Introduzca q y apriete ENTER para salir del programa" << endl;

    Mensaje *messageServer = (Mensaje *) pBuf + 0;
    Mensaje const *messageClient = ((Mensaje *) pBuf) + 1;

    ///Crea el nuevo mensaje con su ID correspondiente
    messageServer->id = messageServer->id + 1;
    message = string("Hola Caracola") + to_String(messageServer->id);
    cout << "Preparando el mensaje numero #" << messageServer->id <<  "." << endl;
    memcpy(&messageServer->message, message.c_str(), strlen(message.c_str()) + 1);

    ///Espera hasta recibir el mensaje
    bool salir = false;
    string choice;
    cin >> choice;
	
	//Sale del bucle si el usuario introduce una "q"
    do {
	if (messageClient->ack == messageServer->id) {
	    cout << "message #" << messageServer->id << " received by client." << endl;

	    ///Prepara el proximo mensaje
	    messageServer->id = messageServer->id + 1;
	    message = string("hola caracola (") + to_string(messageServer->id) + string(").");
	    cout << "Setting message #" << messageServer->id <<  "." << endl;
	    memcpy(&messageServer->message, message.c_str(), strlen(message.c_str()) + 1);
	}

    if (choice == "q") {
        salir=true;
        break;
    }

	this_thread::sleep_for (chrono::seconds(3));

    } while(salir == false);

	#ifdef OS_WINDOWS
		CloseHandle (hMapFile);
	#else
		shmdt((char *)pBuf);
		shmctl (id_memoria, IPC_RMID, (struct shmid_ds *)NULL);
	#endif


    return 0;
}
