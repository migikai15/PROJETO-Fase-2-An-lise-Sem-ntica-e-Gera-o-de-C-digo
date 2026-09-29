/*

Gabriel Tortolio Fonseca - 10416751

Anna Luiza Stella Santos - 10417401



LEGENDA DE CORREÇÕES (Feedback do Professor e Otimizações):

//1. Alteracoes referentes a funcao len() (Lexico, Sintatico e Semantico).

//2. Alteracoes referentes a virgula ',' (Tokens, separacao de argumentos no print e em listas).

//3. Alteracoes referentes a listas [ ] (Tokens, analise de colchetes e tipagem "LIST").

//4. Alteracoes no isolamento de simbolos (O uso de strchr no analisador lexico para parar de engolir caracteres).

//5. Alteracoes na checagem semantica de matematica (Garantir que operacoes matematicas so ocorram com "INT").

//6. Codigo de Tres Enderecos (TAC) e remocao de labels inativos.

*/



#include <stdio.h>    

#include <stdlib.h>  

#include <string.h>  

#include <ctype.h>  



// ESTRUTURAS E GLOBAIS 



typedef enum {

    ERRO, EOS, IDENTIFICADOR, NUMERO, BOOLEANO,

    KW_RETURN, KW_FROM, KW_WHILE, KW_AS, KW_ELIF, KW_WITH, KW_ELSE, KW_IF,

    KW_BREAK, KW_LEN, KW_INPUT, KW_PRINT, KW_EXEC, KW_IN, KW_RAISE,

    KW_CONTINUE, KW_RANGE, KW_DEF, KW_FOR,

    OP_SOMA, OP_SUB, OP_MULT, OP_EXP, OP_DIV, OP_MOD, OP_AND, OP_OR, OP_NOT, OP_DIF,

    OP_IGUAL, OP_IS, OP_MENOR, OP_MAIOR, OP_MENORIGUAL, OP_MAIORIGUAL, OP_ATRIB,

    DEL_ABRE_PAR, DEL_FECHA_PAR, DEL_DOIS_PONTOS, 

    DEL_ABRE_COL, DEL_FECHA_COL, //3. Tokens de colchetes para listas

    DEL_ABRE_CHAVE, DEL_FECHA_CHAVE, 

    DEL_VIRGULA, //2. Token de vírgula adicionado

    DEL_ASPAS_SIMPLES, DEL_ASPAS_DUPLAS

} TAtomo;



typedef struct {

    TAtomo nome;

    char atributo[100];

    int linha;

} TInfoAtomo;



FILE *arquivo_fonte = NULL;    

FILE *arquivo_saida = NULL;

int linha_atual = 1;    

TInfoAtomo lookahead;    



typedef struct _TNo {

    char ID[16];

    int endereco;

    char tipo[7]; 

    int referenciada; 

    struct _TNo *prox;

} TNo;



TNo *tabela_simbolos = NULL;

int contador_endereco = 0;



// Variáveis para o Código de Três Endereços (TAC) - //6.

int rotulo_atual = 1;

int contador_temp = 0; 

FILE *arquivo_codigo = NULL;





TInfoAtomo obter_atomo();

void consome(TAtomo esperado);

void erro_lexico(const char *lexema);

void erro_sintatico(TAtomo esperado, TAtomo recebido);

void erro_semantico(const char *mensagem);



void analisa_comando();

void analisa_expressao(char *tipo_retorno, char *lugar_retorno);

void analisa_lista_args();





// FUNÇÕES DA TABELA DE SÍMBOLOS E GERAÇÃO DE CÓDIGO



int proximo_rotulo() {

    return rotulo_atual++;

}



void inserir_tabela_simbolos(char *id, char *tipo) {

    TNo *novo = (TNo *)malloc(sizeof(TNo));

    strncpy(novo->ID, id, 15);

    novo->ID[15] = '\0';

    novo->endereco = contador_endereco++;

    strncpy(novo->tipo, tipo, 6);

    novo->tipo[6] = '\0';

    novo->referenciada = 0; 

    novo->prox = tabela_simbolos;

    tabela_simbolos = novo;

}



