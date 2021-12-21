#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<math.h>                       //编译参数加 -lm
#include<time.h>

#include"../include/syscompile.h"

#define RoundPI      3.141592653589793
#define FRPI           3.1415926535898
#define SWPTIME                     20
#define MAXSpeed                1600.0 //最大空气分子速率，单位m/s.
#define MASS     4.817275747508306e-26 //分子质量，单位kg.
#define KBLZM             1.380649e-23 //玻尔兹曼常数，单位 J/K

FILE  *fileinit, *fileruntime, *fileinit_t, *fileruntime_t;
double tempr,     mD2kT,   mD2PkT;
double airInitPoss[1601], airRTPoss[1601]  ;
double inSpeed,   airSpeed = 0.0, airSpeed2;
_int_64_t  molSizeInit = 0, molSizeRT = 0, molPerInit[1601], molPerRT[1601]; //请在64位Linux上编译!!!
_int_64_t  molSizeCount=0, i0=0, i0_0=0, i1=0, i1_0=0, i2=0, i2_0=0 , airSpin_t;
float speedOut=0.0, temprOut=0.0, i3=0.0;
int   airSpin[91], airCount = 0 ;

int main() {
	srand((unsigned)time(NULL));
	printf("输入风速值(m/s):");
	scanf("%lf", &inSpeed);
	speedOut = inSpeed;
	printf("输入温度值(K):");
	scanf("%lf", &tempr);
	temprOut = tempr  ;
	mD2kT  = MASS / (2 * KBLZM * tempr);
	mD2PkT = mD2kT / RoundPI;
	for (airCount = 0 ; airCount < 1601; airCount++) {
		airSpeed2 = airSpeed * airSpeed;
		airInitPoss[airCount]  = 4 * RoundPI * sqrt(mD2PkT) * mD2PkT * exp(-mD2kT * airSpeed2) * airSpeed;
		airRTPoss[airCount]    = airInitPoss[airCount] * airSpeed;
		airInitPoss[airCount] *= 5e9 ;								//放大数值以能正确平分
		molPerInit[airCount]   = round(airInitPoss[airCount]);      //分配速率会用到
		molSizeInit           += molPerInit[airCount];				//计算总大小, 下同
		airRTPoss[airCount]   *= 1e8 ;
		molPerRT[airCount]	   = round(airRTPoss[airCount]);
		molSizeRT             += molPerRT[airCount];
		airSpeed += 1.0;
	}
	printf("正在向操作系统申请内存...\n");
//	double airSpeed3[molSizeInit];栈溢出,原因是molSizeInit太大，内存栈撑不下.
//	double airSpeed4[molSizeRT]  ;同样栈溢出，改用动态分配。这时增加头文件引用<memory.h>(好像不要?)
	float *airSpeed3, *airSpeed4;
	airSpeed3 = (float*)malloc(sizeof(float)*(molSizeInit+1));
	airSpeed4 = (float*)malloc(sizeof(float)*(molSizeRT+1));   //通过malloc向系统申请内存 
	printf("正在分配速率...\n");
	for(airCount=1;airCount<1601;airCount++){
	    if(molPerInit[airCount]==0){
	        break;
	    }
		float tempx=1.0/molPerInit[airCount];
		for(_int_64_t i=0;i<molPerInit[airCount];i++){
			airSpeed3[molSizeCount+1]=airSpeed3[molSizeCount]+tempx;
			molSizeCount++;
		}
	}molSizeCount=0;
	for(airCount=1;airCount<1601;airCount++){
	    if(molPerInit[airCount]==0){
	        break;
	    }
		float tempx=1.0/molPerRT[airCount];
		for(_int_64_t i=0;i<molPerRT[airCount];i++){
			airSpeed4[molSizeCount+1]=airSpeed4[molSizeCount]+ tempx;
			molSizeCount++;
		}
	}
	printf("已生成%ld+%ld=%ld组数据.\n",molSizeInit,molSizeRT, molSizeInit+molSizeRT);
	printf("正在分配速度方向...\n");
	for(int theta=0;theta<=90;theta++){
		airSpin[theta] = roundf(128*RoundPI*sinf(theta*FRPI/180.0));
		airSpin_t+= airSpin[theta];
	}
	printf("正在向操作系统申请内存(第二阶段)...\n");
	float *airSPossx, *airSPossy, *airSPossz; 
	airSPossx = (float*)malloc(sizeof(float)*(airSpin_t+1));
	airSPossy = (float*)malloc(sizeof(float)*(airSpin_t+1));
	airSPossz = (float*)malloc(sizeof(float)*(airSpin_t+1));
	molSizeCount=0;
	for(int theta=0;theta<=90;theta++){
		float deltaphi=2*FRPI/airSpin[theta];
		float phi=0.0,    aircc=sinf(theta*FRPI/180);
		airSPossz[molSizeCount]=cosf(theta*FRPI/180);//sinf就是float版的sin
		for(int iphi=0;iphi<airSpin[theta];iphi++){
			airSPossx[molSizeCount]   = aircc * sinf(phi);
			airSPossy[molSizeCount]   = aircc * cosf(phi);
			airSPossz[molSizeCount+1] = airSPossz[molSizeCount];
			phi += deltaphi;
			molSizeCount++;
		}
	}
	printf("已生成%ld组数据.\n",airSpin_t);
	printf("正在写入文件...\n");
	fileinit    = fopen("./vxyz_init.wvp", "w+b");//然后当测试程序在Linux上创建WVP文件时，Linux识别该扩展名为另外一种文件... 
	fileruntime = fopen("./vxyz_RT.wvr"  , "w+b");//好吧我无语了...... 
	fwrite("WVP" , 4, 1, fileinit);
	fwrite(&temprOut,    sizeof(float), 1, fileinit);
	fwrite(&speedOut,    sizeof(float), 1, fileinit);
	fwrite(&molSizeInit, sizeof(_int_64_u), 1 ,  fileinit);
	fwrite(&airSpin_t  , sizeof(_int_64_u), 1 ,  fileinit);
	fwrite("WVR" , 4, 1, fileruntime);
	fwrite(&temprOut,    sizeof(float), 1,fileruntime);
	fwrite(&speedOut,    sizeof(float), 1,fileruntime);
	fwrite(&molSizeRT,   sizeof(_int_64_u), 1,fileruntime);
	for (_int_64_t fileCount = 0; fileCount < molSizeInit; fileCount++) {
		fwrite(&airSpeed3[fileCount], 4, 1, fileinit);
	}
	for (_int_64_t fileCount = 0; fileCount < molSizeRT  ; fileCount++) {
		fwrite(&airSpeed4[fileCount], 4, 1, fileruntime);
	}
	for (_int_64_t fileCount = 0; fileCount < airSpin_t; fileCount++) {
		fwrite(&airSPossx[fileCount], sizeof(float),1,fileinit);
		fwrite(&airSPossy[fileCount], sizeof(float),1,fileinit);
		fwrite(&airSPossz[fileCount], sizeof(float),1,fileinit);
	}
	fclose(fileinit);
	fclose(fileruntime);
	free(airSpeed3);
	free(airSpeed4);
	free(airSPossx);
	free(airSPossy);
	free(airSPossz);
	printf("完毕.\n请复制程序所在目录下的\"*.wvp, *.wvr\"文件。\n");
	return 0;
}
