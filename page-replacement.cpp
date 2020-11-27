#define MEMORY_SIZE 2048
#define SWAPPING_SIZE 4096
#define PAGE_SIZE 16

#define LOAD 'P'
#define ACCESS 'A'
#define FREE 'L'
#define COMMENT 'C'
#define END_SET_REQUESTS 'F'
#define EXIT 'E'


#include <cctype>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace std;

bool loadProcess(string request);

int main(void)
{
    /*
    // Memoria real M de 2048 bytes
    bool M[MEMORY_SIZE];
    // Área de swapping S de 4096 bytes
    bool S[SWAPPING_SIZE];

    // Cada proceso tiene un mapa que indica el número de marco asignado a cada página
    // Mapa para almacenar los índices de cada proceso en la memoria real
    map<int, map<int, int> > indicesM;
    // Mapa para almacenar los índices de cada proceso en el área de swapping
    map<int, map<int, int> > indicesM;
    // Mapa para almacenar el número de page faults por proceso
    map<int, int> pageFaultsCount;

    // Reloj que será actualizado por cada operación
    double clock = 0.0;
    */
    // Archivo de lectura
    ifstream file;
    // Nombre del archivo
    string filename = "ArchivoTrabajo.txt";
    // String para almacenar cada solicitud
    string request;
    // Tipo de solicitud
    char requestType;

    // Abrir archivo
    file.open(filename);

    // Leer el archivo línea por línea si se logró abrir
    if (file.is_open())
    {
        while (getline(file, request))
        {
            // Extraer tipo de solicitud
            requestType = request[0];
            // Verificar que se trate de una letra
            if (isalpha(requestType))
            {
                requestType = toupper(requestType);
                cout << requestType << ": ";
                switch (requestType)
                {
                    case LOAD:
                        cout << "Cargar un proceso" << endl;
                        loadProcess(request);
                        break;
                    case ACCESS:
                        cout << "Acceder a una direccion virtual" << endl;
                        break;
                    case FREE:
                        cout << "Liberar a un proceso" << endl;
                        break;
                    case COMMENT:
                        cout << "Comentario" << endl;
                        break;
                    case END_SET_REQUESTS:
                        cout << "Final de conjunto de solicitudes" << endl;
                        break;
                    case EXIT:
                        cout << "Fin" << endl;
                        break;
                    default:
                        break;
                }
            }
        }
    } 
}

bool loadProcess(string request)
{
    // Número de bytes y número de proceso
    string bytes, process;
    // Control de posición para extracción de datos de solicitud
    int pos = 2;
    
    // Extraer número de bytes
    bytes = request.substr(pos, request.find(' ', pos) - pos);
    // Actualizar posición de apoyo para encontrar el siguiente valor
    pos = request.find(' ', pos) + 1;
    // Extraer número de proceso
    process = request.substr(pos, request.find(' ', pos) - pos); 
    
    // Verificar que los elementos en bytes y proceso sean digitos
    for (char digit : bytes)
    {
        if (!isdigit(digit))
            return false;
    }

    for (char digit : process)
    {
        if (!isdigit(digit))
            return false;
    }
    
    // Número de bytes y número de proceso como enteros
    int n = stoi(bytes);
    int p = stoi(process);;
    
    // Imprimir número de bytes y número de proceso
    cout << n << " " << p << endl;

    return true;
}