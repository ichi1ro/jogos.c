#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define DELIM_STR "|"
#define TRUE     1
#define FALSE    0
#define TAM_MAX_REG 256
#define TAM_STR 50
#define MAX_SOBRA 20

typedef struct list{
    int indice; 
    int prox; 
    int busca; 
    int ant;
}lista;

short leia_reg(char * buffer, int tam, FILE* arq){
    short comp_reg;

    if (fread(&comp_reg, sizeof(comp_reg), 1, arq) == 0) {
        return 0;
    }

    if (comp_reg < tam) {
        comp_reg = fread(buffer, sizeof(char), comp_reg, arq);
        buffer[comp_reg] = '\0';
        return comp_reg;
    } else {
        printf("Buffer overflow\n");
        return 0;
    }
}

void busca_reg(char* chave_busca, FILE *entrada, FILE *saida){ 
    int achou;
    short comp_reg;
    char buffer[TAM_MAX_REG];
    char *chave;
    char *campo;
    int byte_offset;

    achou = FALSE;
    fseek(entrada,4,SEEK_SET);
    comp_reg = leia_reg(buffer,TAM_MAX_REG,entrada);

    while(!achou && comp_reg > 0){
        chave = strtok(buffer,DELIM_STR);
        if(strcmp(chave_busca,chave)==0){
            achou = TRUE;
        }else{
            comp_reg = leia_reg(buffer,TAM_MAX_REG,entrada);
        }
    }
    if(achou){ 
        byte_offset = (ftell(entrada) - comp_reg) - 2;
        fprintf(saida, "Busca pelo registro de chave %s", chave);
        fprintf(saida, "\n%s|", chave);
        campo = strtok(NULL,DELIM_STR);
        while(campo!=NULL){
            fprintf(saida, "%s|", campo);
            campo = strtok(NULL,DELIM_STR);
        }
        fprintf(saida, "(%d bytes) ", comp_reg);
        fprintf(saida, "\nLocal: offset = %d bytes", byte_offset);
    }else{
        fprintf(saida, "Busca pelo registro de chave %s\nErro: registro nao encontrado!", chave_busca);

    }
    fprintf(saida,"\n\n");
}

