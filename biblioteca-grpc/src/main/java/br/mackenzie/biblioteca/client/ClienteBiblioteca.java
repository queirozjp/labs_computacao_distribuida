package br.mackenzie.biblioteca.client;

import br.mackenzie.biblioteca.proto.*;
import br.mackenzie.biblioteca.server.AuthInterceptor;
import io.grpc.*;
import io.grpc.stub.MetadataUtils;
import io.grpc.stub.StreamObserver;

import java.util.Iterator;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class ClienteBiblioteca {

    private static final String HOST  = "localhost";
    private static final int    PORTA = 50051;

    // Chave do header de autorizacao
    private static final Metadata.Key<String> AUTH_KEY =
            Metadata.Key.of("authorization", Metadata.ASCII_STRING_MARSHALLER);

    public static void main(String[] args) throws InterruptedException {

        // Inicializa canal gRPC
        ManagedChannel canal = ManagedChannelBuilder
                .forAddress(HOST, PORTA)
                .usePlaintext()
                .build();

        // Configura metadata com token
        Metadata authHeader = new Metadata();
        authHeader.put(AUTH_KEY, "Bearer " + AuthInterceptor.TOKEN_VALIDO);

        // Stub bloqueante (Unary/Server Streaming)
        BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stubBlocking =
                BibliotecaServiceGrpc.newBlockingStub(canal)
                        .withInterceptors(MetadataUtils.newAttachHeadersInterceptor(authHeader));

        // Stub assincrono (Client/Bidi Streaming)
        BibliotecaServiceGrpc.BibliotecaServiceStub stubAsync =
                BibliotecaServiceGrpc.newStub(canal)
                        .withInterceptors(MetadataUtils.newAttachHeadersInterceptor(authHeader));

        try {
            secao("UNARY - cadastrarLivro");

            String[] ids = new String[3];
            ids[0] = cadastrarLivro(stubBlocking, "Dom Quixote", "Miguel de Cervantes", 1605, "978-0142437230");
            ids[1] = cadastrarLivro(stubBlocking, "Cem Anos de Solidao", "Gabriel Garcia Marquez", 1967, "978-0060883287");
            ids[2] = cadastrarLivro(stubBlocking, "El coronel no tiene...", "Gabriel Garcia Marquez", 1961, "978-0060751555");

            System.out.println("\n[Teste] Tentativa de ISBN duplicado:");
            cadastrarLivroComErro(stubBlocking, "Dom Quixote Copia", "Outro Autor", 2020, "978-0142437230");

            secao("SERVER STREAMING - listarLivrosPorAutor");

            System.out.println("Listando livros de 'Gabriel Garcia Marquez':");
            listarPorAutor(stubBlocking, "Gabriel Garcia Marquez");

            System.out.println("\nListando livros de 'Autor Inexistente' (espera NOT_FOUND):");
            listarPorAutor(stubBlocking, "Autor Inexistente");

            secao("CLIENT STREAMING - registrarEmprestimos");
            registrarEmprestimos(stubAsync, ids);

            secao("BIDIRECTIONAL STREAMING - chatBibliotecario");
            chatBibliotecario(stubAsync);

            secao("BONUS - Token invalido (espera UNAUTHENTICATED)");
            testarTokenInvalido(canal);

        } finally {
            // Encerra canal com timeout
            canal.shutdown().awaitTermination(5, TimeUnit.SECONDS);
        }
    }

    private static String cadastrarLivro(
            BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stub,
            String titulo, String autor, int ano, String isbn) {

        // Chamada Unary
        LivroRequest req = LivroRequest.newBuilder()
                .setTitulo(titulo).setAutor(autor).setAno(ano).setIsbn(isbn)
                .build();
        try {
            LivroResponse resp = stub.cadastrarLivro(req);
            System.out.printf("%s | ID=%s%n", resp.getMensagem(), resp.getId());
            return resp.getId();
        } catch (StatusRuntimeException e) {
            System.err.println("Erro: " + e.getStatus());
            return "";
        }
    }

    private static void cadastrarLivroComErro(
            BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stub,
            String titulo, String autor, int ano, String isbn) {

        LivroRequest req = LivroRequest.newBuilder()
                .setTitulo(titulo).setAutor(autor).setAno(ano).setIsbn(isbn)
                .build();
        try {
            stub.cadastrarLivro(req);
        } catch (StatusRuntimeException e) {
            System.out.printf("Status recebido: %s - %s%n",
                    e.getStatus().getCode(), e.getStatus().getDescription());
        }
    }

    private static void listarPorAutor(
            BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stub,
            String autor) {

        AutorRequest req = AutorRequest.newBuilder().setAutor(autor).build();
        try {
            // Itera sobre stream do servidor
            Iterator<LivroResponse> stream = stub.listarLivrosPorAutor(req);
            int count = 0;
            while (stream.hasNext()) {
                LivroResponse l = stream.next();
                System.out.printf("%s - %s (%d) [%s]%n",
                        l.getTitulo(), l.getAutor(), l.getAno(), l.getId());
                count++;
            }
            System.out.println("Total: " + count + " livro(s).");
        } catch (StatusRuntimeException e) {
            System.out.printf("Status recebido: %s - %s%n",
                    e.getStatus().getCode(), e.getStatus().getDescription());
        }
    }

    private static void registrarEmprestimos(
            BibliotecaServiceGrpc.BibliotecaServiceStub stub,
            String[] ids) throws InterruptedException {

        // Sincronizacao da thread principal
        CountDownLatch latch = new CountDownLatch(1);

        // Callback da resposta do servidor
        StreamObserver<EmprestimoRequest> requestObserver =
                stub.registrarEmprestimos(new StreamObserver<>() {
                    @Override
                    public void onNext(EmprestimoResumo resumo) {
                        System.out.println("Resumo recebido: " + resumo.getMensagem());
                        System.out.println("Total emprestimos: " + resumo.getTotalEmprestimos());
                        System.out.println("Tempo processamento: " + resumo.getTempoProcessamentoMs() + "ms");
                    }
                    @Override public void onError(Throwable t) {
                        System.err.println("Erro: " + t.getMessage());
                        latch.countDown();
                    }
                    @Override public void onCompleted() { latch.countDown(); }
                });

        String[] usuarios = {"Alice", "Bob", "Carlos", "Diana", "Eduardo"};
        for (int i = 0; i < 5; i++) {
            String livroId = ids[i % ids.length];
            if (livroId.isBlank()) livroId = "LIV-1";
            EmprestimoRequest req = EmprestimoRequest.newBuilder()
                    .setUsuario(usuarios[i])
                    .setLivroId(livroId)
                    .build();
            System.out.printf("Enviando emprestimo: usuario='%s' livro='%s'%n",
                    usuarios[i], livroId);

            // Push na stream
            requestObserver.onNext(req);
            Thread.sleep(100);
        }

        // Fecha stream do cliente
        requestObserver.onCompleted();

        boolean concluido = latch.await(10, TimeUnit.SECONDS);
        if (!concluido) System.err.println("Timeout aguardando resumo.");
    }

    private static void chatBibliotecario(
            BibliotecaServiceGrpc.BibliotecaServiceStub stub) throws InterruptedException {

        CountDownLatch latch = new CountDownLatch(1);

        // Callback da stream bidirecional
        StreamObserver<ChatMensagem> requestObserver =
                stub.chatBibliotecario(new StreamObserver<>() {
                    @Override
                    public void onNext(ChatSugestao sugestao) {
                        System.out.printf("Sugestao: '%s' de %s%n %s%n",
                                sugestao.getSugestaoTitulo(),
                                sugestao.getSugestaoAutor(),
                                sugestao.getMotivo());
                    }
                    @Override public void onError(Throwable t) {
                        System.err.println("Erro no chat: " + t.getMessage());
                        latch.countDown();
                    }
                    @Override public void onCompleted() { latch.countDown(); }
                });

        String[][] mensagens = {
                {"Joao", "terror"},
                {"Joao", "fantasia"},
                {"Joao", "tecnologia"}
        };

        for (String[] m : mensagens) {
            ChatMensagem msg = ChatMensagem.newBuilder()
                    .setUsuario(m[0])
                    .setPalavraChave(m[1])
                    .build();
            System.out.printf("[%s] enviou: '%s'%n", m[0], m[1]);

            requestObserver.onNext(msg);
            Thread.sleep(300);
        }

        requestObserver.onCompleted();

        boolean concluido = latch.await(10, TimeUnit.SECONDS);
        if (!concluido) System.err.println("Timeout aguardando fim do chat.");
    }

    private static void testarTokenInvalido(ManagedChannel canal) {
        Metadata headerInvalido = new Metadata();
        headerInvalido.put(AUTH_KEY, "Bearer TOKEN-ERRADO");

        // Injeta token invalido no interceptor
        BibliotecaServiceGrpc.BibliotecaServiceBlockingStub stubInvalido =
                BibliotecaServiceGrpc.newBlockingStub(canal)
                        .withInterceptors(MetadataUtils.newAttachHeadersInterceptor(headerInvalido));
        try {
            stubInvalido.cadastrarLivro(
                    LivroRequest.newBuilder()
                            .setTitulo("Livro Bloqueado").setAutor("Hacker")
                            .setAno(2024).setIsbn("000-0000000000")
                            .build());
        } catch (StatusRuntimeException e) {
            System.out.printf("Status recebido: %s - %s%n",
                    e.getStatus().getCode(), e.getStatus().getDescription());
        }
    }

    private static void secao(String titulo) {
        System.out.println("\n--------------------------------------------");
        System.out.println(titulo);
        System.out.println("--------------------------------------------");
    }
}