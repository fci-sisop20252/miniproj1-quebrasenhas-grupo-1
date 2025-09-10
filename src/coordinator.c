#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 *
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 *
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 *
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 *
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 *
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 *
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len,
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1

    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro
    if (argc != 5){
        printf("Numero de argumento invalido.\n");
        exit(1);
    }

    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);

    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio

    if (password_len < 1 || password_len > 10){
        printf("Largura da senha invalida.\n");
        exit(1);
    }

    if (num_workers < 1 || num_workers > MAX_WORKERS){
        printf("Quantidde de workers invalida.\n");
        exit(1);
    }


    if (charset[0] == '\0'){
        printf("Charset não pode estar vazio.\n");
        exit(1);
    }

    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);

    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);

    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);

    // Registrar tempo de início
    time_t start_time = time(NULL);

    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers

    // IMPLEMENTE AQUI:
    // long long passwords_per_worker = ?
    // long long remaining = ?

    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;

    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];

    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");


    pid_t fd;
    long long count, r, l = 0;

    char start_password[password_len], end_password[password_len];
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
        // TODO: Calcular intervalo de senhas para este worker
        // TODO: Converter indices para senhas de inicio e fim
        // TODO 4: Usar fork() para criar processo filho
        // TODO 5: No processo pai: armazenar PID
        // TODO 6: No processo filho: usar execl() para executar worker
        // TODO 7: Tratar erros de fork() e execl()
        count = passwords_per_worker + (i < remaining ? 1: 0);
        r = l + count - 1;
      

        if (r > total_space) r = total_space - 1;

        index_to_password(l, charset, charset_len, password_len, start_password);
        index_to_password(r, charset, charset_len, password_len, end_password);

        l = r + 1;

        fd = fork();
        if (fd > 0){
                workers[i] = fd;
        }
        else if (fd == 0){
                char s_password_len[12];
                char s_worker_id[12];
                snprintf(s_password_len, 12, "%d", password_len);
                snprintf(s_worker_id, 12, "%d", workers[i]);
                execl("./worker", "worker",target_hash, start_password, end_password, charset, s_password_len, s_worker_id, (char *)NULL);
                printf("Erro ao calcular a senha.\n");
                exit(1);
        }
        else{
                printf("Erro ao calcular a senha.\n");
                exit(1);
        }

    }

    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");

    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis

    // IMPLEMENTE AQUI:
    // - Loop para aguardar cada worker terminar
    // - Usar wait() para capturar status de saída
    // - Identificar qual worker terminou
    // - Verificar se terminou normalmente ou com erro
    // - Contar quantos workers terminaram
    int status;
    pid_t worker;
    for (int i=0;i<num_workers;i++){
        worker = wait(&status);
        printf("%d terminou.\n", worker);
        if (WIFEXITED(status)){
                int code = WEXITSTATUS(status);
                if (code == 0)
                        printf("Processo terminou com sucesso.\n");
                else
                        printf("Processo terminou com erro.\n");
        }
        else{
                printf("Processo terminou com erro.\n");
        }
    }


    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);

    printf("\n=== Resultado ===\n");

    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir

    // IMPLEMENTE AQUI:
    // - Abrir arquivo RESULT_FILE para leitura
    // - Ler conteúdo do arquivo
    // - Fazer parse do formato "worker_id:password"
    // - Verificar o hash usando md5_string()
    // - Exibir resultado encontrado

    // Estatísticas finais (opcional)
    // TODO: Calcular e exibir estatísticas de performance

    int result_file_fd = open(RESULT_FILE, O_RDONLY);
    if (result_file_fd < 0){
        printf("A senha não pode ser encontrada.\n");
    }
    else{
        char buffer[password_len + 100];
        ssize_t read_bytes = read(result_file_fd, buffer, sizeof(buffer) - 1);

        if (read_bytes < 0){
                printf("Algo deu errado.\n");
                exit(1);
        }
        buffer[read_bytes] = '\0';

        char found_password[password_len+1];
        strncpy(found_password, &buffer[strlen(buffer) - password_len - 1], password_len);
        found_password[password_len] = '\0';
        
        char found_password_cpy[password_len+1];
        strcpy(found_password_cpy, found_password);
        char hash[password_len+1];

        md5_string(found_password_cpy, hash);

        if (strcmp(hash, target_hash) == 0)
                printf("Senha: %s", found_password);
        else

                printf("Senha não pode ser encontrada.\n");
    }

    printf("Tempo de execução: %.10f", elapsed_time);
    return 0;
}