package br.mackenzie.biblioteca.server;

import io.grpc.*;

public class AuthInterceptor implements ServerInterceptor {

    public static final String TOKEN_VALIDO = "biblioteca-token-secreto";

    static final Metadata.Key<String> AUTHORIZATION_KEY =
            Metadata.Key.of("authorization", Metadata.ASCII_STRING_MARSHALLER);

    @Override
    public <ReqT, RespT> ServerCall.Listener<ReqT> interceptCall(
            ServerCall<ReqT, RespT> call,
            Metadata headers,
            ServerCallHandler<ReqT, RespT> next) {

        // Extrai o header de autorizacao da requisicao
        String authHeader = headers.get(AUTHORIZATION_KEY);

        // Valida o token recebido
        if (authHeader == null || !authHeader.equals("Bearer " + TOKEN_VALIDO)) {
            System.out.println("[AUTH] Token invalido ou ausente: " + authHeader);

            // Rejeita a chamada com status UNAUTHENTICATED
            call.close(
                    Status.UNAUTHENTICATED
                            .withDescription("Token de autenticacao invalido ou ausente."),
                    new Metadata()
            );

            // Interrompe o fluxo retornando um listener vazio
            return new ServerCall.Listener<>() {};
        }

        System.out.println("[AUTH] Token valido - metodo: "
                + call.getMethodDescriptor().getFullMethodName());

        // Continua a cadeia de execucao do gRPC
        return next.startCall(call, headers);
    }
}