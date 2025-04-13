# C API To-Do List - Vercel Version

Este projeto é uma adaptação da API To-Do List originalmente escrita em C para funcionar no Vercel.

## Como usar

1. Instale o Vercel CLI:
```bash
npm install -g vercel
```

2. Faça login no Vercel:
```bash
vercel login
```

3. Deploy o projeto:
```bash
vercel
```

## Endpoints

A API mantém as mesmas funcionalidades da versão em C:

- `GET /tasks` - Lista todas as tarefas
- `POST /tasks` - Cria uma nova tarefa
- `PUT /tasks/:id` - Marca uma tarefa como completa
- `DELETE /tasks/:id` - Remove uma tarefa

## Autenticação

Use o mesmo header de autenticação da versão C:
```
Authorization: Basic dXNlcjpwYXNz
```

## Notas

- A API mantém a mesma interface da versão C
- Os dados são armazenados em memória com backup em arquivo
- Todas as rotas têm CORS habilitado
- A autenticação básica é mantida igual à versão original