TNo* obter_no_tabela(char *id) {

    TNo *atual = tabela_simbolos;

    while (atual != NULL) {

        if (strcmp(atual->ID, id) == 0) return atual;

        atual = atual->prox;

    }

    return NULL;

}



int busca_tabela_simbolos(char *id) {

    TNo *no = obter_no_tabela(id);

    if (no != NULL) {

        no->referenciada = 1; 

        return no->endereco;

    }

    char msg[100];

    sprintf(msg, "Variavel '%s' nao foi declarada ou tipada antes do uso.", id);

    erro_semantico(msg);

    return -1;

}



void verificar_variaveis_nao_usadas() {

    TNo *atual = tabela_simbolos;

    int encontrou_waring = 0;

    while (atual != NULL) {

        if (atual->referenciada == 0) {

            if (!encontrou_waring) {

                printf("\n--- AVISOS SEMANTICOS ---\n");

                encontrou_waring = 1;

            }

            printf("Aviso: A variavel '%s' (tipo %s) recebeu valor, mas nunca foi referenciada.\n", atual->ID, atual->tipo);

        }

        atual = atual->prox;

    }

}



void liberar_tabela() {

    TNo *atual = tabela_simbolos;

    while (atual != NULL) {

        TNo *temp = atual;

        atual = atual->prox;

        free(temp);

    }

}





// MENSAGENS E TRATAMENTO DE ERROS



const char* nome_token(TAtomo t) {

    switch(t) {

        case ERRO: return "ERRO"; case EOS: return "EOS";

        case IDENTIFICADOR: return "IDENTIFICADOR"; case NUMERO: return "NUMERO";

        case BOOLEANO: return "BOOLEANO"; case KW_IF: return "KW_IF";

        case KW_ELSE: return "KW_ELSE"; case KW_ELIF: return "KW_ELIF";

        case KW_WHILE: return "KW_WHILE"; case KW_FOR: return "KW_FOR";

        case KW_IN: return "KW_IN"; case KW_PRINT: return "KW_PRINT";

        case KW_INPUT: return "KW_INPUT"; 

        case KW_LEN: return "KW_LEN"; //1. Mapeamento do nome do token len

        case KW_RANGE: return "KW_RANGE"; case OP_ATRIB: return "OP_ATRIB"; 

        case OP_IGUAL: return "OP_IGUAL"; case OP_SOMA: return "OP_SOMA";

        case OP_SUB: return "OP_SUB"; case OP_MULT: return "OP_MULT";

        case OP_DIV: return "OP_DIV"; case OP_MOD: return "OP_MOD";

        case OP_MENOR: return "OP_MENOR"; case OP_MAIOR: return "OP_MAIOR";

        case OP_MENORIGUAL: return "OP_MENORIGUAL"; case OP_MAIORIGUAL: return "OP_MAIORIGUAL";

        case DEL_ABRE_PAR: return "DEL_ABRE_PAR"; case DEL_FECHA_PAR: return "DEL_FECHA_PAR"; 

        case DEL_ABRE_COL: return "DEL_ABRE_COL"; case DEL_FECHA_COL: return "DEL_FECHA_COL"; //3. Nome colchetes

        case DEL_DOIS_PONTOS: return "DEL_DOIS_PONTOS"; 

        case DEL_VIRGULA: return "DEL_VIRGULA"; //2. Mapeamento do nome do token vírgula

        default: return "OUTRO_TOKEN";

    }

}



void encerrar_arquivos() {

    if (arquivo_saida) fclose(arquivo_saida);

    if (arquivo_fonte) fclose(arquivo_fonte);

    if (arquivo_codigo) fclose(arquivo_codigo);

    liberar_tabela();

}



void erro_lexico(const char *lexema) {

    printf("ERRO LÉXICO na linha %d: %s\n", linha_atual, lexema);

    encerrar_arquivos();

    exit(EXIT_FAILURE); 

}



