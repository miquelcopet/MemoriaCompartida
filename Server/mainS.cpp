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

int main() {

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

    cout << ".:SERVIDOR:." << endl;
    cout << "Introduzca q y apriete ENTER para salir del programa" << endl;

    Mensaje *messageServer = (Mensaje *) pBuf + 0;
    Mensaje const *messageClient = ((Mensaje *) pBuf) + 1;

    /// Sets new messsage (and message id)
    messageServer->id = messageServer->id + 1;
    message = string("Hola Caracola") + to_String(messageServer->id);
    cout << "Preparando el mensaje numero #" << messageServer->id <<  "." << endl;
    memcpy(&messageServer->message, message.c_str(), strlen(message.c_str()) + 1);

    /// Wait until message is received.
    bool salir = false;
    string choice;
    cin >> choice;
    do {
	if (messageClient->ack == messageServer->id) {
	    cout << "message #" << messageServer->id << " received by client." << endl;

	    /// Sets next message:
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

    //LIbera la memória compartida
    shmdt ((char *)pBuf);
    shmctl (id_memoria, IPC_RMID, (struct shmid_ds *)NULL);

    return 0;
}
