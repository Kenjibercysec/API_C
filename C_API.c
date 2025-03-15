#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 8080          // Port where the server will listen
#define BUFFER_SIZE 1024   // Size of the buffer for receiving/sending data
#define MAX_TASKS 100      // Maximum number of tasks you can create


// Structure to store all the tasks
typedef struct {
    int id;             // Unique identifier for the task
    char description[100]; // Description of the task
    int completed;      // Flag to mark the task as completed
} Task;

// Global array to store all the tasks
Task tasks[MAX_TASKS];
int task_count = 0; // Number of tasks currently stored

// Function to save the log requests in a file
void log_request(const char *request){
    FILE *file = fopen("api_log.txt", "a");
    if(log){
        fprintf(log, "[%s] %s\n", __TIMESTAMP__, request);
        fclose(log);
    }
}



#pragma comment(lib, "ws2_32.lib") // Links the Winsock library
                                // Error flag: This function is not working properly but it doesn't seem to be a critical error, it still works so...

// Function executed by each thread to handle a client connection
DWORD WINAPI handle_client(LPVOID client_socket) {
    SOCKET sock = (SOCKET)client_socket; // Cast the void pointer to a SOCKET
    char buffer[BUFFER_SIZE];            // Buffer to store the incoming request
    char response[BUFFER_SIZE];          // Buffer to store the outgoing response

    // Receive the request from the client
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        printf("Error receiving data: %d\n", WSAGetLastError());
        closesocket(sock); // Close the socket if there's an error
        return 1;
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data
    printf("Request received:\n%s\n", buffer); // Print the request for debugging

    // Route: GET /hello
    if (strncmp(buffer, "GET /hello", 10) == 0) {
        // If the request is a GET to /hello, return a JSON response
        sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"message\": \"Hello from GET /hello!\"}");
    }
    // Route: POST /data 
    // Future fix flag: Still has some work to be done, MUST implement buffer overflow protection ASAP
    else if (strncmp(buffer, "POST /data", 10) == 0) {
        // Find the body of the request (after headers, separated by double newline)
        char *body = strstr(buffer, "\r\n\r\n");
        if (body) {
            body += 4; // Skip the "\r\n\r\n" to reach the actual content
            // Look for a "name" field in the JSON (e.g., {"name": "John"})
            char *name = strstr(body, "\"name\":\"");
            if (name) {
                name += 7; // Skip the "name":" part
                char *name_end = strchr(name, '"'); // Find the end of the value
                if (name_end) {
                    *name_end = '\0'; // Null-terminate the name string
                    // Build a response with the extracted name
                    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"received\": \"Hello, %s!\"}", name);
                } else {
                    // If the JSON is malformed, return a 400 Bad Request
                    sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid JSON");
                }
            } else {
                // If "name" field is not found, return a 400 Bad Request
                sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nField 'name' not found");
            }
        } else {
            // If no body is found, return a 400 Bad Request
            sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nRequest body not found");
        }
    }

    //FEAT log? adding the PUT and DELETE methods
    //Test prompt? PUT http://localhost:8080/data -d "new stuff"
    // or DELETE http://localhost:8080/data
    else if(strncmo(buffer, "PUT /data", 9) == 0) {
        char *body = strstr(buffer, "\r\n\r\n");
        if(body) {
            body += 4;
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"updated\": \"Data updated with %s\"}", body);
        }else {
            sprintf(response, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nBody required");
        }
    } else if(strncmp(buffer, "DELETE /data", 12) == 0){
        spritnf(response, "HTTP/1.1 200 OK\r\n\r\n{\"deleted\": \"Data removed\"}");
    }
    // Unknown route
    else {
        // If no matching route is found, return a 404 Not Found
        sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nWrong route buddy");
    }

    // Send the response back to the client
    send(sock, response, strlen(response), 0);

    // Close the client socket
    closesocket(sock);
    return 0;
}

int main() {
    WSADATA wsa;                        // Structure for Winsock initialization
    SOCKET server_socket, client_socket; // Sockets for server and clients
    struct sockaddr_in server_addr, client_addr; // Server and client address structures
    int addr_len = sizeof(client_addr); // Size of the client address structure

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock: %d\n", WSAGetLastError()); 
        return 1;
    }

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;         // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP // Possible implementatio of a wannabe firewall that blocks some IP addresses, maybe one day if I remember
    server_addr.sin_port = htons(PORT);       // Convert port to network byte order

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections (max 5 in queue)
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server running on port %d...\n", PORT);

    // Main loop to accept client connections
    while (1) {
        // Accept a new client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;
        }

        // Create a thread to handle the client
        HANDLE thread = CreateThread(NULL, 0, handle_client, (LPVOID)client_socket, 0, NULL);
        if (thread == NULL) {
            printf("Failed to create thread: %lu\n", GetLastError());
            closesocket(client_socket); // Close socket if thread creation fails
        } else {
            CloseHandle(thread); // Close the thread handle (we don't wait for it)
        }
    }

    // Cleanup (unreachable in this example due to infinite loop)
    closesocket(server_socket);
    WSACleanup();
    return 0;
}



