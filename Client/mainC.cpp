#include <stdio.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>

#include <thread>
#include <chrono>

using namespace std;

struct Mensaje {
    int id = 0;
    int ack = 0;
    char message[256];
};

int main () {

    int key = 7680;
    int BUFFER_SIZE = sizeof(Mensaje) * 2;

    //Se crea la memoria
    int id_memoria = 0;
    id_memoria = shmget(key, BUFFER_SIZE, 0777 | IPC_CREAT);
    if (id_memoria == 1) {
          fprintf (stderr, "Error con el segmento de memoria");
          return 1;
    }

    //Se de fine el bufeferrerere
    void *pBuf;
       pBuf = shmat (id_memoria, (char *)0, 0);
       if (pBuf == NULL) {
          fprintf (stderr, "Error al reservar al memroria");
          return 1;
       }

    cout << ".:CLIENTE:." << endl;

    Mensaje const *messageServer = (Mensaje *) pBuf;
    Mensaje *messageClient = ((Mensaje *) pBuf) + 1;

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

    this_thread::sleep_for (chrono::seconds(1));

    } while (1);

    shmdt ((char *)pBuf);
    shmctl (id_memoria, IPC_RMID, (struct shmid_ds *)NULL);

    return 0;
}

