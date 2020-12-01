/*
 * Administrador de memoria
 * Sistemas operativos
 * Tecnológico de Monterrey
 *
 * Equipo 1
 * A01561977 ITC11 Juan Carlos Gómez
 * A01610267 ITC11 David Alejandro Martínez
 * A01365919 ITC11 Ernesto García
 *
 * Administra una memoria de 2048 bytes con técnica de paginación.
 * Ejecuta una política de reemplazo FIFO con un tamaño de página de 16 bytes y
 * con apoyo de un área de swapping de 4096 bytes.
 * Lee solicitudes de un archivo de texto y carga, accede y libera procesos
 * de un tamaño determinado.
 * Muestra un reporte de métricas al final:
 *  Turnaround time
 *  Turnaround time promedio
 *  Número de page faults por proceso
 *  Número total de operaciones de swapping por proceso
 *
 * Lenguaje utilizado: C++
 * Versión utilizada: C++11
 * Total de líneas: 953
 * Total de líneas sin comentarios: 670
 *
 * Noviembre de 2020
*/
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

#define LOADING_TIME 1.0
#define ACCESING_TIME 0.1
#define FREE_TIME 0.1
#define SWAPPING_TIME 1.0

#define UNASSIGNED_ARGUMENT 0

#define FIFO 1
#define LRU 2

#define PROCESS_INDEX 0
#define PAGE_INDEX 1

#include <cctype>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <queue>

using namespace std;

// Declaración de métodos a usar
void init();
bool validateRequestType(char requestType);
bool loadProcess(string request);
bool accessVirtualAddress(string request);
bool freeProcess(string request);
bool freeProcess(int p);
void comment(string request);
void endSetRequests(string request);
bool parseArgToInt(int &arg, string request, int &pos);
void replaceFIFO(int missingFrames);
void swapOut(int frame);
void swapIn(int process, int page);
int findFreeMP();
void printProcessFrames(int p);
bool processExists(int p);

// Política de reemplazo: seleccionar FIFO o LRU
int replacementPolicy;

// Memoria real M de 2048 bytes, inicializada en false para simular vacía
bool M[MEMORY_SIZE];

// Área de swapping S de 4096 bytes, inicializada en false para simular vacía
bool S[SWAPPING_SIZE];

// Reloj que será actualizado por cada operación
double clock;

// Cada proceso tiene un mapa que indica el número de marco asignado a cada página
// Mapa para almacenar los índices de cada proceso en la memoria real
map<int, map<int, int> > indicesM;

// Mapa para los índices de cada proceso en el área de swapping
map<int, map<int, int> > indicesS;

// Mapa para almacenar el proceso y la página correspondiente a cada marco en memoria real
map<int, vector<int> > indicesFrameProcessM;
// Mapa para almacenar el proceso y la página correspondiente a cada marco en swapping
map<int, vector<int> > indicesFrameProcessS;

// Mapa para el número de page faults por proceso
map<int, int> pageFaultsCount;
// Mapa para los tiempos de inicio y fin de cada proceso
map<int, vector<int>> processTimes;

// Priority queue para registrar los marcos libres en memoria real
priority_queue<int, vector<int>, greater<int> > FreeFramesMqueue;
// Priority queue para registrar los marcos libres en área de swapping
priority_queue<int, vector<int>, greater<int> > FreeFramesSqueue;

// Estructura para verificar si los marcos asignados son continuos o intermitentes
map<int, bool> ContinuosOno;

//Contador para los marcos de página libres en memoria
int FreeFramesMCount = NUMBER_FRAMES_MEMORY;
//Contador para los marcos de página libres en swapping
int FreeFramesSCount = NUMBER_FRAMES_SWAPPING;

// Queue de páginas para usar en FIFO
queue<int> PagesFIFO;
// Queue de páginas para usar en LRU
queue<int> PagesLRU;

// Contador para la cantidad de operaciones de swapping
int SwappingOperationsCount;