void erro_sintatico(TAtomo esperado, TAtomo recebido) {

    printf("ERRO SINTÁTICO na linha %d: Esperado %s, Recebido %s\n", 

           lookahead.linha, nome_token(esperado), nome_token(recebido));

    encerrar_arquivos();

    exit(EXIT_FAILURE);

}



void erro_semantico(const char *mensagem) {

    printf("ERRO SEMÂNTICO na linha %d: %s\n", lookahead.linha, mensagem);

    encerrar_arquivos();

    exit(EXIT_FAILURE);

}



void consome(TAtomo esperado) {

    if (lookahead.nome == esperado) {

        lookahead = obter_atomo();

    } else {

        erro_sintatico(esperado, lookahead.nome); 

    }

}





// ANALISADOR LÉXICO 



TInfoAtomo obter_atomo() {

    TInfoAtomo atomo;           

    atomo.atributo[0] = '\0';  

    int c;



    while ((c = fgetc(arquivo_fonte)) != EOF) {

        if (c == '\n') {

            linha_atual++;

        } else if (c == '#') {

            while ((c = fgetc(arquivo_fonte)) != '\n' && c != EOF);

            if (c == '\n') linha_atual++;

        } else if (!isspace(c)) {

            ungetc(c, arquivo_fonte);

            break;

        }

    }



    atomo.linha = linha_atual; 



    c = fgetc(arquivo_fonte);

    if (c == EOF) {

        atomo.nome = EOS;

        strcpy(atomo.atributo, "EOF");

        return atomo;

    }



    if (c == '"' || c == '\'') {

        char delim = c; int i = 0; atomo.atributo[i++] = c;

        while ((c = fgetc(arquivo_fonte)) != delim && c != '\n' && c != EOF) atomo.atributo[i++] = c;

        if (c == delim) atomo.atributo[i++] = c; else if (c == '\n') ungetc(c, arquivo_fonte);

        atomo.atributo[i] = '\0';

        atomo.nome = (delim == '"') ? DEL_ASPAS_DUPLAS : DEL_ASPAS_SIMPLES;

        goto REGISTRA_TOKEN;

    }



    int i = 0; 



    //4. Isolamento perfeito de delimitadores e operadores. Impede que a leitura engula ',' '[' ou ']'

    if (strchr("():=<>+-*%,[]", c) != NULL) {

        atomo.atributo[i++] = c;

        if (c == '=' || c == '<' || c == '>') {

            int prox = fgetc(arquivo_fonte);

            if (prox == '=') {

                atomo.atributo[i++] = prox;

            } else {

                if (prox != EOF) ungetc(prox, arquivo_fonte);

            }

        }

    } else {

        //4. Formação de identificadores e números agora PARA no primeiro delimitador listado no strchr

        atomo.atributo[i++] = c;

        while ((c = fgetc(arquivo_fonte)) != EOF && !isspace(c) && strchr("():=<>+-*%,[]", c) == NULL) {

            atomo.atributo[i++] = c;

        }

        if (c != EOF) ungetc(c, arquivo_fonte); 

    }

    

    atomo.atributo[i] = '\0';



    if (strcmp(atomo.atributo, "True") == 0 || strcmp(atomo.atributo, "False") == 0) atomo.nome = BOOLEANO;

    else if (strcmp(atomo.atributo, "if") == 0) atomo.nome = KW_IF;

    else if (strcmp(atomo.atributo, "elif") == 0) atomo.nome = KW_ELIF;

    else if (strcmp(atomo.atributo, "else") == 0) atomo.nome = KW_ELSE;

    else if (strcmp(atomo.atributo, "while") == 0) atomo.nome = KW_WHILE;

    else if (strcmp(atomo.atributo, "print") == 0) atomo.nome = KW_PRINT;

    else if (strcmp(atomo.atributo, "len") == 0) atomo.nome = KW_LEN; //1. Identificação da palavra reservada len

    else if (strcmp(atomo.atributo, "+") == 0) atomo.nome = OP_SOMA;

    else if (strcmp(atomo.atributo, "-") == 0) atomo.nome = OP_SUB;

    else if (strcmp(atomo.atributo, "*") == 0) atomo.nome = OP_MULT; //4. Identificação do operador de multiplicação

    else if (strcmp(atomo.atributo, "/") == 0) atomo.nome = OP_DIV;

    else if (strcmp(atomo.atributo, "%") == 0) atomo.nome = OP_MOD; //4. Identificação do operador de módulo

    else if (strcmp(atomo.atributo, "=") == 0) atomo.nome = OP_ATRIB;

    else if (strcmp(atomo.atributo, "==") == 0) atomo.nome = OP_IGUAL;

    else if (strcmp(atomo.atributo, "<") == 0) atomo.nome = OP_MENOR;

    else if (strcmp(atomo.atributo, ">") == 0) atomo.nome = OP_MAIOR;

    else if (strcmp(atomo.atributo, "<=") == 0) atomo.nome = OP_MENORIGUAL;

    else if (strcmp(atomo.atributo, ">=") == 0) atomo.nome = OP_MAIORIGUAL;

    else if (strcmp(atomo.atributo, "(") == 0) atomo.nome = DEL_ABRE_PAR; 

    else if (strcmp(atomo.atributo, ")") == 0) atomo.nome = DEL_FECHA_PAR; 

    else if (strcmp(atomo.atributo, "[") == 0) atomo.nome = DEL_ABRE_COL; //3. Identificação do abre colchete

    else if (strcmp(atomo.atributo, "]") == 0) atomo.nome = DEL_FECHA_COL; //3. Identificação do fecha colchete

    else if (strcmp(atomo.atributo, ":") == 0) atomo.nome = DEL_DOIS_PONTOS; 

    else if (strcmp(atomo.atributo, ",") == 0) atomo.nome = DEL_VIRGULA; //2. Identificação da vírgula

    else if (isdigit(atomo.atributo[0])) atomo.nome = NUMERO; 

    else if (isalpha(atomo.atributo[0]) || atomo.atributo[0] == '_') atomo.nome = IDENTIFICADOR;

    else atomo.nome = ERRO; 



    if (atomo.nome == ERRO) erro_lexico(atomo.atributo);



REGISTRA_TOKEN:

    if (arquivo_saida) fprintf(arquivo_saida, "%d# %s | %s\n", atomo.linha, nome_token(atomo.nome), atomo.atributo);

    return atomo;

}





