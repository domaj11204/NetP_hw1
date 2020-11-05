//關於PNG檔的CRC問題https://zenhack.it/challenges/2017/05/30/pngcrc/
//gcc -I../lib -g -O2 -D_REENTRANT -w t_serv01.c -o t_serv01 ../libunp.a -lpthread
#include	"unp.h"

char http_resp[] = {"HTTP/1.1 200 OK\r\nContent-Type: "};

int find_size(char buf[]){
    char tmp_buf[MAXLINE];//strtok用
    strcpy(tmp_buf,buf);
    char* len = strstr(tmp_buf,"Content-Length: ");
    char* tmp;
    tmp = strtok(len+16,"\r\n");
    return (atoi(tmp));
    /*char* id = strstr(tmp_buf,"WebKitFormBoundary");
    //printf("POST1\n%s\n",tmp_buf);
    //printf("\nsize_all:%d\n",size_all);
    //tmp = strtok(id,"\r\n");
    //char ID[50];
    //strcpy(ID,tmp);
    //printf("id:\n%s\n",ID);
    debug用*/
}

int find_name(char buf[], char filename[]){
    char tmp_buf[MAXLINE];
    strcpy(tmp_buf,buf);
    char* tmp = strstr(tmp_buf,"filename=\"");
    if(tmp == NULL){
        printf("find name error\n");
        return -1;
    }
    tmp = strtok(tmp,"\r\n");
    strcpy(filename,tmp+10);
    filename[strlen(filename)-1] = '\0';
    //printf("filename = %s\n",filename);
    return 0;
}

void send_test(int sockfd)
{
	ssize_t		n;
	char		buf[MAXLINE],outbuf[MAXLINE];
    char ipstr[INET6_ADDRSTRLEN];
    long ret;
    int fild_fd;
    int port;
    //getpeername -> ip/port
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    getpeername(sockfd, (struct sockaddr*)&addr, &len);
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof (ipstr));
  
    long size_now_buf =read(sockfd,buf,MAXLINE);
    printf("%s:%d------>serv     %ld bytes\n",ipstr,port,size_now_buf);
    
    if((strncmp(buf,"GET /i",6)) == 0){//若符合
        sprintf(outbuf,"%s%s\r\n\r\n",http_resp,"image/jpg");
        fild_fd = open("./html/image/tte.jpg",O_RDONLY);
    }
    else if((strncmp(buf,"GET / ",6)) == 0)
    {
        sprintf(outbuf,"%s%s\r\n\r\n",http_resp,"text/html");
        fild_fd = open("./html/index.html",O_RDONLY);
    }
    else if((strncmp(buf,"GET /upload.html",16)) == 0){
        sprintf(outbuf,"%s%s\r\n\r\n",http_resp,"text/html");
        fild_fd = open("./html/upload.html",O_RDONLY);
    }
    else if((strncmp(buf,"GET /favicon.ico",16)) == 0){
        sprintf(outbuf,"%s%s\r\n\r\n",http_resp,"image/jpg");
        fild_fd = open("./html/image/tte.jpg",O_RDONLY);
    }
    else if((strncmp(buf,"POST",4)) == 0){
        int size_all = find_size(buf);
        char buf_POST[MAXLINE];
        long size_output;
        char* tmp = strstr(buf,"\r\n\r\n-");
        char* post_start;
        if(tmp!= NULL){//找到第一個檔案的檔案頭
            post_start = tmp+4;
        }
        else//第一個buf沒了 讀下一個buf
        {
            size_now_buf = read(sockfd,buf,MAXLINE);
            printf("%s:%d------>serv     %ld bytes\n",ipstr,port,size_now_buf);
            buf[size_now_buf-1] = '\0';
            post_start = buf;
        }
        strcpy(buf_POST,post_start);//從buf移到 buf_POST
        //找到檔案起始點-------------------
        tmp = strstr(post_start,"\r\n")+2;
        for(int i = 0 ; i < 3 ; i++){
            tmp = strstr(tmp,"\r\n")+2;
        }
        char* file_start = tmp;
        //printf("file_start:\n%s",file_start);
        //------------------------
        int size_head = (int)(file_start - post_start);//buf前段的非檔部份
        printf("size_now_buf = size_now_buf - size_head = %d  - %d\n",size_now_buf, size_head);
        size_now_buf = size_now_buf - size_head;//該buf剩下要讀的大小
        
        char filename[50];
        find_name(post_start,filename);
        printf("%s:%d------>serv     %s\n",ipstr,port,filename);
        //printf("\nfilename=%s\n",filename);
        char file_PATH[100];
        //依照IP設檔案路徑～
        sprintf(file_PATH,"./file/%s",filename);//設定檔案路徑
        FILE* output_fd = fopen(file_PATH,"wb+");
        int size_file = size_all - size_head - 46;//檔案大小=傳輸大小-頭-尾
        int size_now;  //目前讀完的檔案
        if(size_now_buf >= size_file){//檔案只用一個buf
            size_output = fwrite(file_start,1,size_file,output_fd);
            printf("file_size %d/%d bytes\n",size_output,size_file);
            size_now = size_file;
        }
        else{
            size_output = fwrite(file_start,1,size_now_buf,output_fd);
            printf("file_size %d/%d bytes\n",size_output,size_file); 
            size_now =  size_output;
        }

        int size_next = (size_file-size_now >= MAXLINE)?MAXLINE:size_file-size_now;
        while (size_now<size_file)
        {   
            if(size_next < MAXLINE)
                printf("read%dbytes....\n",size_next);
            int S = read(sockfd,buf_POST,size_next);
            //if(size_next < MAXLINE)
            //    printf("read%dbytes end\n",size_next);
            printf("read size:%d/%d\n",size_now,size_file);
            n = fwrite(buf_POST,1, size_next,output_fd);
            //printf("wirte to output: %ld\n",n);    
            size_now += size_next;
            size_next = (size_file-size_now >= MAXLINE)?MAXLINE:size_file-size_now;
            if(size_now>=size_file)
                break;
        }
        fclose(output_fd);
        printf("Transfer end\nsocket_size= %d\nsize_now/size_file = %d/%d\n",size_all, size_now,size_file);
        sprintf(outbuf,"HTTP/1.1 200 OK\r\n\r\n");
    }
    else{
        sprintf(outbuf,"HTTP/1.1 404 NOOOOO\r\n");
    }
    Writen(sockfd, outbuf, strlen(outbuf));

	while ( (ret = read(fild_fd,outbuf,4096))>0){
        n = write(sockfd,outbuf,ret);
        //printf("write to sockfd: %ld\n",n);
    }
    printf("serv    ------>%s:%d   HTTP/1.1 200 OK\n",ipstr,port,size_now_buf);
}

void sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

int main(int argc, char **argv)
{
    int SERV_PORT2 = SERV_PORT;
    if(argc == 2){
        SERV_PORT2 = atoi(argv[1]);
    }
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT2);//9877

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);
    //char* ip;
    //inet_ntop(AF_INET,servaddrsin_addr ,ip,sizeof(servaddr.sin_addr));
    printf("Listen at Port %d.......\n",SERV_PORT2);
	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}

		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			//str_echo(connfd);	/* process the request */
			send_test(connfd); 
            exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}
}
