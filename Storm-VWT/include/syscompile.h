// syscompile.h

#ifndef _SYSCOMPILE_H_
#define _SYSCOMPILE_H_

#include<stdlib.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifdef _WIN32
    #define _int_64_u unsigned long long
    #define _int_64_t          long long
    _int_64_u randlu(void){
		_int_64_u randomsd[4]={0};
		randomsd[0]=rand();
		randomsd[1]=rand();
		randomsd[2]=rand();
		randomsd[3]=rand();
		randomsd[0]+=(randomsd[1]<<16)+(randomsd[2]<<32)+(randomsd[3]<<48);
		return randomsd[0];
	}
	_int_64_t randl(void){
		_int_64_t randomsd[4]={0};
		randomsd[0]=rand();
		randomsd[1]=rand();
		randomsd[2]=rand();
		randomsd[3]=rand();
		randomsd[0]+=(randomsd[1]<<16)+(randomsd[2]<<32)+(randomsd[3]<<48);
		return randomsd[0];
	}
	#define pause() system("pause")

#elif  __linux__
	#define _int_64_u unsigned      long
    #define _int_64_t               long
	_int_64_u randlu(void){
		_int_64_u randomsd[2]={0};
		randomsd[0]=rand();
		randomsd[1]=rand();
		randomsd[1]+=(randomsd[0]<<(8*sizeof(unsigned int)));
		return randomsd[1];
	}
	_int_64_t randl(void){
		_int_64_t randomsd[2]={0};
		randomsd[0]=rand();
		randomsd[1]=rand();
		randomsd[1]+=(randomsd[0]<<(8*sizeof(unsigned int)));
		return randomsd[1];
	}
	#include<stdio.h>
	#include<termios.h>
	#include<unistd.h>
	int pause(void){
		printf("请按任意键继续...\n");
		struct termios tm, tm_old;
		int fd = STDIN_FILENO,c;
		if (tcgetattr(fd, &tm) < 0){
			return -1;
		}
		tm_old = tm; cfmakeraw(&tm);
		if (tcsetattr(fd, TCSANOW, &tm) < 0){
			return -1;
		}
		c = fgetc(stdin);
		if (tcsetattr(fd,TCSANOW,&tm_old) < 0){
			return -1;
		}return c;
	}
#endif

#ifdef __cplusplus
}
#endif

#endif
