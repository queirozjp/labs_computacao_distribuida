# 📚 Biblioteca Digital — Sistema gRPC

Sistema de Gerenciamento de Biblioteca Digital distribuído, implementado em Java 17 + Maven + gRPC 1.68.x com Protocol Buffers 3.

## Descrição

O projeto implementa um servidor gRPC central que oferece **4 tipos de comunicação RPC** para gerenciar livros, empréstimos e um chat com bibliotecário virtual.

Inclui o bônus de **autenticação via metadata** (token Bearer no header de todas as chamadas).

---

## Grupo

| Nome | RA | 
|------|----|
| Henrique Brainer Costa | 10420717 | 
| João Pedro Queiroz de Andrade | 10425822 |
| João Victor Vidal Barbosa | 10410165 |

---

## Operações RPC implementadas

| # | Tipo | Método |
|---|------|--------|
| 1 | Unary | `cadastrarLivro` |
| 2 | Server Streaming | `listarLivrosPorAutor` |
| 3 | Client Streaming | `registrarEmprestimos` |
| 4 | Bidirectional Streaming | `chatBibliotecario` |

## Bônus — Autenticação via Metadata

Todas as chamadas do cliente enviam o header:

```
authorization: Bearer biblioteca-token-secreto
```

O `AuthInterceptor` no servidor intercepta cada chamada antes de chegar ao serviço. Se o token estiver ausente ou incorreto, a chamada é rejeitada imediatamente com `Status.UNAUTHENTICATED`.

---

## Como compilar e executar

### 1. Compilar

```bash
mvn clean package -q
```

### 2. Iniciar o servidor
Windows:
```bash
mvn exec:java "-Dexec.mainClass=br.mackenzie.biblioteca.server.ServidorBiblioteca"
```
linux:
```bash
mvn exec:java -Dexec.mainClass="br.mackenzie.biblioteca.server.ServidorBiblioteca"
```

Saída esperada:
```
════════════════════════════════════════════
     Servidor Biblioteca gRPC iniciado!
  Porta : 50051
  Auth  : Bearer biblioteca-token-secreto
════════════════════════════════════════════
```

### 3. Executar o cliente (em outro terminal)
Windows:
```bash
mvn exec:java "-Dexec.mainClass=br.mackenzie.biblioteca.client.ClienteBiblioteca"
```

linux:
```bash
mvn exec:java -Dexec.mainClass="br.mackenzie.biblioteca.client.ClienteBiblioteca"
```

---

## Roteiro de testes demonstrado pelo cliente

1. **Cadastro de 3 livros** (Unary RPC)
2. **Listagem por autor existente** — `Gabriel García Márquez` (Server Streaming)
3. **Listagem por autor inexistente** — retorna `NOT_FOUND` (Server Streaming)
4. **Registro de 5 empréstimos consecutivos** (Client Streaming)
5. **Chat com 3 mensagens** — terror, fantasia, tecnologia (Bidirectional Streaming)
6. **ISBN duplicado** — retorna `ALREADY_EXISTS` (Unary RPC com erro)
7. **Token inválido** — retorna `UNAUTHENTICATED` (Bônus)

### Teste - Lado do Cliente: 

![alt text](image.png)
![alt text](image-1.png)

### Teste - Lado do Servidor

![alt text](image-2.png)
![alt text](image-3.png)