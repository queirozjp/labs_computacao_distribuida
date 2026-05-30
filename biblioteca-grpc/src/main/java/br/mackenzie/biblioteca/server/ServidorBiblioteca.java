package br.mackenzie.biblioteca.server;

import io.grpc.Server;
import io.grpc.ServerBuilder;

import java.io.IOException;
import java.util.concurrent.TimeUnit;

public class ServidorBiblioteca {

    private static final int PORTA = 50051;

    public static void main(String[] args) throws IOException, InterruptedException {

        // Configura e inicializa o servidor gRPC
        Server server = ServerBuilder
                .forPort(PORTA)
                .addService(new BibliotecaServiceImpl()) // Registra a implementacao do servico
                .intercept(new AuthInterceptor())        // Injeta o interceptor de autenticacao na pipeline
                .build();

        server.start();

        System.out.println("--------------------------------------------");
        System.out.println("Servidor Biblioteca gRPC iniciado");
        System.out.println("Porta: " + PORTA);
        System.out.println("Auth (Bearer): " + AuthInterceptor.TOKEN_VALIDO);
        System.out.println("--------------------------------------------");

        // Hook na JVM para capturar sinais de interrupcao (ex: SIGINT/Ctrl+C)
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("Sinal de interrupcao recebido. Encerrando servidor gRPC...");
            try {
                // Forca o encerramento das conexoes ativas com timeout de 5 segundos
                server.shutdown().awaitTermination(5, TimeUnit.SECONDS);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
            System.out.println("Servidor encerrado.");
        }));

        // Bloqueia a thread principal para manter o processo do servidor rodando
        server.awaitTermination();
    }
}