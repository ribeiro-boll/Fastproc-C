#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>


typedef struct{
    char pid[9];
    char name[40];
    double memoria;
} Processos_struct;

long int convert_int(char* arr){
    long int retorno_nmr =0;
    for (int i = 0;i<strlen(arr);i++){
        switch (arr[i]) {
        case '0': retorno_nmr*=10;break;
        case '1':case '2':case '3':
        case '4':case '5':case '6':
        case '7':case '8':case '9':
            retorno_nmr = retorno_nmr*10 + (arr[i] - '0');break;
        default:break;
        }
    }
    return retorno_nmr;
}

void sort(Processos_struct *arr, long int total){
    if (total == 0){
        return;
    }
    double max = arr[0].memoria; 
    long int index = 0; 
    
    for (int i = 0; i<total;i++){
        if (arr[i].memoria >=max){
            max = arr[i].memoria;
            index = i;
        }
    }
    double memoria_temp = arr[total-1].memoria;
    char pid_temp[10], nome_atual_temp[41];
    strcpy(pid_temp,arr[total-1].pid);
    strcpy(nome_atual_temp, arr[total-1].name);
    nome_atual_temp[sizeof(nome_atual_temp)-1] = '\0';
    pid_temp[sizeof(pid_temp)-1] = '\0';
    //  variaveis temporarias para colar no index de maior memoria que sera substituido

    strcpy(arr[total-1].name, arr[index].name);
    strcpy(arr[total-1].pid, arr[index].pid);
    arr[total-1].name[sizeof(arr[total-1]).name-1] = '\0';
    arr[total-1].pid[sizeof(arr[total-1].pid)-1] = '\0';
    arr[total-1].memoria = arr[index].memoria;
    // substituindo as infos no ultimo index com as infos do index de maior memoria

    strcpy(arr[index].name, nome_atual_temp);
    strcpy(arr[index].pid, pid_temp);
    arr[index].name[sizeof(arr[index].name)-1] = '\0';
    arr[index].pid[sizeof(arr[index].pid)-1] = '\0';
    arr[index].memoria = memoria_temp;
    sort(arr, total-1); 
}

Processos_struct *processos = NULL;
long int total_processos = 0;
double total=0;
void listar_processos(char*path){
    double memoria=0;
    char memoria_temporaria[1024];
    char name_processo[40];
    DIR *diretorio = opendir(path);
    if (diretorio == NULL) {
        return;
    }
    struct dirent *diretorio_atual;
    while ((diretorio_atual = readdir(diretorio)) != NULL){
        if (strcmp(diretorio_atual->d_name, ".")==0||strcmp(diretorio_atual->d_name,"..")==0||strcmp(diretorio_atual->d_name,"task")==0||strcmp(diretorio_atual->d_name,"asound")==0||strcmp(diretorio_atual->d_name,"sys")==0){
            continue;
        }
        else if(diretorio_atual->d_type == DT_DIR){
            char new_path[1024];
            snprintf(new_path, sizeof(new_path),"%s/%s",path,diretorio_atual->d_name);
            listar_processos(new_path);
        }
        else if (strcmp(diretorio_atual->d_name, "status") == 0) {
            char linha_arq_status[1024];
            char status_path[1024];
            snprintf(status_path, sizeof(status_path), "%s/%s", path, diretorio_atual->d_name);
            FILE *arquivo_status = fopen(status_path, "r");

            if (arquivo_status == NULL) {
                perror("Erro ao abrir o arquivo status");
                return;
            }
            while (fgets(linha_arq_status, sizeof(linha_arq_status), arquivo_status) != NULL) {
                if (strncmp(linha_arq_status, "Name:", 5) == 0) {
                    char *temp_name = linha_arq_status+5;
                    strcpy(name_processo, temp_name);
                }
                else if (strncmp(linha_arq_status, "VmRSS:", 6) == 0) {
                    char *temp_memory = linha_arq_status+6;
                    strcpy(memoria_temporaria, temp_memory);
                    sscanf(memoria_temporaria, "%lf",&memoria);
                    memoria = memoria/1024;
                    total+=memoria;
                }
            }
        for (int i = 0;i<(int)strlen(name_processo);i++){
            if (name_processo[i] == '\n' || name_processo[i] == ' '){
                name_processo[i] = '\0';
            }
        }
        for (int i = strlen(name_processo);i<41;i++) {
            name_processo[i] = ' ';
        }
        fclose(arquivo_status);
        char*temp = path+5;
        char pid[9] ={' '};
        int j=0;
        for (int i =0;i<9;i++){
            pid[i] = ' ';
        }
        for (int i=strlen(temp);0<i;i--){
            pid[8-j] = temp[i];
            j++;
        }
        total_processos++;
        processos = (Processos_struct* ) realloc(processos,total_processos*sizeof(Processos_struct));
        if (processos == NULL){
            free(processos);
            printf("deu pane");
        }
        strcpy(processos[total_processos-1].pid, pid);
        strcpy(processos[total_processos-1].name, name_processo);
        processos[total_processos-1].memoria = memoria;
        }
    }
    closedir(diretorio);
}

