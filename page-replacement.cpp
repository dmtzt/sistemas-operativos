#define MEMORY_SIZE 2048
#define SWAPPING_SIZE 4096
#define PAGE_SIZE 16
#define NUMBER_FRAMES_MEMORY 128
#define NUMBER_FRAMES_SWAPPING 256

#define LOAD 'P'
#define ACCESS 'A'
#define FREE 'L'
#define COMMENT 'C'
#define END_SET_REQUESTS 'F'
#define EXIT 'E'

#define START_TIME 0
#define END_TIME 1

#define AVAILABLE false
#define NOT_AVAILABLE true

#define SWAP_TIME 1.0
#define FREE_TIME 0.1

#include <cctype>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <queue>

using namespace std;

void init(bool *M, bool *S, bool *MP);
bool parseArg(string &arg, string request, int &pos);
bool loadProcess(string request, double &clock, map<int, vector<int>> &processTimes);
bool accessVirtualAddress(string request);
bool freeProcess(string request, double &clock, bool *M, bool *S, 
                 map<int, map<int, int> > indicesM, map<int, map<int, int> > indicesS, 
                 map<int, vector<int>> &processTimes);
bool freeProcess(int p, double &clock, bool *M, bool *S, 
                 map<int, map<int, int> > indicesM, map<int, map<int, int> > indicesS, 
                 map<int, vector<int>> &processTimes);
void comment(string request);
void endSetRequests(string request, double &clock,  bool *M, bool *S, 
                 map<int, map<int, int> > indicesM, map<int, map<int, int> > indicesS, 
                 map<int, vector<int>> &processTimes);
//void testString(string s);
int findFreeMP();

void byteToPage();
void pageToByte();

// Memoria real M de 2048 bytes, inicializada en false para simular vacía
bool M[MEMORY_SIZE];

// Área de swapping S de 4096 bytes, inicializada en false para simular vacía
bool S[SWAPPING_SIZE];

bool MP[NUMBER_FRAMES_MEMORY];

//Contador para los marcos de página libres en memoria
int FreeFramesMCount = NUMBER_FRAMES_MEMORY;
//Contador para los marcos de página libres en swapping
int FreeFramesSCount = NUMBER_FRAMES_SWAPPING;

// Priority queue para registrar los marcos libres en memoria real
priority_queue<int, vector<int>, greater<int> > FreeFramesMqueue;
// Priority queue para registrar los marcos libres en área de swapping
priority_queue<int, vector<int>, greater<int> > FreeFramesSqueue;