// ANALISADOR SINTÁTICO, SEMÂNTICO E GERAÇÃO DE CÓDIGO (TAC)



void analisa_lista_args() {

    if (lookahead.nome == DEL_FECHA_PAR || lookahead.nome == DEL_FECHA_COL) return; 

    char tipo_lixo[7], lugar_lixo[100];

    analisa_expressao(tipo_lixo, lugar_lixo);

    //2. Usa a vírgula para processar múltiplos argumentos numa função

    while (lookahead.nome == DEL_VIRGULA) { 

        consome(DEL_VIRGULA); 

        analisa_expressao(tipo_lixo, lugar_lixo);

    }

}



void analisa_expressao(char *tipo_retorno, char *lugar_retorno) {

    char tipo_esq[7] = "UNK", tipo_dir[7] = "UNK";

    char lugar_esq[100] = "", lugar_dir[100] = "";



    if (lookahead.nome == DEL_ASPAS_DUPLAS || lookahead.nome == DEL_ASPAS_SIMPLES) {

        strcpy(tipo_esq, "STR");

        strcpy(lugar_esq, lookahead.atributo); //6. Armazena o valor bruto para o TAC

        consome(lookahead.nome);

    } else if (lookahead.nome == NUMERO) {

        strcpy(tipo_esq, "INT");

        strcpy(lugar_esq, lookahead.atributo); //6. Armazena o número para o TAC

        consome(NUMERO);

    } else if (lookahead.nome == BOOLEANO) {

        strcpy(tipo_esq, "BOOL");

        strcpy(lugar_esq, lookahead.atributo); //6. Armazena o bool para o TAC

        consome(BOOLEANO);

    } else if (lookahead.nome == IDENTIFICADOR) {

        char nome_var[100];

        strcpy(nome_var, lookahead.atributo);

        

        busca_tabela_simbolos(nome_var);

        TNo *no = obter_no_tabela(nome_var);

        if (no != NULL) strcpy(tipo_esq, no->tipo); 



        strcpy(lugar_esq, nome_var); //6. Armazena a variável para o TAC

        consome(IDENTIFICADOR);

        

        // --- Suporte a acesso de índice: list[i] ---

        if (lookahead.nome == DEL_ABRE_COL) {

            consome(DEL_ABRE_COL);

            char tipo_idx[7], lugar_idx[100];

            analisa_expressao(tipo_idx, lugar_idx);

            consome(DEL_FECHA_COL);

            

            //6. Geração TAC para buscar no índice

            sprintf(lugar_esq, "t%d", contador_temp++);

            fprintf(arquivo_codigo, "  %s = %s[%s]\n", lugar_esq, nome_var, lugar_idx);

            strcpy(tipo_esq, "INT"); 

        }

        

        if (lookahead.nome == DEL_ABRE_PAR) { 

            consome(DEL_ABRE_PAR); analisa_lista_args(); consome(DEL_FECHA_PAR);

        }

    } else if (lookahead.nome == DEL_ABRE_PAR) {

        consome(DEL_ABRE_PAR);

        analisa_expressao(tipo_esq, lugar_esq);

        consome(DEL_FECHA_PAR);

    } else if (lookahead.nome == KW_LEN) {  //1. Suporte sintático à palavra reservada len()

        consome(KW_LEN);

        consome(DEL_ABRE_PAR);

        char tipo_arg[7], lugar_arg[100];

        analisa_expressao(tipo_arg, lugar_arg); 

        consome(DEL_FECHA_PAR);

        strcpy(tipo_esq, "INT"); //1. Semântica: define que o retorno de len() é sempre inteiro (INT)

        

        //6. Geração TAC para função embutida len()

        sprintf(lugar_esq, "t%d", contador_temp++);

        fprintf(arquivo_codigo, "  %s = LEN(%s)\n", lugar_esq, lugar_arg);

    } else if (lookahead.nome == DEL_ABRE_COL) { //3. Suporte sintático a listas de inteiros [1, 2, 3]

        consome(DEL_ABRE_COL);

        if (lookahead.nome != DEL_FECHA_COL) {

            char tipo_elem[7], lugar_elem[100];

            analisa_expressao(tipo_elem, lugar_elem);

            while (lookahead.nome == DEL_VIRGULA) { //2. Usa o token de vírgula para separar elementos

                consome(DEL_VIRGULA);

                analisa_expressao(tipo_elem, lugar_elem);

            }

        }

        consome(DEL_FECHA_COL);

        strcpy(tipo_esq, "LIST"); //3. Semântica: define que esta expressão cria um tipo LIST

        

        //6. Geração TAC para criação de lista

        sprintf(lugar_esq, "t%d", contador_temp++);

        fprintf(arquivo_codigo, "  %s = MAKE_LIST\n", lugar_esq); 

    } else {

        erro_sintatico(IDENTIFICADOR, lookahead.nome); 

    }



    strcpy(tipo_retorno, tipo_esq);

    strcpy(lugar_retorno, lugar_esq);



    //4. Adicionados os operadores matemáticos (+, -, *, /, %) no loop de expressões

    while (lookahead.nome == OP_MAIOR || lookahead.nome == OP_MENOR || lookahead.nome == OP_MENORIGUAL ||

           lookahead.nome == OP_MAIORIGUAL || lookahead.nome == OP_IGUAL || lookahead.nome == OP_SOMA || 

           lookahead.nome == OP_SUB || lookahead.nome == OP_MULT || lookahead.nome == OP_DIV || lookahead.nome == OP_MOD) {

        

        TAtomo op = lookahead.nome;

        consome(op);

        analisa_expressao(tipo_dir, lugar_dir); 

        

        //5. Checagem Semântica Estrita: verifica se as operações matemáticas estão recebendo números inteiros

        if (op == OP_SOMA || op == OP_SUB || op == OP_MULT || op == OP_DIV || op == OP_MOD) {

            if (strcmp(tipo_retorno, "INT") != 0 || strcmp(tipo_dir, "INT") != 0) {

                //5. Dispara erro se tentar fazer matemática com booleanos, listas, etc.

                erro_semantico("Operacao aritmetica com tipos incompativeis. Esperado INT.");

            }

            strcpy(tipo_retorno, "INT");

        } else {

            if (strcmp(tipo_retorno, tipo_dir) != 0) {

                erro_semantico("Comparacao entre tipos diferentes.");

            }

            strcpy(tipo_retorno, "BOOL");

        }

        

        //6. Lógica perfeita para Código de Três Endereços: t0 = op1 + op2

        char operador[4] = "";

        switch (op) {

            case OP_SOMA: strcpy(operador, "+"); break;

            case OP_SUB: strcpy(operador, "-"); break;

            case OP_MULT: strcpy(operador, "*"); break;

            case OP_DIV: strcpy(operador, "/"); break;

            case OP_MOD: strcpy(operador, "%"); break;

            case OP_MAIOR: strcpy(operador, ">"); break;

            case OP_MENOR: strcpy(operador, "<"); break;

            case OP_MAIORIGUAL: strcpy(operador, ">="); break;

            case OP_MENORIGUAL: strcpy(operador, "<="); break;

            case OP_IGUAL: strcpy(operador, "=="); break;

            default: break;

        }



        char nova_temp[100];

        sprintf(nova_temp, "t%d", contador_temp++);

        fprintf(arquivo_codigo, "  %s = %s %s %s\n", nova_temp, lugar_retorno, operador, lugar_dir); 

        strcpy(lugar_retorno, nova_temp); // O resultado desta iteração vira o operando da próxima

    }

}



