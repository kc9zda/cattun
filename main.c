#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

static int tunfd;

void sig_handle(int signo) {
	if (signo == SIGINT) {
		if (tunfd < 0) close(tunfd);
		tunfd = -1;
		}
	}

int tun_alloc(char *dev)
{
        struct ifreq ifr;
        int fd, err;

        if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ) return fd;

        memset(&ifr, 0, sizeof(ifr));

        /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
        *        IFF_TAP   - TAP device  
        *
        *        IFF_NO_PI - Do not provide packet information  
        */ 
        ifr.ifr_flags = IFF_TUN; 
        if( *dev ) strncpy(ifr.ifr_name, dev, IFNAMSIZ);

        if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
                close(fd);
                return err;
        }
        strcpy(dev, ifr.ifr_name);
        return fd;
}


int main(int argc, char **argv) {
	char buf[1500];
	int nread = 0;
	int szfr;

	if (argc < 2) {
		printf("usage: cattun <tun dev name>\n");
		return;
		}
	tunfd = tun_alloc(argv[1]);
	if (tunfd < 0) {
		printf("error creating tun device\n");
		return;
		}
	if (signal(SIGINT, sig_handle) == SIG_ERR) {
		printf("error setting signal handler\n");
		return;
		}
	int flags = fcntl(tunfd, F_GETFL, 0);
	fcntl(tunfd, F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
	while (tunfd != -1) {
		nread = read(tunfd,buf,1500);
		if (nread > 0) {
			szfr = nread;
			write(STDOUT_FILENO, (void*)&szfr, sizeof(int));
			write(STDOUT_FILENO, &buf, nread);
			}
		nread = read(STDIN_FILENO, &szfr, sizeof(int));
		if (nread > 0) {
			read(STDIN_FILENO, &buf, szfr);
			write(tunfd, &buf, szfr);
			}
		}
	}
