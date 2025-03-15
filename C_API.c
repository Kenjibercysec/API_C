#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 8080
#define BUFFER_SIZE 1024 // Buffer size for reading data. 1024 bytes should be enough for anyone, right?
#define MAX_TASKS 100  // Maximum number of tasks.

// Task structure:
typedef struct {
    int id;
    char description[100];
    int completed;
} Task;

Task tasks[MAX_TASKS];
int task_count = 0;

// Function to save tasks to a file.
void save_tasks() {
    FILE *file = fopen("c:\\Users\\silas\\OneDrive\\Desktop\\API_C\\tasks.txt", "w");
    if (file) {
        printf("Saving tasks to tasks.txt...\n");
        for (int i = 0; i < task_count; i++) {
            fprintf(file, "%d,%s,%d\n", tasks[i].id, tasks[i].description, tasks[i].completed);
            printf("Saved task: %d,%s,%d\n", tasks[i].id, tasks[i].description, tasks[i].completed);
        }
        fclose(file);
        printf("Tasks saved successfully.\n");
    } else {
        printf("Failed to open tasks.txt for writing: %lu\n", GetLastError());
    }
}

// Function to load tasks from a file.
void load_tasks() {
    FILE *file = fopen("c:\\Users\\silas\\OneDrive\\Desktop\\API_C\\tasks.txt", "r");
    if (file) {
        task_count = 0;
        while (task_count < MAX_TASKS && fscanf(file, "%d,%99[^,],%d\n", 
                &tasks[task_count].id, tasks[task_count].description, &tasks[task_count].completed) == 3) {
            task_count++;
        }
        fclose(file);
        printf("Tasks loaded successfully: %d tasks\n", task_count);
    } else {
        printf("No tasks.txt found or failed to open: %lu\n", GetLastError());
    }
}

// Function to log requests to a file.
void log_request(const char *request) {
    FILE *file = fopen("c:\\Users\\silas\\OneDrive\\Desktop\\API_C\\api_log.txt", "a");
    if (file) {
        fprintf(file, "[%s] %s\n", __TIMESTAMP__, request);
        fclose(file);
    }
}

// Function to check if the request is authorized.
int is_authorized(const char *buffer) {
    const char *auth = strstr(buffer, "Authorization: Basic ");
    if (auth) {
        auth += 21;
        return strncmp(auth, "dXNlcjpwYXNz", 12) == 0;
    }
    return 0;
}

// Function to handle client requests.
DWORD WINAPI handle_client(LPVOID client_socket) {
    SOCKET sock = (SOCKET)client_socket;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        printf("Error receiving data: %d\n", WSAGetLastError());
        closesocket(sock);
        return 1;
    }
    buffer[bytes_received] = '\0';
    printf("Request received:\n%s\n", buffer);

    log_request(buffer);

    // Handle OPTIONS request for CORS preflight
    if (strncmp(buffer, "OPTIONS /tasks", 14) == 0) {
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                          "Access-Control-Allow-Headers: Authorization, Content-Type\r\n"
                          "Content-Length: 0\r\n"
                          "\r\n");
        send(sock, response, strlen(response), 0);
        closesocket(sock);
        return 0;
    }

    // Check authorization for non-OPTIONS requests
    if (!is_authorized(buffer)) {
        sprintf(response, "HTTP/1.1 401 Unauthorized\r\nAccess-Control-Allow-Origin: *\r\nWWW-Authenticate: Basic realm=\"To-Do API\"\r\nContent-Type: text/plain\r\n\r\nUnauthorized access");
        send(sock, response, strlen(response), 0);
        closesocket(sock);
        return 1;
    }

    // Handle POST request to create a new task
    if (strncmp(buffer, "POST /tasks", 11) == 0) {
        char *body = strstr(buffer, "\r\n\r\n");
        if (body && task_count < MAX_TASKS) {
            body += 4;
            if (strlen(body) > 99) {
                sprintf(response, "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nDescription too long");
            } else {
                tasks[task_count].id = task_count + 1;
                strncpy(tasks[task_count].description, body, 99);
                tasks[task_count].description[99] = '\0';
                tasks[task_count].completed = 0;
                task_count++;
                save_tasks();
                sprintf(response, "HTTP/1.1 201 Created\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"id\": %d, \"description\": \"%s\", \"completed\": false}", 
                        tasks[task_count-1].id, tasks[task_count-1].description);
            }
        } else {
            sprintf(response, "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nBody required or task limit reached");
        }
    }
    // Handle GET request to retrieve all tasks
    else if (strncmp(buffer, "GET /tasks", 10) == 0) {
        char task_list[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"tasks\": [";
        for (int i = 0; i < task_count; i++) {
            char task[256];
            sprintf(task, "{\"id\": %d, \"description\": \"%s\", \"completed\": %s}%s", 
                    tasks[i].id, tasks[i].description, tasks[i].completed ? "true" : "false", 
                    i < task_count - 1 ? "," : "");
            strcat(task_list, task);
        }
        strcat(task_list, "]}");
        strcpy(response, task_list);
    }
    // Handle PUT request to update a task as completed
    else if (strncmp(buffer, "PUT /tasks/", 11) == 0) {
        char *id_start = buffer + 11;
        char *id_end = strchr(id_start, ' ');
        if (id_end) {
            *id_end = '\0';
            int id = atoi(id_start);
            for (int i = 0; i < task_count; i++) {
                if (tasks[i].id == id) {
                    tasks[i].completed = 1;
                    save_tasks();
                    sprintf(response, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"id\": %d, \"description\": \"%s\", \"completed\": true}", 
                            tasks[i].id, tasks[i].description);
                    send(sock, response, strlen(response), 0);
                    closesocket(sock);
                    return 0;
                }
            }
            sprintf(response, "HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nTask not found");
        } else {
            sprintf(response, "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nInvalid task ID");
        }
    }
    // Handle DELETE request to delete a task
    else if (strncmp(buffer, "DELETE /tasks/", 14) == 0) {
        char *id_start = buffer + 14;
        char *id_end = strchr(id_start, ' ');
        if (id_end) {
            *id_end = '\0';
            int id = atoi(id_start);
            for (int i = 0; i < task_count; i++) {
                if (tasks[i].id == id) {
                    for (int j = i; j < task_count - 1; j++) {
                        tasks[j] = tasks[j + 1];
                    }
                    task_count--;
                    save_tasks();
                    sprintf(response, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\r\n{\"deleted\": \"Task %d\"}", id);
                    send(sock, response, strlen(response), 0);
                    closesocket(sock);
                    return 0;
                }
            }
            sprintf(response, "HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nTask not found");
        } else {
            sprintf(response, "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nInvalid task ID");
        }
    }
    // Handle unknown routes
    else {
        sprintf(response, "HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n\r\nWrong route buddy");
    }

    send(sock, response, strlen(response), 0);
    closesocket(sock);
    return 0;
}

// Main function to initialize the server and handle incoming connections.
int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    load_tasks();

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server running on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;
        }

        HANDLE thread = CreateThread(NULL, 0, handle_client, (LPVOID)client_socket, 0, NULL);
        if (thread == NULL) {
            printf("Failed to create thread: %lu\n", GetLastError());
            closesocket(client_socket);
        } else {
            CloseHandle(thread);
        }
    }

    save_tasks();
    closesocket(server_socket);
    WSACleanup();
    return 0;
}