void analisa_comando() {

    if (lookahead.nome == KW_IF) {

        int rotulo_fim = proximo_rotulo();

        int rotulo_proximo = proximo_rotulo();



        consome(KW_IF);

        char tipo_expr[7], lugar_expr[100];

        analisa_expressao(tipo_expr, lugar_expr); 

        

        if (strcmp(tipo_expr, "BOOL") != 0) erro_semantico("A condicao do 'if' deve ser BOOL.");

        

        //6. TAC com controle limpo de labels

        fprintf(arquivo_codigo, "  IF_FALSE %s GOTO L%d\n", lugar_expr, rotulo_proximo);

        

        consome(DEL_DOIS_PONTOS);

        analisa_comando();

        

        fprintf(arquivo_codigo, "  GOTO L%d\n", rotulo_fim);

        fprintf(arquivo_codigo, "L%d:\n", rotulo_proximo);

        

        while (lookahead.nome == KW_ELIF) {

            rotulo_proximo = proximo_rotulo();

            consome(KW_ELIF);

            analisa_expressao(tipo_expr, lugar_expr);

            

            if (strcmp(tipo_expr, "BOOL") != 0) erro_semantico("A condicao do 'elif' deve ser BOOL.");

            

            fprintf(arquivo_codigo, "  IF_FALSE %s GOTO L%d\n", lugar_expr, rotulo_proximo);

            

            consome(DEL_DOIS_PONTOS);

            analisa_comando();

            

            fprintf(arquivo_codigo, "  GOTO L%d\n", rotulo_fim);

            fprintf(arquivo_codigo, "L%d:\n", rotulo_proximo);

        }



        if (lookahead.nome == KW_ELSE) {

            consome(KW_ELSE);

            consome(DEL_DOIS_PONTOS);

            analisa_comando();

        }

        fprintf(arquivo_codigo, "L%d:\n", rotulo_fim);



    } else if (lookahead.nome == KW_WHILE) {

        int rotulo_inicio = proximo_rotulo();

        int rotulo_fim = proximo_rotulo();



        fprintf(arquivo_codigo, "L%d:\n", rotulo_inicio);

        consome(KW_WHILE);

        char tipo_expr[7], lugar_expr[100];

        analisa_expressao(tipo_expr, lugar_expr);

        

        if (strcmp(tipo_expr, "BOOL") != 0) erro_semantico("A condicao do 'while' deve ser BOOL.");

        

        fprintf(arquivo_codigo, "  IF_FALSE %s GOTO L%d\n", lugar_expr, rotulo_fim);

        

        consome(DEL_DOIS_PONTOS);

        analisa_comando();

        

        fprintf(arquivo_codigo, "  GOTO L%d\n", rotulo_inicio);

        fprintf(arquivo_codigo, "L%d:\n", rotulo_fim);



    } else if (lookahead.nome == KW_PRINT) { 

        consome(KW_PRINT);

        consome(DEL_ABRE_PAR);

        if (lookahead.nome != DEL_FECHA_PAR) {

            char tipo_expr[7], lugar_expr[100];

            analisa_expressao(tipo_expr, lugar_expr); 

            fprintf(arquivo_codigo, "  PRINT %s\n", lugar_expr); //6. Imprime via TAC

            

            //2. Suporte sintático para imprimir várias coisas de uma vez separadas por vírgula

            while (lookahead.nome == DEL_VIRGULA) {

                consome(DEL_VIRGULA); 

                analisa_expressao(tipo_expr, lugar_expr); 

                fprintf(arquivo_codigo, "  PRINT %s\n", lugar_expr); //6. Imprime via TAC

            }

        }

        consome(DEL_FECHA_PAR);



    } else if (lookahead.nome == IDENTIFICADOR) {

        char nome_var[100];

        strcpy(nome_var, lookahead.atributo);

        consome(IDENTIFICADOR);

        

        if (lookahead.nome == OP_ATRIB) { 

            consome(OP_ATRIB);

            char tipo_expr[7], lugar_expr[100];

            analisa_expressao(tipo_expr, lugar_expr); 



            TNo *no = obter_no_tabela(nome_var);

            if (no == NULL) {

                inserir_tabela_simbolos(nome_var, tipo_expr);

            } else {

                if (strcmp(no->tipo, tipo_expr) != 0) {

                    char msg[256]; 

                    snprintf(msg, sizeof(msg), "Incompatibilidade de tipo. Variavel '%s' e '%s', tentou receber '%s'.", 

                            nome_var, no->tipo, tipo_expr);

                    erro_semantico(msg);

                }

            }

            //6. Atribuição final usando TAC (ex: number1 = t3)

            fprintf(arquivo_codigo, "  %s = %s\n", nome_var, lugar_expr);

        } else {

             erro_sintatico(OP_ATRIB, lookahead.nome);

        }

    } else {

        lookahead = obter_atomo(); 

    }

}



// MAIN



int main(int argc, char *argv[]) {

    if (argc < 2) {

        printf("Uso: %s <arquivo_fonte.py>\n", argv[0]);

        return EXIT_FAILURE;

    }

  

    arquivo_fonte = fopen(argv[1], "r");

    if (!arquivo_fonte) {

        printf("Erro ao abrir o arquivo %s\n", argv[1]);

        return EXIT_FAILURE;

    }



    arquivo_saida = fopen("saida_tokens.txt", "w");

    arquivo_codigo = fopen("saida_codigo.txt", "w"); 



    if (!arquivo_saida || !arquivo_codigo) {

        printf("Erro ao criar os arquivos de saida.\n");

        encerrar_arquivos();

        return EXIT_FAILURE;

    }



    lookahead = obter_atomo();

    

    while (lookahead.nome != EOS) { 

        analisa_comando();

    }

    

    verificar_variaveis_nao_usadas();



    printf("\nAnalise concluida com sucesso! Sem erros.\n");

    printf("Tokens salvos em 'saida_tokens.txt'\n");

    printf("Codigo Intermediario salvo em 'saida_codigo.txt'\n");



    encerrar_arquivos();

    return EXIT_SUCCESS; 

} 
