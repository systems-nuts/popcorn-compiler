#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "config.h"
#include "dsm.h"
#include "communicate.h"
#include "migrate.h"
#include "pmparser.h"


#define CMD_EMBEDED_ARG_SIZE 64
//TODO: struct cmd_s must have the same size across archs
struct  __attribute__((__packed__)) cmd_s{
	enum comm_cmd cmd;
	uint32_t size;
	char arg[CMD_EMBEDED_ARG_SIZE];
};
	

static int server_sock_fd = 0;
static int ori_to_remote_sock = 0;

#define MAX_NUM_CHAR_SIZE 32
typedef int (*cmd_func_t) (char* arg, int size); /* Note arg is freed after the function call */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static ssize_t						 /* Write "n" bytes to a descriptor. */
__writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;   /* and call write() again */
			else
				return (-1);	/* error */
		 }

		 nleft -= nwritten;
		 ptr += nwritten;
	}
	return (n);
}
static ssize_t						 /* Write "n" bytes to a descriptor. */
writen(int fd, void *vptr, size_t n)
{
	//pthread_mutex_lock(&mutex);
	ssize_t ret=__writen(fd,vptr,n);
	//pthread_mutex_unlock(&mutex);
	return ret;
}

static ssize_t						 /* Read "n" bytes from a descriptor. */
__readn(int fd, void *vptr, size_t n)
{
	size_t  nleft;
	ssize_t nread;
	char   *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;	  /* and call read() again */
			else
				return (-1);
		} else if (nread == 0)
			break;			  /* EOF */

		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);		 /* return >= 0 */
}
static ssize_t						 /* Write "n" bytes to a descriptor. */
readn(int fd, void *vptr, size_t n)
{
	pthread_mutex_lock(&mutex);
	ssize_t ret=__readn(fd,vptr,n);
	pthread_mutex_unlock(&mutex);
	return ret;
}

int send_data(void* addr, size_t len)
{
	return writen(server_sock_fd, addr, len);
}



static int print_text(char* arg, int size)
{
	fwrite(arg, size, sizeof(char), stdout);
	return 0;
}

static int get_ctxt(char* arg, int size)
{
	void *ptr;
	int sz;

	ptr = NULL;
	sz=0;
	get_context(&ptr, &sz);
	up_log("%s: ptr = %p , size %d\n", __func__, ptr, size);
	writen(server_sock_fd, ptr, sz);
	return 0;
}


int hdl_exit(char* arg, int size)
{
	printf("Remote Exit\n");
	exit(0);
}


/* commands table */
cmd_func_t cmd_funcs[]  = {send_page, print_text, get_ctxt, send_pmap, hdl_exit};

int __handle_commands(int sockfd)
{
	int n;
	struct cmd_s cmds;

	up_log("Entering function %s\n", __func__);

	n = readn(sockfd, &cmds, sizeof(cmds));
	if(n<0)
		perror("cmd_read");
	up_log("%s: cmd %d; size %d\n", __func__, (int)cmds.cmd, cmds.size);

	char* arg;
	uint32_t size=cmds.size;
	if(size>0)
	{
		if(size >= CMD_EMBEDED_ARG_SIZE)
		{
			arg = pmalloc(size+1);
			n = readn(sockfd, arg, size);
			if(n<0)
				perror("arg_size");
			//arg[n]='\0';
			//up_log("%s: arg read is %s\n", __func__, arg);
			printf("arg written for %d size %d\n", cmds.cmd, size);
		}
		else
			arg=(char*)&cmds.arg;
	}else
		arg=NULL;

	cmd_funcs[cmds.cmd](arg, size);

	up_log("%s: cmd %d; handled\n", __func__, (int)cmds.cmd);

	if(size>0 && size >= CMD_EMBEDED_ARG_SIZE)
	//if(size >0)
		pfree(arg);

	return 0;
}

int handle_commands(int sockfd)
{
	up_log("Entering function %s\n", __func__);
	server_sock_fd = sockfd;
	while(1){
		__handle_commands(sockfd);
	}
}

