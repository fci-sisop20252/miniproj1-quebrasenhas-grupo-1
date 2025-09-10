# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** Nome (Matrícula), Nome (Matrícula),,,  
---
## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

Eu primeiro faço a divisão inteira do espaço de busca pelo numero de workers e guardo em "passwords_per_worker" depois eu guardo o resto da divisão em "remaining". Então eu defino as variáveis "l"(indice inferior), "r"(indice superior) e "count"(quantidade de senhas a serem calculadas)."count" vai ser igual a "passwords_per_worker" counr + 1 se o espaço total de busca não for divisivel pelo quantidade de workers a ainda houver pelo menos uma unidade sobrando. "r" vai ser igual a l (inicialmente igual a 0) + count - 1. "l" vai ser igual a r + 1;


**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```
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
```

---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**

Depois de calcular a senha inicial e a senha final para servirem como parametros para o worker, eu fiz N chamadas a "fork" (N = quantidade de workers) dentro de um loop e então chamei "execl" para substituir a imagem do processo filho pela do worker e passei os arguementos necessários por linha de comando.
Posteriormente eu fiz N chamadas a "wait" de forma que o processo pai esperasse pelo termino de execução de todos os seus filhos. 

**Código do fork/exec:**
```
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
```

---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**

[Explique como você implementou uma escrita atômica e como isso evita condições de corrida]
Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**

O "coordinator" espera pela conclusão de todos os workers e então tenta ler o arquivo "found_password.txt"
onde os workers gravam a senha encontrada, se o arquivo não existir quer dizer que a senha não pode ser descoberta. Uma vez que o arquivo foi encontrado, o "coordinator" ira extrair a senha encontrada da linha que foi lida a partir do caracter que esteja na posição "quantidade de bytes lidos pela chamada read - o comprimento da senha" e extrair uma quantidade de caracteres equivalente ao comprimento da senha.  

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | ___s | ___s | ___s | ___ |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | ___s | ___s | ___s | ___ |

**O speedup foi linear? Por quê?**
[Analise se dobrar workers realmente dobrou a velocidade e explique o overhead de criar processos]

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
[Descreva um problema e como resolveu. Ex: "Tive dificuldade com o incremento de senha, mas resolvi tratando-o como um contador em base variável"]

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [ ] Código compila sem erros
- [ ] Todos os TODOs foram implementados
- [ ] Testes passam no `./tests/simple_test.sh`
- [ ] Relatório preenchido