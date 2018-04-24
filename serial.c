#include<stdio.h>   
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>
#include<termios.h>
#include<string.h>    

int setup_uart(int fd, int n_speed, int n_bits, char parity, int n_stop){
	struct termios newtio,oldtio;
	if( tcgetattr( fd,&oldtio) != 0){
		perror("init error");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
	switch( n_bits ){
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}
	switch( parity ){
		case 'O': 
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E': 
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
	}
	switch( n_speed ){
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
				cfsetispeed(&newtio, B115200);
				cfsetospeed(&newtio, B115200);
				break;
		case 230400:
				cfsetispeed(&newtio, B230400);
				cfsetospeed(&newtio, B230400);
				break;
		case 460800:
				cfsetispeed(&newtio, B460800);
				cfsetospeed(&newtio, B460800);
				break;
		case 921600:
				cfsetispeed(&newtio, B921600);
				cfsetospeed(&newtio, B921600);
				break;
		case 1152000:
				cfsetispeed(&newtio, B1152000);
				cfsetospeed(&newtio, B1152000);
				break;
		case 2000000:
				cfsetispeed(&newtio, B2000000);
				cfsetospeed(&newtio, B2000000);
				break;
		case 2500000:
				cfsetispeed(&newtio, B2500000);
				cfsetospeed(&newtio, B2500000);
				break;
		case 3500000:
				cfsetispeed(&newtio, B3500000);
				cfsetospeed(&newtio, B3500000);
				break;
		case 3000000:
				cfsetispeed(&newtio, B3000000);
				cfsetospeed(&newtio, B3000000);
				break;
		case 4000000:
				cfsetispeed(&newtio, B4000000);
				cfsetospeed(&newtio, B4000000);
				break;
		default:
				cfsetispeed(&newtio, B9600);
				cfsetospeed(&newtio, B9600);
				break;
	}
	if( n_stop == 1 ){
		newtio.c_cflag &= ~CSTOPB;
	}
	else if ( n_stop == 2 ){
		newtio.c_cflag |= CSTOPB;

	}
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	int flags = fcntl(fd, F_GETFL, 1);
	flags &= ~O_NONBLOCK;
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0){
		perror("com set error");
		return -1;
	}
	return 0;
}
int read_uart(int fd, char *rcv_buf, int buf_len, int timeo_us){
	fd_set fs_read;
	struct timeval time;    	
	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);    
	
	int ret;
	if(timeo_us < 0){
		ret = select(fd+1, &fs_read ,NULL, NULL, NULL); 
	}else{		    
		time.tv_sec = timeo_us/1000/1000;
		time.tv_usec = timeo_us%(1000*1000);
		ret = select(fd+1, &fs_read ,NULL, NULL, &time); 
	}
	
	if (ret < 0){
		printf("ret %d\n", ret);
		return -1;
	}else{
		if( FD_ISSET(fd, &fs_read) ){   
			int len = read(fd, rcv_buf, buf_len);
			if (len < 0){
				printf("ret len %d\n", len);
				return -1;
			}
			//test the connection
			if(len == 0 && ret == 1){
				int wlen = write(fd, "test", 0);
				if(wlen < 0){
					return -1;
				}
			}
			return len;			
	    }	   
	} 	
	return 0;
}
int read_uart_frame_aggregation(int fd, char *rcv_buf, int buf_len, int timeo_us, int aggr_us) {    
	fd_set fs_read;   
	struct timeval time; 

	FD_ZERO(&fs_read);
	FD_SET(fd,&fs_read);
	
	int ret;
	if(timeo_us < 0){
		ret = select(fd+1, &fs_read ,NULL, NULL, NULL); 
	}else{		
		time.tv_sec = timeo_us/1000/1000;
		time.tv_usec = timeo_us%(1000*1000);
		ret = select(fd+1, &fs_read ,NULL, NULL, &time); 
	}
	
	if (ret < 0){
		printf("ret %d\n", ret);
		return -1;
	}
	
	int currLen = 0;
	while(ret >= 0){
			
	    if( FD_ISSET(fd, &fs_read) ){
			int len = read(fd, rcv_buf+currLen, buf_len - currLen);
			if (len < 0){
				printf("ret len %d\n", len);
				return -1;
			}
			//test the connection
			if(len == 0 && ret == 1){
				int wlen = write(fd, "test", 0);
				if(wlen < 0){
					return -1;
				}
			}
			currLen += len;
			if( currLen >= buf_len){
				//reach the end of the buffer, return right now
				return buf_len;
			}
			
	    }
	    if(ret>0){
	        time.tv_sec = aggr_us/(1000*1000);    
	        time.tv_usec = aggr_us%(1000*1000);    
	        FD_ZERO(&fs_read);    
	        FD_SET(fd,&fs_read);    
	        ret = select(fd+1,&fs_read,NULL,NULL,&time);
			if (ret < 0){
				printf("ret %d\n", ret);
				return -1;
			}
	    }else{
			break;
	    }
	}
	
	return currLen;
}    

int write_uart(int fd, char *send_buf, int len){
	return write(fd, send_buf, len);
}