int main(void)
{
    // Configuración inicial de memoria, swapping, marcos de página, índices, contadores y tiempos
    init();

    // Archivo de lectura
    ifstream file;
    // Nombre del archivo
    string filename = "ArchivoTrabajo.txt";
    // String para almacenar cada solicitud
    string request;
    // Tipo de solicitud
    char requestType;
    // Bandera de salida
    bool exitFlag = false;

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
            if (validateRequestType(requestType))
            {
                // Imprimir la solicitud
                cout << request << endl;
                // Dar formato correcto a tipo de solicitud
                requestType = toupper(requestType);
                switch (requestType)
                {
                    // Cargar un proceso
                    case LOAD:
                    //cout << "Cargar un proceso: ";
                        loadProcess(request);
                        break;
                    // Acceder a una direccion virtual
                    case ACCESS:
                      //  cout << "Acceder a una direccion virtual: ";
                        accessVirtualAddress(request);
                        break;
                    // Liberar a un proceso
                    case FREE:
                        //cout << "Liberar a un proceso: ";
                        freeProcess(request);
                        break;
                    // Comentario
                    case COMMENT:
                       // cout << "Comentario: " << endl;
                        comment(request);
                        break;
                    // Fin de conjunto de solicitudes
                    case END_SET_REQUESTS:
                       // cout << "Fin de conjunto de solicitudes" << endl;
                        endSetRequests(request);
                        break;
                    case EXIT:
                        exitFlag = true;
                        break;
                    default:
                        break;
                }

                cout << endl;

                if (exitFlag)
                    break;
            }
        }

        cout << "Fin del programa." << endl;
    }
}

void init()
{
    // Definir política de reemplazo
    replacementPolicy = FIFO;

    // Inicializar reloj
    clock = 0.0;

    // Desocupar memoria real
    for(int i = 0; i < MEMORY_SIZE; i++)
    {
        M[i] = false;
    }
    // Desocupar área de swapping
    for(int i = 0; i < SWAPPING_SIZE; i++)
    {
        S[i] = false;
    }
    // Desocupar marcos de página en memoria real
    for(int i = 0; i < NUMBER_FRAMES_MEMORY; i++)
        FreeFramesMqueue.push(i);

    // Desocupar marcos de página en swapping
    for(int i = 0; i < NUMBER_FRAMES_SWAPPING; i++)
        FreeFramesSqueue.push(i);

    // Vaciar índices de marcos de página en memoria real y área de swapping
    indicesM.clear();
    indicesS.clear();

    // Inicializar contador de page faults
    pageFaultsCount.clear();
    // Vaciar tiempos de procesos
    processTimes.clear();

    // Inicializar marcos disponibles en memoria real y área de swappipng
    int FreeFramesMCount = NUMBER_FRAMES_MEMORY;
    int FreeFramesSCount = NUMBER_FRAMES_SWAPPING;

    // Inicializar marco-proceso-página
    indicesFrameProcessM.clear();
    indicesFrameProcessS.clear();

    // Vaciar queues
    while (!PagesFIFO.empty())
    {
        PagesFIFO.pop();
    }

    while (!PagesLRU.empty())
    {
        PagesLRU.pop();
    }

    // Se reinicia el contador de operaciones de swapping
    SwappingOperationsCount = 0;
}

// Función para validar que el input introducido sea una letra
bool validateRequestType(char requestType)
{
    if (isalpha(requestType))
        return true;

    return false;
}