void remove_reg(char* chave_busca, FILE *entrada, FILE *saida){
    int achou;
    short comp_reg;
    char buffer[TAM_MAX_REG];
    char *chave;
    short tam;

    lista *led;
    led = (lista *)malloc(sizeof(lista));

    achou = FALSE;
    fseek(entrada,4,SEEK_SET);
    comp_reg = leia_reg(buffer,TAM_MAX_REG,entrada);

    while(!achou && comp_reg > 0){
        chave = strtok(buffer,DELIM_STR);
        if(strcmp(chave_busca,chave)==0){
            achou = TRUE;
        }else{
            comp_reg = leia_reg(buffer,TAM_MAX_REG,entrada);
        }
    }
    
    if(achou){
        int i=-1;
        int byte_2remove = ftell(entrada) - comp_reg; 
        int loc_reg = (ftell(entrada) - comp_reg) - 2; 

        fseek(entrada,0,SEEK_SET);
        fread(&led->indice,sizeof(int),1,entrada);
        if(led->indice == 0){ // inicializa a led
            fwrite(&i,sizeof(i),1,entrada);
            fseek(entrada,0,SEEK_SET);
            fread(&(led->indice),sizeof(int),1,entrada);
        }   

        if(led->indice == -1){ // primeira remoçao
            fseek(entrada,0,SEEK_SET);
            fwrite(&loc_reg,sizeof(int),1,entrada);
            fseek(entrada,byte_2remove,SEEK_SET);
            fwrite("*",sizeof(char),1,entrada);
            fwrite(&(led->indice),sizeof(int),1,entrada);
            ///
            fprintf(saida,"Remocao do registro de chave %s", chave_busca);
            fprintf(saida,"\nRegistro removido! (%d bytes)", comp_reg);
            fprintf(saida,"\nLocal: offset = %d bytes", loc_reg);
        }else{
            fseek(entrada,led->indice,SEEK_SET);
            fread(&tam,sizeof(tam),1,entrada);
            fseek(entrada,led->indice+3,SEEK_SET);
            fread(&(led->prox),sizeof(led->prox),1,entrada);
            
            if(comp_reg <= tam){ 
                if(led->prox == -1){ // segunda remoçao
                    fseek(entrada,byte_2remove,SEEK_SET);
                    fwrite("*",sizeof(char),1,entrada);
                    fwrite(&(led->prox),sizeof(int),1,entrada);
                    fseek(entrada,led->indice+3,SEEK_SET);
                    fwrite(&loc_reg,sizeof(int),1,entrada);
                    ///
                    fprintf(saida,"Remocao do registro de chave %s", chave_busca);
                    fprintf(saida,"\nRegistro removido! (%d bytes)", comp_reg);
                    fprintf(saida,"\nLocal: offset = %d bytes", loc_reg);
                }else{ 
                    short tam2;
                    led->busca = led->prox;
                    led->ant = led->indice;
                    fseek(entrada,led->busca,SEEK_SET);
                    fread(&tam2,sizeof(tam2),1,entrada);

                    while(comp_reg <= tam2 && led->busca != -1){
                        fseek(entrada,-2,SEEK_CUR);
                        led->ant = ftell(entrada);
                        fseek(entrada,3,SEEK_CUR);
                        fread(&(led->busca),sizeof(led->busca),1,entrada);
                        if(led->busca == -1){
                            break;
                        }
                        fseek(entrada,led->busca,SEEK_SET);
                        fread(&tam2,sizeof(tam2),1,entrada);
                    }
                    if(led->busca == -1){
                        fseek(entrada,led->ant+3,SEEK_SET);
                        fwrite(&loc_reg,sizeof(loc_reg),1,entrada);
                        fseek(entrada,byte_2remove,SEEK_SET);
                        fwrite("*",sizeof(char),1,entrada);
                        fwrite(&i,sizeof(int),1,entrada);
                        ///
                        fprintf(saida,"Remocao do registro de chave %s", chave_busca);
                        fprintf(saida,"\nRegistro removido! (%d bytes)", comp_reg);
                        fprintf(saida,"\nLocal: offset = %d bytes", loc_reg);
                        }
                    if(comp_reg >= tam2){
                        fseek(entrada,led->ant+3,SEEK_SET);
                        fread(&(led->busca),sizeof(led->busca),1,entrada);
                        fseek(entrada,led->ant+3,SEEK_SET);
                        fwrite(&loc_reg,sizeof(loc_reg),1,entrada);
                        fseek(entrada,byte_2remove,SEEK_SET);
                        fwrite("*",sizeof(char),1,entrada);
                        fwrite(&(led->busca),sizeof(led->busca),1,entrada);
                        ///
                        fprintf(saida,"Remocao do registro de chave %s", chave_busca);
                        fprintf(saida,"\nRegistro removido! (%d bytes)", comp_reg);
                        fprintf(saida,"\nLocal: offset = %d bytes", loc_reg);
                        }
                    }
            }else if(comp_reg >= tam){
                fseek(entrada,0,SEEK_SET);
                fwrite(&loc_reg,sizeof(int),1,entrada);
                fseek(entrada,byte_2remove,SEEK_SET);
                fwrite("*",sizeof(char),1,entrada);
                fwrite(&(led->indice),sizeof(int),1,entrada);
                ///
                fprintf(saida,"Remocao do registro de chave %s", chave_busca);
                fprintf(saida,"\nRegistro removido! (%d bytes)", comp_reg);
                fprintf(saida,"\nLocal: offset = %d bytes", loc_reg);
            }
        }
    }else{
        fprintf(saida,"Remocao do registro de chave %s\nErro: registro nao encontrado!", chave_busca);
        
    }
    fprintf(saida,"\n\n");
    free(led);
}