int send_cmd(enum comm_cmd cmd, int size, char *arg)
{
	int n;
	struct cmd_s cmds;

	up_log("sending a command %d of size %d using socket %d\n", cmd, size, ori_to_remote_sock);
	cmds.cmd = cmd;
	cmds.size = size;
	if(size>0  && (size <= CMD_EMBEDED_ARG_SIZE))
		memcpy(&(cmds.arg), arg, size);

	n = writen(ori_to_remote_sock, &cmds, sizeof(cmds));
	if(n<0)
		perror("cmd_write");
	up_log("cmd written %d\n", cmds.cmd);

	if(size >= CMD_EMBEDED_ARG_SIZE)
	{
		n = writen(ori_to_remote_sock, arg, size);
		if(n<0)
			perror("arg_size write");
		up_log("arg written for %d size %d\n", cmds.cmd, size);
	}

	return 0;
}

/* send a cmd and wait for a response */
int send_cmd_rsp(enum comm_cmd cmd, int size, char *arg, int resp_size, void* resp)
{
	send_cmd(cmd, size, arg);

	int n = readn(ori_to_remote_sock, resp, resp_size);
	if(n<0)
		perror("resp_read");
	up_log("%s resp read\n", __func__);

	return 0;
}

int print_arch_suffix(char* buff, int max)
{
#ifdef __x86_64__
    	return snprintf(buff, max, "_%s", "x86-64");
#elif defined(__aarch64__)
    	return snprintf(buff, max, "_%s", "aarch64");
#endif
}

//TODO: don't always open a link see if one already exist?
// or do it in the origine_init funciton below
int comm_migrate(int nid)
{
	int sockfd = 0, n = 0;
	struct sockaddr_in serv_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		up_log("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(DEFAULT_PORT);

	if(inet_pton(AF_INET, arch_nodes[nid], &serv_addr.sin_addr)<=0)
	{
		perror("\n inet_pton error occured\n");
		return 1;
	}

	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
	   perror("\n Error : Connect Failed \n");
	   return 1;
	}

	/* get the path to the binary. TODO: consider the arch extension */
	/* should we use __progname?	*/
	int ps;
	char *path = pcalloc(PATH_MAX, 1);
	ps = readlink("/proc/self/exe", path, PATH_MAX);
	path[ps++] = '\0';
	up_log("path is %s, size %ld\n", path, strlen(path));

	/* add arch suffix */
	ps += print_arch_suffix(&path[ps], PATH_MAX - ps);
	ps++;//+1 for '\0
	if(ps==PATH_MAX)
		perror("path max");
	else
		up_log("suffixed path is %s, size with null %d\n", path, ps);

	/* Write path size */
	char path_size[NUM_LINE_SIZE_BUF+1];
	//snprintf(path_size, NUM_LINE_SIZE_BUF, "%."NUM_LINE_SIZE_BUF_STRING"d",ps);
	snprintf(path_size, NUM_LINE_SIZE_BUF+1, "%.8d",ps);
	up_log("path size ori %d %.9s\n", ps, path_size);
	n = writen(sockfd, path_size, NUM_LINE_SIZE_BUF);
	up_log("\n %d bytes written\n", n);
	/* Write the path */
	n = writen(sockfd, path, ps);
	up_log("\n %d bytes written\n", n);

	handle_commands(sockfd);

	//close(sockfd);//?
	return 0;
}

static void test(void)
{
	int ret;
        char msg[] = "Hello world from prog\n";
	printf("sending test hello\n");
        ret = send_cmd(PRINT_ST, strlen(msg), msg);
        if(ret < 0)
                perror(__func__);
}

static int remote_init()
{
        char *cfd = getenv("POPCORN_SOCK_FD");

        ori_to_remote_sock = atoi(cfd);

        up_log("%s: %d\n", __func__, ori_to_remote_sock);

	test();
        //close(fd);

        up_log("%s: end\n", __func__);

	return 0;
}

static int origin_init()
{
	return 0;
}

int comm_init(int remote)
{
	if(remote)
		remote_init();
	else
		origin_init();
	return 0;
}