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

bool parseArg(string &arg, string request, int &pos);
bool loadProcess(string request);
void testString(string s);

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
    int pos = 1;
    // Número de bytes y número de proceso
    string bytes = "";
    string process = "";

    if (!parseArg(bytes, request, pos))
    {
        cout << "El numero de bytes no pudo ser extraido" << endl;
        return false;
    }
    else if (!parseArg(process, request, pos))
    {
        cout << "El numero de proceso no pudo ser extraido" << endl;
        return false;
    }
    else
    {
        int n = stoi(bytes);
        int p = stoi(process);
        cout << n << " " << p << endl;

        return true;
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