int main(void)
{
    init(M, S, MP);

    // Cada proceso tiene un mapa que indica el número de marco asignado a cada página
    // Mapa para almacenar los índices de cada proceso en la memoria real
    map<int, map<int, int> > indicesM;
    // Mapa para los índices de cada proceso en el área de swapping
    map<int, map<int, int> > indicesS;

    // Mapa para el número de page faults por proceso
    map<int, int> pageFaultsCount;
    // Mapa para los tiempos de inicio y fin de cada proceso
    map<int, vector<int>> processTimes;

    // Reloj que será actualizado por cada operación
    double clock = 0.0;
    // Archivo de lectura
    ifstream file;
    // Nombre del archivo
    string filename = "ArchivoTrabajo.txt";
    // String para almacenar cada solicitud
    string request;
    // Tipo de solicitud
    char requestType;

    /////////////////////////////////////////////////////////////////////////////////////
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
                // Imprimir la solicitud
                cout << request << endl;
                // Dar formato correcto a tipo de solicitud
                requestType = toupper(requestType);
                switch (requestType)
                {
                    // Cargar un proceso
                    case LOAD:
                    cout << "Cargar un proceso: ";
                        loadProcess(request, clock, processTimes);
                        break;
                    // Acceder a una direccion virtual
                    case ACCESS:
                        cout << "Acceder a una direccion virtual: ";
                        accessVirtualAddress(request);
                        break;
                    // Liberar a un proceso
                    case FREE:
                        cout << "Liberar a un proceso: ";
                        freeProcess(request, clock, M, S, indicesM, indicesS, processTimes);
                        break;
                    // Comentario
                    case COMMENT:
                        cout << "Comentario: " << endl;
                        comment(request);
                        break;
                    // Fin de conjunto de solicitudes
                    case END_SET_REQUESTS:
                        cout << "Fin de conjunto de solicitudes" << endl;
                        endSetRequests(request, clock, M, S, indicesM, indicesS, processTimes);
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

void init(bool *M, bool *S, bool *MP)
{
    for(int i = 0; i < MEMORY_SIZE; i++)
    {
        M[i] = false;
    }
    for(int i = 0; i < SWAPPING_SIZE; i++)
    {
        S[i] = false;
    }
    for(int i = 0; i < NUMBER_FRAMES_MEMORY; i++)
    {
        MP[i] = false;
        FreeFramesMqueue.push(i);
    }
}

/*
 * Cargar un proceso
 * Extrae el número de bytes y el número de proceso y carga el proceso en memoria real,
 * asignando los marcos de página necesarios.
 *
 * Registra el tiempo de inicio del proceso.
*/
bool loadProcess(string request, double &clock, map<int, vector<int>> &processTimes)
{
    // Número de bytes y número de proceso
    string bytes = "", process = "";
    // Posición inicial en la solicitud para extraer argumento
    int pos = 1;

    // Intentar extraer el número de bytes
    if (!parseArg(bytes, request, pos))
    {
        cout << "El numero de bytes no pudo ser extraido" << endl;
        return false;
    }
    // Intentar extraer el número de proceso
    else if (!parseArg(process, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    // Éxito: los argumentos fueron extraídos con éxito
    else
    {
        // Casting de argumentos a números enteros
        int n = stoi(bytes);
        int p = stoi(process);

        // Debugging: valor de n y p
        cout << n << " " << p << endl;

        // Registramos el tiempo de inicio del proceso
        processTimes[p].push_back(clock);

        /* Aquí se carga el proceso según sea FIFO o LRU*/

        if (n > MEMORY_SIZE) //Verifica que el proceso no tenga tamaño mayor a 2048 bytes
        {
            cout << "El proceso no cabe en la memoria" << endl;
            return false;
        }

        //Número de marcos de página que se requieren
        int q = n / PAGE_SIZE;
        // Agregar un marco más si hubo bytes insuficientes para una página completa
        q += (n%PAGE_SIZE != 0) ? 1 : 0;

        if (q > FreeFramesMqueue.size())
        {
            cout <<"Hace falta un swapping" << endl; //Nomas pa ver si si jala
            return false;
        }

        //Llenar la memoria y los marcos de pagina correspondientes
        for(int i = 0; i < q; i++)
        {
            int MarcoAOcupar = FreeFramesMqueue.top();
            MP[MarcoAOcupar] = true; //Ocupa el marco de página
            cout << "Se ocupo el marco de pagina: " << MarcoAOcupar << endl;    //Esto debe cambiarse, nomas pruebo que sí se ocupen
            FreeFramesMqueue.pop();
            int LeftLimit = MarcoAOcupar*PAGE_SIZE;
            int RightLimit = LeftLimit+PAGE_SIZE;
            for (int i = LeftLimit; i < RightLimit; i++)
            {
                M[i] = true; //Ocupa la memoria correspondiente al marco
            }
        }

        return true;
    }
}

/*
 * Accede a una dirección de memoria virtual
 * Extrae la dirección de memoria virtual, el número de proceso y el modificador.
 *
 * Page faults
 *
 * Swapping
*/

bool accessVirtualAddress(string request) {
    // Virtual address, process number and modifier
    string address = "", process = "", modifier = "";
    // Initial position in request for argument extraction
    int pos = 1;

    // Try parsing virtual address
    if (!parseArg(address, request, pos))
    {
        cout << "La dirección virtual no pudo ser extraida" << endl;
        return false;
    }
    // Try parsing process number
    else if (!parseArg(process, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    // Try parsing modifier
    else if (!parseArg(modifier, request, pos))
    {
        cout << "El modificador no pudo ser extraido" << endl;
        return false;
    }
    // Success
    else
    {
        int d = stoi(address);
        int p = stoi(process);
        int m = stoi(modifier);
        cout << d << " " << p << " " << m << endl;

        /* Aquí se accede a la memoria virtual del proceso*/

        return true;
    }
}

/*
 * Libera a un proceso
 * Extrae el número de proceso y libera todas sus páginas, en memoria real y en swapping.
 *
 * Registra el tiempo de terminación del proceso.
*/

bool freeProcess(string request, double &clock, bool *M, bool *S, 
                 map<int, map<int, int> > indicesM, map<int, map<int, int> > indicesS, 
                 map<int, vector<int>> &processTimes)
{
    // Virtual address, process number and modifier
    string process = "";
    // Initial position in request for argument extraction
    int pos = 1;

    // Try parsing process number
    if (!parseArg(process, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    // Success
    else
    {
        int p = stoi(process);
        cout << p << endl;

        /* Aquí se libera el proceso*/
        freeProcess(p, clock, M, S, indicesM, indicesS, processTimes);
        
        
    }
}

/*
 * Libera a un proceso.
 * Método exclusivo para liberar un proceso cuando aún no ha ocurrido al momento
 * de terminar un conjunto de solicitudes.
*/
bool freeProcess(int p, double &clock, bool *M, bool *S, 
                 map<int, map<int, int> > indicesM, map<int, map<int, int> > indicesS, 
                 map<int, vector<int>> &processTimes)
{
// Imprimir marcos de página liberados
    cout << "Se liberan los marcos de página de memoria real: [";
    // Liberar proceso en memoria real
    for (map<int, int>::iterator it = indicesM[p].begin(); it != indicesM[p].end(); it++)
    {
        // Página liberada
        int page = it->first;
        // Marco de página liberado
        int frame = it->second;
        // Agregar frame liberado a priority queue
        FreeFramesMqueue.push(frame);
        // Actualizar el contador de frames libres
        FreeFramesMCount++;
        // Liberar bytes en M - PENDIEMTE
        
        // Actualiza reloj +0.1 segundos/página
        clock += FREE_TIME;
        
        // Imprimir lista o fin de lista según sea o no el último marco liberado
        cout << frame << ((next(it, 1) == indicesM[p].end()) ? ", " : "]");
    }
    
    // Liberar proceso en área de swapping
    for (map<int, int>::iterator it = indicesS[p].begin(); it != indicesS[p].end(); it++)
    {
        // Página liberada
        int page = it->first;
        // Marco de página liberado
        int frame = it->second;
        // Agregar frame liberado a priority queue
        FreeFramesSqueue.push(frame);
        // Actualizar el contador de frames libres
        FreeFramesSCount++;
        // Liberar bytes en S - PENDIEMTE
        
        // Actualiza reloj +0.1 segundos/página
        clock += FREE_TIME;
    }

    // Registra tiempo final
    processTimes[p].push_back(clock);
    return true;
}

void comment(string request)
{

}


/*
 * Termina un conjunto de solicitudes
 *
 * Termina los procesos que quedaron en ejecución en el momento actual.
 * Calcula el turnaround time de todos los procesos.
 *
*/
void endSetRequests(string request, double &clock,  bool *M, bool *S, 
                 map<int, map<int, int> > indicesM, map<int, map<int, int> > indicesS, 
                 map<int, vector<int>> &processTimes)
{
    double turnaroundTime;

    // Calcula el turnaround time de cada proceso
    for (map<int, vector<int>>::iterator it = processTimes.begin(); it != processTimes.end(); it++)
    {
        // Libera el proceso actual si no ha sido liberado
        if (it->second.size() < 2)
            freeProcess(it->first, clock, M, S, indicesM, indicesS, processTimes);

        // Realiza el cálculo si el proceso fue liberado
        turnaroundTime = it->second[END_TIME] - it->second[START_TIME];

        cout << it->first << " " << turnaroundTime << endl;
    }

}

/*
 * Helper method
 * Parse a single argument from request, from the first digit found
 * until a space to the right is found
 *
 * The position variable is updated by reference for future extractions in same request
 *
 * Handles the following cases:
 * 1. Success: The argument is preceded by or followed by several spaces
 * 2. Success: The argument is followed by end of request (null char)
 * 3. Error: The argument is composed by non-digit characters
 * 4. Error: The argument is missing
*/
bool parseArg(string &arg, string request, int &pos)
{
    // Validate function args
    if (pos >= request.length())
    {
        cout << "Invalid start position for argument parsing" << endl;
        return false;
    }

    // Control state to know if prev char was a digit
    bool prevWasDigit = false;

    // Iterate over the request and append digits to request arg as soon as
    // one is found, until they are over and one adjacent space is found
    while (pos < request.length())
    {
        // Find the next digit until an adjacent space is found
        if (prevWasDigit == false || request[pos] != ' ')
        {
            // If current char is a digit
            if (isdigit(request[pos]))
            {
                // Change control state to true
                prevWasDigit = true;
                // Append digit
                arg += request[pos];
            }
            // If a space is found
            else if (request[pos] == ' ')
                // Change control state to false
                prevWasDigit = false;
            // Error: a char not digit or space was found
            else
            {
                cout << "A non-numeric character was found in argument" << endl;
                return false;
            }

            // Update position in request
            pos++;
        }
        // Success: A space after a digit has been found
        else
        {
            //testString(arg);
            return true;
        }
    }

    // The last digit was the last char of the request, no subsequent spaces
    if (pos == request.length() && prevWasDigit)
    {
        //testString(arg);
        return true;
    }

    // Search excedes request length and no argument was found
    cout << "No argument was found" << endl;
    return false;
}

void testString(string s)
{
    cout << "|" << s << "|" << endl;
}

int findFreeMP() //Encuentra la posición del primer marco de página disponible, con el priority queue ya no hará falta
{
    int MPcounter = 0;
    for(int i = 0; i < NUMBER_FRAMES_MEMORY; i++)
    {
        if (MP[i] == true)
        {
            MPcounter++;
        }
        else
        {
            break;
        }
    }
    return MPcounter;
}