void insere_reg(char* reg, FILE* entrada, FILE* saida){ 
    short tam,tam2,maior,sobra;
    int loc_sobra;
    lista *led;
    char *chave;
    char copy[TAM_MAX_REG];
    int offset;

    strcpy(copy,reg);
    chave = strtok(copy,DELIM_STR);

    led = (lista *)malloc(sizeof(lista));

    fseek(entrada,0,SEEK_SET);
    fread(&led->indice,sizeof(int),1,entrada);

    if(led->indice!=-1){
        fseek(entrada,led->indice,SEEK_SET);
        fread(&maior,sizeof(short),1,entrada);
    }
    tam = strlen(reg);

    if(led->indice==-1 || tam>maior){
        fseek(entrada,0,SEEK_END);
        fwrite(&tam,sizeof(tam),1,entrada);
        fwrite(reg,sizeof(char),tam,entrada);
        ///
        fprintf(saida,"Insercao do registro de chave %s (%d bytes)\n", chave,tam);
        fprintf(saida,"Local: Fim do arquivo");
    }else{
        fseek(entrada,led->indice+3,SEEK_SET);
        fread(&(led->prox),sizeof(int),1,entrada);
        fseek(entrada,0,SEEK_SET);
        fwrite(&(led->prox),sizeof(int),1,entrada);
        fseek(entrada,led->indice,SEEK_SET);
        offset = ftell(entrada);
        fseek(entrada,led->indice+2,SEEK_SET);
        fwrite(reg,sizeof(char),tam,entrada);
        loc_sobra = ftell(entrada);
        for(int i=0;i<maior-tam;i++){
            fwrite("\0",sizeof(char),1,entrada);
        }
        sobra = (maior - tam) -2;

        if(sobra > MAX_SOBRA){
            fseek(entrada,0,SEEK_SET);
            fread(&(led->prox),sizeof(int),1,entrada);
            fseek(entrada,led->prox,SEEK_SET);
            fread(&tam2,sizeof(short),1,entrada);
            if(sobra >= tam2){
                fseek(entrada,offset,SEEK_SET);
                fwrite(&tam,sizeof(short),1,entrada);
                fseek(entrada,loc_sobra,SEEK_SET);
                fwrite(&sobra,sizeof(short),1,entrada);
                fwrite("*",sizeof(char),1,entrada);
                fwrite(&(led->prox),sizeof(int),1,entrada);
                fseek(entrada,0,SEEK_SET);
                fwrite(&loc_sobra,sizeof(int),1,entrada);
            }else{
                led->indice = led->prox;
                fseek(entrada,led->prox+3,SEEK_SET);
                fread(&led->prox,sizeof(int),1,entrada);
                if(led->prox == -1){
                    fseek(entrada,offset,SEEK_SET);
                    fwrite(&tam,sizeof(short),1,entrada);
                    fseek(entrada,loc_sobra,SEEK_SET);
                    fwrite(&sobra,sizeof(short),1,entrada);
                    fwrite("*",sizeof(char),1,entrada);
                    fwrite(&(led->prox),sizeof(int),1,entrada);
                    fseek(entrada,led->indice+3,SEEK_SET); 
                    fwrite(&loc_sobra,sizeof(int),1,entrada);
                }else{
                    led->busca = led->indice+3;
                    led->ant = led->indice;
                    fseek(entrada,led->prox,SEEK_SET);
                    fread(&tam2,sizeof(short),1,entrada);
                    while(sobra <= tam2 && led->busca != -1){
                        fseek(entrada,-2,SEEK_CUR);
                        led->ant = ftell(entrada);
                        fseek(entrada,3,SEEK_CUR);
                        fread(&(led->busca),sizeof(led->busca),1,entrada);
                        if(led->busca == -1){
                            break;
                        }
                        fseek(entrada,led->busca,SEEK_SET);
                        fread(&tam2,sizeof(tam2),1,entrada);
                    }
                    if(led->busca == -1){
                        fseek(entrada,offset,SEEK_SET);
                        fwrite(&tam,sizeof(short),1,entrada);
                        fseek(entrada,led->ant+3,SEEK_SET);
                        fwrite(&loc_sobra,sizeof(int),1,entrada);
                        fseek(entrada,loc_sobra,SEEK_SET);
                        fwrite(&sobra,sizeof(short),1,entrada);
                        fwrite("*",sizeof(char),1,entrada);
                        fwrite(&(led->busca),sizeof(int),1,entrada);
                    }
                    if(sobra >= tam2){
                        fseek(entrada,offset,SEEK_SET);
                        fwrite(&tam,sizeof(short),1,entrada);
                        fseek(entrada,led->ant+3,SEEK_SET);
                        fread(&(led->busca),sizeof(int),1,entrada);
                        fseek(entrada,led->ant+3,SEEK_SET);
                        fwrite(&loc_sobra,sizeof(int),1,entrada);
                        fseek(entrada,loc_sobra,SEEK_SET);
                        fwrite(&sobra,sizeof(short),1,entrada);
                        fwrite("*",sizeof(char),1,entrada);
                        fwrite(&(led->busca),sizeof(int),1,entrada);
                    }
                }
            }
            fprintf(saida,"Insercao do registro de chave %s (%d bytes)\n", chave,tam);
            fprintf(saida,"Tamanho do espaco reutilizado: %d (Sobra de %d bytes)\n", maior, sobra);
            fprintf(saida,"Local: offset = %d bytes", offset);
        }else{
            fprintf(saida,"Insercao do registro de chave %s (%d bytes)\n", chave, tam);
            fprintf(saida,"Tamanho do espaco reutilizado: %d bytes\n", maior);
            fprintf(saida,"Local: offset = %d bytes", offset);
        }
    }
    fprintf(saida,"\n\n");
    free(led);

}