/*
 * Cargar un proceso
 * Extrae el número de bytes y el número de proceso y carga el proceso en memoria real,
 * asignando los marcos de página necesarios.
 *
 * Registra el tiempo de inicio del proceso.
*/
bool loadProcess(string request)
{
    // Número de bytes (n) y número de proceso (p)
    int n = 0;
    int p = 0;

    // Posición inicial en la solicitud para extraer argumento
    int pos = 1;

    // Intentar extraer el número de bytes
    if (!parseArgToInt(n, request, pos))
    {
        cout << "El numero de bytes no pudo ser extraido" << endl;
        return false;
    }
    // Intentar extraer el número de proceso
    else if (!parseArgToInt(p, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    // Éxito: los argumentos fueron extraídos con éxito
    else
    {
        // Imprimir todas las líneas solicitadas por la rúbrica
        // Imprimir solicitud
        cout << request[0] << " " << n << " " << p << endl;
        cout << "Asignar " << n << " bytes al proceso " << p << endl;

        /* Aquí se carga el proceso según sea FIFO o LRU*/

        //Verifica que el tamaño del proceso no exceda las dimensiones de la memoria
        if (n > MEMORY_SIZE || n < 1)
        {
            cout << "El proceso excede las dimensiones de la memoria" << endl;
            return false;
        }

        // Registrar el tiempo de inicio del proceso
        processTimes[p].push_back(clock);

        //Número de marcos de página que se requieren
        int q = n / PAGE_SIZE;
        // Agregar un marco más si hubo bytes insuficientes para completar otra página
        q += (n % PAGE_SIZE != 0) ? 1 : 0;

        // No hay suficientes marcos disponibles para cargar el proceso
        // Liberar marcos necesarios para cargar el proceso
        if (q > FreeFramesMCount)
        {
            // DEBUGGING
            //cout << "Hacen falta " << (q - FreeFramesMCount) << " marcos" << endl;

            // Validar que haya suficiente espacio en área de swapping
            if (FreeFramesSCount >= (q - FreeFramesMCount))
            {
                // Realizar reemplazo según política: FIFO o LRU
                if (replacementPolicy == FIFO)
                {
                    // Realizar swapping con estrategia de FIFO
                    replaceFIFO(q - FreeFramesMCount);
                }
                else if (replacementPolicy == LRU)
                {
                    // Reemplazo con LRU
                    // PENDIENTE
                }
            }
            else
                cout << "Area de swapping llena" << endl;

            return false;
        }

        // Se incializa el mapa de números continuos en verdadero para cada proceso
        ContinuosOno[p] = true;

        //Llena la memoria y los marcos de página correspondientes
        for(int i = 0; i < q; i++)
        {
            // Extrae el primer marco disponible en la memoria
            int MarcoAOcupar = FreeFramesMqueue.top();
            FreeFramesMqueue.pop();

            // Insertar página y marco de página en el índice del proceso actual
            indicesM[p][i] = MarcoAOcupar;


            // Revisa que dos marcos de página que usa un proceso son continuos o no
            if (i > 0)
            {
                if(indicesM[p][i - 1] != indicesM[p][i] - 1 && ContinuosOno[p] == true)
                    ContinuosOno[p] = false;
            }


            // Guardamos proceso y número de página correspondientes al marco actual
            indicesFrameProcessM[MarcoAOcupar].push_back(p);
            indicesFrameProcessM[MarcoAOcupar].push_back(i);

            //cout << endl << "El marco " << MarcoAOcupar << " pertenece al proceso: "<< indicesFrameProcessM[MarcoAOcupar][PROCESS_INDEX] << endl;

            // Actualizar contador de marcos disponibles
            FreeFramesMCount--;

            //cout << "Se ocupo el marco de pagina: " << MarcoAOcupar << endl;    //Esto debe cambiarse, nomas pruebo que sí se ocupen

            // Calcula las direcciones de memoria iniciales y finales del marco
            int LeftLimit = MarcoAOcupar*PAGE_SIZE;
            int RightLimit = LeftLimit+PAGE_SIZE;

            PagesFIFO.push(MarcoAOcupar);

            // Ocupa todas las direcciones de memoria en el rango calculado
            for (int i = LeftLimit; i < RightLimit; i++)
                M[i] = true; //Ocupa la memoria correspondiente al marco

            clock += LOADING_TIME;
        }

        // DEBUGGING: Imprimir los marcos del proceso
        //printProcessFrames(p);

        // Imprimir rango de marcos de página según la rúbrica

        if (ContinuosOno[p] == false)
        {
            cout << "Se asignaron los marcos de pagina [";
            for (int i = 0; i < q; i++)
            {
                //cout << indicesM[p][i] << (i < q - 1) ? ", " : "]";
                if (i < q-1)
                {
                    cout << indicesM[p][i] << ", ";
                }
                else
                {
                    cout << indicesM[p][i] << "]" << endl;
                }

            }
        }
        else
        {
            cout << "Se asignaron los marcos de pagina " << indicesM[p][0] << "-" << indicesM[p][indicesM[p].size() - 1] << " al proceso " << p << endl;
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

bool accessVirtualAddress(string request)
{
    // Virtual address (d), process number (p) and modifier (m)
    int d = 0;
    int p = 0;
    int m = 0;

    // Initial position in request for argument extraction
    int pos = 1;

    // Try parsing virtual address
    if (!parseArgToInt(d, request, pos))
    {
        cout << "La direccion virtual no pudo ser extraida" << endl;
        return false;
    }
    // Try parsing process number
    else if (!parseArgToInt(p, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    // Try parsing modifier
    else if (!parseArgToInt(m, request, pos))
    {
        cout << "El modificador no pudo ser extraido" << endl;
        return false;
    }
    // Success
    else
    {
        if (!processExists(p))
            return false;
        /* Aquí se accede a la memoria virtual del proceso*/
        // Calcular página correspondiente a la dirección de memoria virtual
        int page = d / PAGE_SIZE + 1;

        // Imprimir solicitud
        cout << request << endl;
        if (m == 0)
        {
            cout << "Obtener la direccion real correspondiente a la direccion virtual " << d << " del proceso " << p << endl;
        }
        else
        {
            cout << "Obtener la direccion real correspondiente a la direccion virtual " << d << " del proceso " << p << " y modificar dicha direccion"<< endl;
            cout << "Pagina " << page << " del proceso " << p << " modificada." << endl;
        }

        // Verificar que la página se encuentre en memoria
        if (indicesM[p].find(page) != indicesM[p].end())
        {
            //cout << "La pagina esta en memoria" << endl;
        }
        else
        {
            //cout << "La pagina no esta en memoria" << endl;
            if (replacementPolicy == FIFO)
            {
                if (!PagesFIFO.empty())
                {
                    swapOut(PagesFIFO.front());
                    PagesFIFO.pop();
                }
            }
            else if (replacementPolicy == LRU)
            {
                // PENDIENTE
            }

            swapIn(p, page);
        }

        // Dirección virtual
        int virtualAddress = page * PAGE_SIZE + (d % PAGE_SIZE);
        // Dirección real
        int realAddress = indicesM[p][page] * PAGE_SIZE + (d % PAGE_SIZE);

        cout << "Direccion virtual: " << virtualAddress << ". Direccion real: " << realAddress << endl;

        clock += ACCESING_TIME;

        return true;
    }
}

/*
 * Libera a un proceso
 * Extrae el número de proceso y libera todas sus páginas, en memoria real y en swapping.
 *
 * Registra el tiempo de terminación del proceso.
*/

bool freeProcess(string request)
{
    // Número de proceso
    int p = 0;
    // Initial position in request for argument extraction
    int pos = 1;

    // Try parsing process number
    if (!parseArgToInt(p, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    // Success
    else
    {
        // Imprimir solicitud
        cout << request << endl;
        cout << "Liberar los marcos de pagina ocupados por el proceso " << p << endl;

        /* Aquí se libera el proceso*/
        freeProcess(p);
    }
}

/*
 * Libera a un proceso.
 * Método sobrecargado debido a los procesos que aún no han sido liberados al momento
 * de terminar un conjunto de solicitudes.
*/
bool freeProcess(int p)
{

/*
 * Durante operaciones de swap-in i swap-out las siguientes estructuras son modificadas:
 *
 * S
 * indicesS
 * indicesFrameProcessS
 * FreeFramesSQueue
 * FreeFramesSCount
 *
 * clock
 * pageFaultsCount
 *
*/

    // Imprimir marcos de página liberados
    if(indicesM.find(p) == indicesM.end())
    {
        //cout << "El proceso no fue cargado en memoria" << endl;
        return false;
    }

    if (indicesM[p].size() > 0)
    {
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

            // Liberar rango de direcciones de memoria principal
            // Calcula las direcciones de memoria iniciales y finales del marco
            int LeftLimit = frame*PAGE_SIZE;
            int RightLimit = LeftLimit+PAGE_SIZE;

            // Libera todas las direcciones de memoria en el rango calculado
            for (int i = LeftLimit; i < RightLimit; i++)
                M[i] = false; //Libera la memoria correspondiente al marco

            // Vaciar registros marco-proceso-página
            indicesFrameProcessM[frame].clear();

            // Imprimir lista o fin de lista según sea o no el último marco liberado
            cout << frame << ((next(it, 1) != indicesM[p].end()) ? ", " : "]");

            // Actualiza reloj +0.1 segundos/página
            clock += FREE_TIME;
        }

        cout << endl;

        indicesM[p].clear();
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

        // Liberar rango de direcciones de memoria principal
        // Calcula las direcciones de memoria iniciales y finales del marco
        int LeftLimit = frame*PAGE_SIZE;
        int RightLimit = LeftLimit+PAGE_SIZE;

        // Libera todas las direcciones de memoria en el rango calculado
        for (int i = LeftLimit; i < RightLimit; i++)
            S[i] = false; //Libera la memoria correspondiente al marco

        // Vaciar registros marco-proceso-página
        indicesFrameProcessS[frame].clear();

        // Actualiza reloj +0.1 segundos/página
        clock += FREE_TIME;
    }

    indicesS[p].clear();

    // Registra tiempo final
    processTimes[p].push_back(clock);
    return true;
}

void comment(string request)
{
    // Imprimir solicitud
    cout << request << endl;
}


/*
 * Termina un conjunto de solicitudes
 *
 * Termina los procesos que quedaron en ejecución en el momento actual.
 * Calcula el turnaround time de todos los procesos.
 *
*/
void endSetRequests(string request)
{
    double turnaroundTime;
    double turnaroundTimeMean = 0.0;

    // Imprimir solicitud
    cout << request << endl;
    cout << "Fin. Reporte de salida:" << endl;

    // Calcula el turnaround time de cada proceso
    for (map<int, vector<int>>::iterator it = processTimes.begin(); it != processTimes.end(); it++)
    {
        if (processExists(it->first))
        {
            // Libera el proceso actual si no ha sido liberado
            if (it->second.size() < 2)
                freeProcess(it->first);

            // Realiza el cálculo si el proceso fue liberado
            turnaroundTime = it->second[END_TIME] - it->second[START_TIME];
        }
        else
            turnaroundTime = 0;

        // Imprimir turnaround time por proceso
        cout << "Turnaround time para el proceso " << it->first << ": " << turnaroundTime << endl;

        // Agregar turnaround time al promedio
        turnaroundTimeMean += turnaroundTime;

        // Imprimir número de page faults por proceso
        cout << "Page faults ocurridos para el proceso: "<< it->first << ": " << pageFaultsCount[it->first] << endl;
    }

    // Imprimir turnaround promedio
    turnaroundTimeMean / processTimes.size();
    cout << "Turnaround promedio: " << turnaroundTimeMean << endl;

    // Imprimir número total de operaciones de swapping
    cout << "Numero de operaciones de swapping: " << SwappingOperationsCount << endl;

    init();
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
bool parseArgToInt(int &arg, string request, int &pos)
{
    // Validate function args
    if (pos >= request.length())
    {
        cout << "Invalid start position for argument parsing" << endl;
        return false;
    }

    // Control state to know if prev char was a digit
    bool prevWasDigit = false;

    string str = "";

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
                str += request[pos];
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
            arg = stoi(str);
            return true;
        }
    }

    // The last digit was the last char of the request, no subsequent spaces
    if (pos == request.length() && prevWasDigit)
    {
        //testString(arg);
        arg = stoi(str);
        return true;
    }

    // Search excedes request length and no argument was found
    cout << "No argument was found" << endl;
    return false;
}

void replaceFIFO(int missingFrames)
{
    for (int i = 0; i < missingFrames; i++)
    {
        if (!PagesFIFO.empty())
        {
            int frame = PagesFIFO.front();
            PagesFIFO.pop();
            swapOut(frame);
        }

    }
}

void swapOut(int frame)
{
    SwappingOperationsCount++;
    clock += SWAPPING_TIME;

    int process = indicesFrameProcessM[frame][PROCESS_INDEX];
    int page = indicesFrameProcessM[frame][PAGE_INDEX];

    /////////////////////////// Memoria principal ///////////////////////////
    // Eliminar registro del índice de memoria del proceso correspondiente
    indicesM[process].erase(page);
    // Eliminar registro del índice de marcos de memoria
    indicesFrameProcessM.erase(frame);
    // DEBUGGING
    //cout << "Marco " << frame << " liberado de M" <<endl;

    // Liberar rango de direcciones de memoria principal
    // Calcula las direcciones de memoria iniciales y finales del marco
    int LeftLimit = frame*PAGE_SIZE;
    int RightLimit = LeftLimit+PAGE_SIZE;

    // Libera todas las direcciones de memoria en el rango calculado
    for (int i = LeftLimit; i < RightLimit; i++)
        M[i] = false; //Libera la memoria correspondiente al marco

    // Agrega el marco libre al priority queue
    FreeFramesMqueue.push(frame);
    // Actualiza contador
    FreeFramesMCount++;

    /////////////////////////// Swapping ///////////////////////////
    // Extraemos un marco de swapping
    int frameS = FreeFramesSqueue.top();
    FreeFramesSqueue.pop();

    // Insertar página y marco de página en el índice del proceso actual
    indicesS[process][page] = frameS;

    // Guardamos proceso y número de página correspondientes al marco actual
    indicesFrameProcessS[frameS].push_back(process);
    indicesFrameProcessS[frameS].push_back(page);
    //cout << "Marco " << frameS << " agregado a S" << endl;

    //cout << endl << "El marco " << MarcoAOcupar << " pertenece al proceso: "<< indicesFrameProcessM[MarcoAOcupar][PROCESS_INDEX] << endl;

    // Actualizar contador de marcos disponibles
    FreeFramesSCount--;

    // Calcula las direcciones de memoria iniciales y finales del marco
    int LeftLimitS = frameS*PAGE_SIZE;
    int RightLimitS = LeftLimitS+PAGE_SIZE;

    // Ocupa todas las direcciones de memoria en el rango calculado
    for (int i = LeftLimitS; i < RightLimitS; i++)
        S[i] = true; //Ocupa la memoria correspondiente al marco

    cout << "Se localizo la pagina " << page << " del proceso " << process << " que estaba en la posicion " << frameS << " y se cargo al marco " << frame << endl;
}

void swapIn(int process, int page)
{
    SwappingOperationsCount++;
    clock += SWAPPING_TIME;

    // Actualizar número de page faults del proceso
    pageFaultsCount[process]++;

    /////////////////////////// Swapping ///////////////////////////
    int frame = indicesS[process][page];
    indicesS[process].erase(page);
    indicesFrameProcessS.erase(frame);
    FreeFramesSqueue.push(frame);
    FreeFramesSCount++;
    //cout << "Marco " << frame << " liberado de S" <<endl;

    // Calcula las direcciones de memoria iniciales y finales del marco
    int LeftLimitS = frame*PAGE_SIZE;
    int RightLimitS = LeftLimitS+PAGE_SIZE;

    // Ocupa todas las direcciones de memoria en el rango calculado
    for (int i = LeftLimitS; i < RightLimitS; i++)
        S[i] = false; //Ocupa la memoria correspondiente al marco

    /////////////////////////// Memoria principal ///////////////////////////
    // Extraer el marco de página que se va a utilizar
    int frameM = FreeFramesMqueue.top();
    FreeFramesMqueue.pop();
    // Guardamos proceso y número de página correspondientes al marco actual
    indicesFrameProcessM[frameM].push_back(process);
    indicesFrameProcessM[frameM].push_back(page);

    // Guardamos marco correspondiente al proceso y página
    indicesM[process][page] = frameM;
    // Actualizar contador
    FreeFramesMCount--;

    // Liberar rango de direcciones de memoria principal
    // Calcula las direcciones de memoria iniciales y finales del marco
    int LeftLimitM = frameM*PAGE_SIZE;
    int RightLimitM = LeftLimitM+PAGE_SIZE;

    // Libera todas las direcciones de memoria en el rango calculado
    for (int i = LeftLimitM; i < RightLimitM; i++)
        M[i] = true; //Libera la memoria correspondiente al marco


    cout << "Pagina " << page << " del proceso " << process << " swappeada al marco " << frame << " del area de swapping" <<endl;
    //cout << "Marco " << frameM << " agregado a M" << endl;
}

void testString(string s)
{
    cout << "|" << s << "|" << endl;
}

void printProcessFrames(int p)
{
    cout << "Marcos del proceso " << p << ": ";

    for (int i = 0; i < indicesM[p].size(); i++)
        cout << indicesM[p][i] << " ";

    cout << endl;
}

bool processExists(int p)
{
    if (indicesM.find(p) != indicesM.end() || indicesS.find(p) != indicesM.end())
        return true;

    return false;
}
