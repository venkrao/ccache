/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <signal.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}
#define CURL_STATICLIB

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}


int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char dest[256], download_url[1000];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);

    char *url;
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();

/////////////////
while (1) {
     newsockfd = accept(sockfd, 
           (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
         error("ERROR on accept");
     signal(SIGCHLD, SIG_IGN); // Otherwise, you end up with too many zombie,
	// that finally leads us to https://access.redhat.com/solutions/543503
	// Hence, handle the childs exit status https://www.win.tue.nl/~aeb/linux/lk/lk-5.html
     int pid = fork();
     if (pid < 0)
         error("ERROR on fork");
     if (pid == 0)  {
         close(sockfd);
//         printf("gotsome message dostuff(newsockfd)\n");
	 //bzero(dest,256);
	int n;
	uint32_t length, packet_size;
	packet_size = sizeof(int);
	int bytes_to_read;
	printf("size of int = %d\n", packet_size);
	 n = read(newsockfd, &bytes_to_read, sizeof(int));
         //if (n < 0) error("ERROR reading download_url from socket");
         if (n < 0) error("ERROR reading bytes_to_read for real message from socket");
	printf("============= read %d bytes\n", ntohl(bytes_to_read));
//////////// GREEN //////////////

         n = read(newsockfd, download_url, ntohl(bytes_to_read));
	printf(" ================= msg 1: %s\n", download_url);


	 n = read(newsockfd, &bytes_to_read, sizeof(int));
         if (n < 0) error("ERROR reading bytes_to_read for destination location from socket");

         n = read(newsockfd, dest, ntohl(bytes_to_read));
         if (n < 0) error("ERROR reading download destination from socket");
//	printf("dest=%s\n", dest);
//	printf("download_url = %s\n", download_url);

        fp = fopen(dest,"wb");
        if ( ! fp ) { 
           perror("failed to open destination file\n");
        }
        curl_easy_setopt(curl, CURLOPT_URL, download_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); 
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); 
        res = curl_easy_perform(curl);
	int result = 0;
	if (res != CURLE_OK) {
		result = 1;
	} else {
		printf("downloaded %s to %s\n", download_url, dest);
	}
        fclose(fp);
	uint32_t response = htonl(result);
	printf("result= %d\n", result);
        n = write(newsockfd, &response, sizeof(int));
        exit(0);
      } else close(newsockfd);

} 
      curl_easy_cleanup(curl);
}