void imprime_led(FILE* arq){
    int offset,prox,i=1;
    short tam;
    fseek(arq,0,SEEK_SET);
    fread(&offset,sizeof(int),1,arq);
    if(offset==-1){
        printf("LED -> [offset: -1]");
        printf("\nTotal: 0 espacos disponiveis");
    }else{
        fseek(arq,offset,SEEK_SET);
        fread(&tam,sizeof(short),1,arq);
        fseek(arq,offset+3,SEEK_SET);
        fread(&prox,sizeof(int),1,arq);
        if(prox!=-1){
            printf("LED -> [offset: %d, tam: %d] -> ", offset, tam);
            while(prox!=-1){
                fseek(arq,prox,SEEK_SET);
                fread(&tam,sizeof(short),1,arq);
                printf("[offset: %d, tam: %d] -> ",prox, tam);
                fseek(arq,prox+3,SEEK_SET);
                fread(&prox,sizeof(int),1,arq);
                i++;
            }
            printf("[offset -1]");
            printf("\nTotal: %d espacos disponiveis", i);
        }else{
            printf("LED -> [offset: %d, tam: %d] -> [offset -1]", offset, tam);
            printf("\nTotal: 1 espaco disponivel");
        }
    }
    fclose(arq);
}

void executa_operacoes (char* nome_arq){
   FILE *entrada, *saida;
   FILE *dados;
   char str[TAM_MAX_REG]; //usar fgets para ler cada linha
   char c;
   char *i;
   
    if((dados=fopen("dados.dat","r+b")) == NULL){
        printf("Erro na abertura do arquivo de dados -- programa abortado\n");
        exit(EXIT_FAILURE);
    }

    if (((entrada = fopen(nome_arq, "r")) == NULL) || ((saida = fopen("output.txt", "a+")) == NULL)){
        printf("Erro na abertura dos arquivos -- programa abortado\n");
        exit(EXIT_FAILURE);
    }

    while ((c=fgetc(entrada))!=EOF){
        if(c == 'b'){
            fseek(entrada,1,SEEK_CUR);
            fgets(str,TAM_MAX_REG,entrada);
            i = strtok(str,"\n");
            busca_reg(i,dados,saida);
        }
        if(c == 'r'){
            fseek(entrada,1,SEEK_CUR);
            fgets(str,TAM_MAX_REG,entrada);
            i = strtok(str,"\n");
            remove_reg(i,dados,saida);
        }
        if(c == 'i'){
            fseek(entrada,1,SEEK_CUR);
            fgets(str,TAM_MAX_REG,entrada);
            i = strtok(str,"\n");
            insere_reg(i,dados,saida);
        }

    }
    fclose(entrada);
    fclose(saida);
    fclose(dados);

}

int main(int argc, char *argv[]) {
    FILE *dados;
    if((dados=fopen("dados.dat","rb")) == NULL){
        printf("Erro na abertura do arquivo de dados -- programa abortado\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 3 && strcmp(argv[1], "-e") == 0) {

        printf("Modo de execucao de operacoes ativado ... nome do arquivo = %s\n", argv[2]);
        // chamada da funcao que executa o arquivo de operacoes
        // o nome do arquivo de operacoes estara armazenado na variavel argv[2]
        // executa_operacoes(argv[2]);
        executa_operacoes(argv[2]);

    } else if (argc == 2 && strcmp(argv[1], "-p") == 0) {

        printf("Modo de impressao da LED ativado ...\n");
        // chamada da funcao que imprime as informacoes da led
        // imprime_led();
        imprime_led(dados);

    } else {

        fprintf(stderr, "Argumentos incorretos!\n");
        fprintf(stderr, "Modo de uso:\n");
        fprintf(stderr, "$ %s -e nome_arquivo\n", argv[0]);
        fprintf(stderr, "$ %s -p\n", argv[0]);
        exit(EXIT_FAILURE);

    }
    fclose(dados);

    return 0;
}
