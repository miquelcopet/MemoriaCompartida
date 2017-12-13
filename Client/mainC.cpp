#include <string.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <iostream>

#ifdef _WIN32
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

#ifdef _WIN32

#else
	int crearSegmento (int id_memoria, int key, int BUFFER_SIZE) {
    #ifdef _WIN32

    #else
        id_memoria = shmget(key, BUFFER_SIZE, 0777 | IPC_CREAT);
        if (id_memoria == 1) {
              cout << "ERROR: Segmento de memoria" << endl;
        }

    #endif
    return id_memoria;
	}

	void * mapearMemoria (void* pBuf, int id_memoria) {
		#ifdef _WIN32

		#else
			pBuf = shmat (id_memoria, (char *)0, 0);
			if (pBuf == NULL) {
				cout << "ERROR: Reservar la memoria" << endl;
			}
		#endif
		return pBuf;
	}
#endif
int main () {
	
	#ifdef _WIN32
		int BUF_SIZE = sizeof(Message) * 2;

		string sharedName = "Global\\MyFileMappingObject";

		HANDLE hMapFile;

		hMapFile = OpenFileMapping(
					   FILE_MAP_ALL_ACCESS,   // read/write access
					   FALSE,                 // do not inherit the name
					   sharedName.c_str());   // name of mapping object

		if (hMapFile == NULL) {
		cout << "Could not open file mapping object (" << GetLastError() << ").\n";
		return 1;
		} // end if

		char *pBuf = (char *) MapViewOfFile(hMapFile,    // handle to map object
						   FILE_MAP_ALL_ACCESS,  // read/write permission
						   0,
						   0,
						   BUF_SIZE);

		if (pBuf == NULL)
		{
		cout << "Could not map view of file (" << GetLastError() << ").\n";
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

    cout << ".:CLIENTE:." << endl;
	cout << "Introduzca q y apriete ENTER para salir del programa" << endl;

    Mensaje const *messageServer = (Mensaje *) pBuf;
    Mensaje *messageClient = ((Mensaje *) pBuf) + 1;

	bool salir = false;
    string choice;
    cin >> choice;
	
    do {
    ///Recoge el mensaje del servidor
    if (messageServer->id > messageClient->ack) {
        ///Nuevo mensaje en la parte del servidor

        cout << "ID del mensaje recibido: #" << messageServer->id << endl;
        cout << messageServer->message << endl;
        ///Le cambia el ack al mensaje
        messageClient->ack = messageServer->id;
    } else {
        cout << "No hay ningun mensaje nuevo." << endl;
    } // end if

	if (choice == "q") {
        salir=true;
        break;
    }
	
    this_thread::sleep_for (chrono::seconds(1));

    } while (salir == false);

    #ifdef _WIN32
		CloseHandle (hMapFile);
	#else
		shmdt((char *)pBuf);
		shmctl (id_memoria, IPC_RMID, (struct shmid_ds *)NULL);
	#endif

    return 0;
}

