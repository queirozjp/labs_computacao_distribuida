package br.mackenzie.biblioteca.server;

import br.mackenzie.biblioteca.proto.*;
import io.grpc.Status;
import io.grpc.stub.StreamObserver;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class BibliotecaServiceImpl extends BibliotecaServiceGrpc.BibliotecaServiceImplBase {

    // Armazenamento em memoria
    private final Map<String, LivroResponse> acervo = new ConcurrentHashMap<>();
    private final Set<String> isbnsUsados = ConcurrentHashMap.newKeySet();
    private final AtomicInteger contadorId = new AtomicInteger(1);

    private static final Map<String, String[]> SUGESTOES = new LinkedHashMap<>() {{
        put("terror",       new String[]{"It - A Coisa", "Stephen King"});
        put("romance",      new String[]{"Orgulho e Preconceito", "Jane Austen"});
        put("fantasia",     new String[]{"O Senhor dos Aneis", "J.R.R. Tolkien"});
        put("ciencia",      new String[]{"Uma Breve Historia do Tempo", "Stephen Hawking"});
        put("distopia",     new String[]{"1984", "George Orwell"});
        put("aventura",     new String[]{"As Aventuras de Tom Sawyer", "Mark Twain"});
        put("filosofia",    new String[]{"O Mundo de Sofia", "Jostein Gaarder"});
        put("tecnologia",   new String[]{"Clean Code", "Robert C. Martin"});
        put("misterio",     new String[]{"O Nome da Rosa", "Umberto Eco"});
        put("default",      new String[]{"O Guia do Mochileiro das Galaxias", "Douglas Adams"});
    }};

    @Override
    public void cadastrarLivro(LivroRequest request, StreamObserver<LivroResponse> responseObserver) {

        System.out.printf("[cadastrarLivro] titulo='%s' autor='%s' ano=%d isbn='%s'%n",
                request.getTitulo(), request.getAutor(), request.getAno(), request.getIsbn());

        // Valida inputs obrigatorios
        if (request.getTitulo().isBlank() || request.getIsbn().isBlank()) {
            responseObserver.onError(Status.INVALID_ARGUMENT
                    .withDescription("Titulo e ISBN sao obrigatorios.")
                    .asRuntimeException());
            return;
        }

        // Valida duplicidade de ISBN
        if (isbnsUsados.contains(request.getIsbn())) {
            responseObserver.onError(Status.ALREADY_EXISTS
                    .withDescription("Livro com ISBN '" + request.getIsbn() + "' ja cadastrado.")
                    .asRuntimeException());
            return;
        }

        String id = "LIV-" + contadorId.getAndIncrement();

        LivroResponse livro = LivroResponse.newBuilder()
                .setId(id)
                .setTitulo(request.getTitulo())
                .setAutor(request.getAutor())
                .setAno(request.getAno())
                .setIsbn(request.getIsbn())
                .setStatus("CADASTRADO")
                .setMensagem("Livro cadastrado com sucesso! ID: " + id)
                .build();

        // Atualiza estado do servidor
        acervo.put(id, livro);
        isbnsUsados.add(request.getIsbn());

        System.out.println("[cadastrarLivro] Cadastrado: " + id);

        // Emite resposta unica e encerra a chamada (Unary)
        responseObserver.onNext(livro);
        responseObserver.onCompleted();
    }

    @Override
    public void listarLivrosPorAutor(AutorRequest request, StreamObserver<LivroResponse> responseObserver) {

        String autor = request.getAutor();
        System.out.println("[listarLivrosPorAutor] autor='" + autor + "'");

        // Filtra acervo em memoria
        List<LivroResponse> encontrados = acervo.values().stream()
                .filter(l -> l.getAutor().equalsIgnoreCase(autor))
                .toList();

        // Retorna erro gRPC se nenhum dado for encontrado
        if (encontrados.isEmpty()) {
            System.out.println("[listarLivrosPorAutor] Nenhum livro encontrado para: " + autor);
            responseObserver.onError(Status.NOT_FOUND
                    .withDescription("Nenhum livro encontrado para o autor: " + autor)
                    .asRuntimeException());
            return;
        }

        // Stream de respostas do servidor: emite um onNext iterativo para cada item
        for (LivroResponse livro : encontrados) {
            System.out.println("[listarLivrosPorAutor] -> enviando: " + livro.getTitulo());
            responseObserver.onNext(livro);
        }

        responseObserver.onCompleted();
        System.out.println("[listarLivrosPorAutor] Stream concluido - " + encontrados.size() + " livro(s).");
    }

    @Override
    public StreamObserver<EmprestimoRequest> registrarEmprestimos(StreamObserver<EmprestimoResumo> responseObserver) {

        System.out.println("[registrarEmprestimos] Iniciando recepcao de stream de emprestimos...");

        // Instancia um observer para processar o stream contínuo enviado pelo cliente
        return new StreamObserver<>() {

            private int total = 0;
            private final long inicio = System.currentTimeMillis();

            @Override
            public void onNext(EmprestimoRequest req) {
                // Processa o pacote recebido do cliente e acumula no estado local
                total++;
                System.out.printf("[registrarEmprestimos] Emprestimo #%d - usuario='%s' livro_id='%s'%n",
                        total, req.getUsuario(), req.getLivroId());

                if (!acervo.containsKey(req.getLivroId())) {
                    System.out.println("[registrarEmprestimos] Livro nao encontrado: " + req.getLivroId());
                }
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("[registrarEmprestimos] Erro no stream: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                // Ao detectar encerramento por parte do cliente, devolve uma única reposta de resumo
                long tempo = System.currentTimeMillis() - inicio;
                System.out.printf("[registrarEmprestimos] Stream concluido - %d emprestimo(s) em %dms%n",
                        total, tempo);

                EmprestimoResumo resumo = EmprestimoResumo.newBuilder()
                        .setTotalEmprestimos(total)
                        .setTempoProcessamentoMs(tempo)
                        .setMensagem(total + " emprestimo(s) registrado(s) com sucesso em " + tempo + "ms.")
                        .build();

                responseObserver.onNext(resumo);
                responseObserver.onCompleted();
            }
        };
    }

    @Override
    public StreamObserver<ChatMensagem> chatBibliotecario(StreamObserver<ChatSugestao> responseObserver) {

        System.out.println("[chatBibliotecario] Chat iniciado.");

        // Retorna observer para comunicação bidirecional simultânea (Bidi streaming)
        return new StreamObserver<>() {

            @Override
            public void onNext(ChatMensagem msg) {
                String chave = msg.getPalavraChave().toLowerCase().trim();
                String usuario = msg.getUsuario();

                System.out.printf("[chatBibliotecario] Mensagem de '%s': '%s'%n", usuario, chave);

                // Busca relacional de sugestões
                String[] sugestao = SUGESTOES.entrySet().stream()
                        .filter(e -> chave.contains(e.getKey()))
                        .map(Map.Entry::getValue)
                        .findFirst()
                        .orElse(SUGESTOES.get("default"));

                ChatSugestao resposta = ChatSugestao.newBuilder()
                        .setSugestaoTitulo(sugestao[0])
                        .setSugestaoAutor(sugestao[1])
                        .setMotivo("Baseado na palavra-chave '" + chave + "', sugiro este classico!")
                        .build();

                System.out.printf("[chatBibliotecario] -> Sugestao para '%s': '%s'%n",
                        usuario, sugestao[0]);

                // Envia dados de volta assim que recebe e processa o frame
                responseObserver.onNext(resposta);
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("[chatBibliotecario] Erro: " + t.getMessage());
            }

            @Override
            public void onCompleted() {
                System.out.println("[chatBibliotecario] Chat encerrado.");
                responseObserver.onCompleted();
            }
        };
    }
}