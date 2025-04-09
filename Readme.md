# Sistema de Gerenciamento de Tarefas com API em C

Este projeto implementa uma API HTTP em C para gerenciar tarefas, integrada com um frontend simples em HTML/JavaScript. Abaixo estão as principais funcionalidades e implementações realizadas.

## Funcionalidades

### Backend (API em C)
- **Framework**: API HTTP em C com Winsock, rodando na porta 8080 com suporte a múltiplas conexões via threads.
- **Rotas**:
  - `GET /tasks`: Lista todas as tarefas em JSON.
  - `POST /tasks`: Adiciona uma nova tarefa.
  - `PUT /tasks/<id>`: Marca uma tarefa como concluída.
  - `DELETE /tasks/<id>`: Exclui uma tarefa.
  - `OPTIONS /tasks`: Suporte a CORS preflight.
- **Autenticação**: Autenticação Básica (usuário: `user`, senha: `pass`).
- **Persistência**: Tarefas salvas em `tasks.txt` (formato: `id,description,completed`) no diretório `c:\Users\silas\OneDrive\Desktop\API_C\`.
- **Logging**: Requisições registradas em `api_log.txt` com timestamp.
- **CORS**: Suporte a requisições cross-origin com cabeçalhos apropriados.
- **Segurança**: Limite de 99 caracteres nas descrições das tarefas.

### Frontend (HTML/JavaScript)
- **Interface**: Formulário para adicionar tarefas e lista dinâmica de tarefas com botões "Complete" e "Delete".
- **JavaScript**: Usa `fetch` para interagir com a API, com autenticação integrada.
- **Funcionalidades**:
  - Listagem automática de tarefas ao carregar.
  - Adição, conclusão e exclusão de tarefas em tempo real.

### Configuração
- **Compilação**: `gcc -Wall -Wextra -g3 C_API.c -o output/C_API.exe -lws2_32` no MinGW.
- **Execução**: API em `output/C_API.exe`; frontend em `index.html` aberto no navegador.

## Como Executar
1. Compile a API:
```bash
C:\MinGW\bin\gcc.exe -Wall -Wextra -g3 c:\Users\..\..\..\API_C\C_API.c -o c:\Users\..\..\Desktop\API_C\output\C_API.exe -lws2_32
```

2. Execute a API:
```bash
Localize o diretório do .exe
C_API.exe
```
3. Abra o frontend:
- Abra `index.html` no navegador (`file:///c:/Users/silas/OneDrive/Desktop/API_C/index.html`).

- ## Notas
- O projeto foi desenvolvido e testado no Windows com VS Code e MinGW.
- O `tasks.txt` é criado automaticamente ao adicionar a primeira tarefa.