int main(){
    char linha_texto[1024];
    double mem1, mem2;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    int max_linhas = LINES - 16;  // Linhas disponíveis para a lista
    WINDOW *win_mem_total = newwin(7, COLS, 0, 0); // 3 linhas no topo
    WINDOW *win = newwin(max_linhas, COLS, 5, 0); // Restante abaixo
    WINDOW *win_info = newwin(7, COLS, LINES - 14+4, 0);
    int selected = 0;
    int inicio = -2;
    int ch;
    
    while (1) {
        flushinp();
        curs_set(0); 
        // Atualiza a listagem de processos
        listar_processos("/proc");
        long processos_ativos = total_processos;
        if(processos_ativos > 0){
            sort(processos, total_processos);
        }
        // Leitura de meminfo
        FILE *meminfo = fopen("/proc/meminfo", "r");
        if(meminfo) {
            while (fgets(linha_texto, sizeof(linha_texto), meminfo) != NULL){
                if (strncmp(linha_texto, "MemAvailable:", 13) == 0){
                    sscanf(linha_texto + 14, "%lf", &mem2);
                } 
                else if (strncmp(linha_texto, "MemTotal:", 9) == 0){
                    sscanf(linha_texto + 10, "%lf", &mem1);
                }
            }
            fclose(meminfo);
        }
        double memoria_total_pc = mem1/(1024*1024);
        double memoria_usada_atual = ((mem1-mem2)/(1024*1024));
        
        // Ajusta índices
        if (selected < inicio+2) {
            selected = inicio+2;
        }
        if (selected >= total_processos) {
            selected = total_processos - 1;
        }
        int index_max = total_processos-1;
        // Atualiza somente a subjanela 'win'
        werase(win);
        werase(win_mem_total);
        werase(win_info);
        mvwprintw(win_mem_total,0,0,"'||''''|                  .                                          ..|'''.  ");
        mvwprintw(win_mem_total,1,0," ||  .    ....    ....  .||.  ... ...  ... ..    ...     ....      .|'     '  ");
        mvwprintw(win_mem_total,2,0," ||''|   '' .||  ||. '   ||    ||'  ||  ||' '' .|  '|. .|   '' --- ||         ");
        mvwprintw(win_mem_total,3,0," ||      .|' ||  . '|..  ||    ||    |  ||     ||   || ||      --- '|.      . ");
        mvwprintw(win_mem_total,4,0,".||.     '|..'|' |'..|'  '|.'  ||...'  .||.     '|..|'  '|...'      ''|....'  ");
        mvwprintw(win_mem_total,5,0,"                              _||_                                            ");
        mvwprintw(win_mem_total,6,0,"___________________________________________________________________________________");
        for (int i = 0; i < max_linhas && (inicio + i) < total_processos; i++){
            int index = inicio + i;
            if (index == selected) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, i, 0, "PID: %s  Name: %s   Memory: %4.2lf MB", 
                processos[index_max-index].pid, 
                processos[index_max-index].name, 
                processos[index_max-index].memoria
            );
            if (index == selected) {
                wattroff(win, A_REVERSE);
            }
        }
        mvwprintw(win_info,0,0,"Total usable RAM avaliable in this pc: %.2lf GB.", memoria_total_pc);
        mvwprintw(win_info,1,0,"Aproximated RAM usage from this pc: %.2lf GB.", memoria_usada_atual);
        mvwprintw(win_info,3,0, "->Press 'w' or 'W' or 'Arrow Up' to move up a process.\n->Press 's' or 'S' 'Arrow Down' to move down a process.\n->Press 'k' or 'K' to close the selected task.\n->Press 'q' or 'Q' to exit the program.");
        wnoutrefresh(win);
        wnoutrefresh(win_mem_total);
        wnoutrefresh(win_info);
        doupdate();
        ch = getch();
        if (ch == 'w'|| ch =='W' || ch == KEY_UP) {
            if (selected > 0) {
                selected--;
                if (selected < inicio+2)
                    inicio--;
                    
            }
        }
        else if (ch == 's'|| ch == 'S' || ch == KEY_DOWN) {
            if (selected < total_processos - 1) {
                selected++;
                if (selected >= inicio + max_linhas)
                    inicio++;
            }
        }
        else if (ch == 'k'|| ch == 'K'){
            long codigo = convert_int(processos[total_processos-selected-1].pid);
            kill(codigo, SIGTERM);
        }
        else if (ch == 'q' || ch == 'Q') {
            break;  // Sai do loop se 'q' for pressionado
        }
        // Libera os dados para a próxima iteração
        free(processos);
        processos = NULL;
        total_processos = 0;
        total = 0;
        refresh();
    }
    delwin(win);
    delwin(win_mem_total);
    delwin(win_info);
    endwin();
    return 0;
}
