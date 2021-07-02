//程序名称： Storm虚拟风洞 
//可执行文件名称：storm_vwt (Linux),  storm_vwt.exe(Windows)
//最近一次修改：13:01 2021/6/26

#ifndef _STORM_VWT_C_
#define _STORM_VWT_C_

#include<unistd.h>
#include<getopt.h>
#include<pthread.h> //加"-lpthread".此代码仅能在Linux上或Windows MinGW上编译.
#include<time.h>
#include<math.h>    //编译选项加"-lm"
#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
//注意，此程序的Windows版本需要同目录下的"libwinpthread-1.dll"才能执行。 

//自带库 
#include"../include/syscompile.h"
#include"storm_vwt.h"

//定义线程数
#define THNUM         16 

struct _Airmols{
	double      xyz[3];
	float      vxyz[3];
	int           mode;
};
//struct Airmols airmol[320][160][150];//对airmol进行写操作，程序崩溃。改用动态内存分配。

struct _Airfoil{
	double         xy[2];
	double pxy[THNUM][2];
	double          lean;
}airfoil[2][1024];

#define FOILLT 3.2768

struct _input{
	int                thrc;
	struct _Airmols* airmol;
}input;

int   initSpeedAirmol(struct _Airmols *airmol);
void  airProcessor(struct _input input);
void  drawFoil(void);
void  printhelp(void);

FILE *fileInit, *fileOut;
pthread_t pap[THNUM];
pthread_attr_t pap_attriber;
_int_64_u countl=0;
_int_64_u molSpeedSize[2]={0}, randomc;
double    initxyz[2], pxt_total[2]={0.0}, force[2];
double    xFB[4],     yDU[4], drawxy[2][1024][2], space_foil=0.0;
float     angle[2]={-5.0, -5.0}, anglet;
float     averSpeed, readSpeed, airSpin[3];
int       naca, mpt[3], msg[16]={PPause}, count, flip, flipCount;
char     *WVPHead;


int main(int argc, char* argv[]){
	printf("Storm-VWT version 0.0.1\n");
	fileOut=fopen("out.txt","a+");
	fprintf(fileOut, "%s", "========Storm-VWT输出文件========\r\n");
	fprintf(fileOut, "%s", "翼型\t\t攻角\t\t升力\t\t\t\t阻力\r\n");
	srand((unsigned)time(NULL));
	if(argc==1){
		printhelp();
		pause();
		return 0;
	}else{
		int   argvopt;
		char *inputReq="n:a:A:";
		count=0;                                                                 //count征用为参数计数器? 
		while((argvopt=getopt(argc, argv, inputReq))!=-1){
			if(argvopt=='n'){
				naca=strtonuum(optarg,4);
				if((naca<10000)&&(naca>0)){
					mpt[2]=naca%100;
					mpt[1]=((naca-mpt[2])/100)%10;
					mpt[0]=(naca-naca%1000)/1000;
				}else{
					printf("错误。退出。\n");
					return -1;
				}
			}else if(argvopt=='a'){
				angle[0]=atof(optarg);
			}else if(argvopt=='A'){
				angle[1]=atof(optarg);
			}
			count++;
		}
	}
	//加载空气分子&翼型 
	struct _Airmols *airmol=(struct _Airmols*)malloc((1+MAXSize)*sizeof(struct _Airmols));
	drawFoil();
    //分配位置&初始化属性 
    countl=0;
    initxyz[0]=-0.05;
    for(int i=0;i<320;i++){
        initxyz[0]+=0.1;
        initxyz[1]=-0.05;
        for(int j=0;j<160;j++){
            initxyz[1]+=0.1;
            initxyz[2]=-0.05;
            for(int k=0;k<150;k++){
                initxyz[2]+=0.1;
                airmol[countl].xyz[0]=initxyz[0];
                airmol[countl].xyz[1]=initxyz[1];
                airmol[countl].xyz[2]=initxyz[2];
                airmol[countl].mode  = Unknown  ;
				countl ++;
            }
        }
    }initSpeedAirmol(airmol);
	printf("正在初始化翼型");
	//按攻角初始化翼型 
	for(anglet=angle[0];anglet<=angle[1];anglet+=0.5){
		for(int i=0;i<1024;i++){
			printf("已进行%d/1024\n",i);
			airfoil[0][i].xy[0]=FOILLT*(drawxy[0][i][0]*cos(anglet)-drawxy[0][i][1]*sin(anglet))+8.0;
			if(xFB[1]>airfoil[0][i].xy[0]) xFB[1]=airfoil[0][i].xy[0];
			if(xFB[2]<airfoil[0][i].xy[0]) xFB[2]=airfoil[0][i].xy[0];
			airfoil[0][i].xy[1]=FOILLT*(drawxy[0][i][0]*sin(anglet)+drawxy[0][i][1]*cos(anglet))+9.0;
			if(yDU[1]>airfoil[0][i].xy[1]) yDU[1]=airfoil[0][i].xy[1];
			if(yDU[2]<airfoil[0][i].xy[1]) yDU[2]=airfoil[0][i].xy[1];
			airfoil[1][i].xy[0]=FOILLT*(drawxy[1][i][0]*cos(anglet)-drawxy[1][i][1]*sin(anglet))+8.0;
			if(xFB[1]>airfoil[1][i].xy[0]) xFB[1]=airfoil[1][i].xy[0];
			if(xFB[2]<airfoil[1][i].xy[0]) xFB[2]=airfoil[1][i].xy[0];
			airfoil[1][i].xy[1]=FOILLT*(drawxy[1][i][0]*sin(anglet)+drawxy[1][i][1]*cos(anglet))+9.0;
			if(yDU[1]>airfoil[1][i].xy[1]) yDU[1]=airfoil[1][i].xy[1];
			if(yDU[2]<airfoil[1][i].xy[1]) yDU[2]=airfoil[1][i].xy[1];
			for(int j=0;j<THNUM;j++){
				airfoil[0][i].pxy[j][0]=0.0,airfoil[0][i].pxy[j][1]=0.0;
				airfoil[1][i].pxy[j][0]=0.0,airfoil[1][i].pxy[j][1]=0.0;
			}
			//计算斜率 
			if(i>=1){								
				airfoil[0][i-1].lean=(airfoil[0][i].xy[2]-airfoil[0][i-1].xy[2])/(airfoil[0][i].xy[1]-airfoil[0][i-1].xy[1]);
				airfoil[0][i-1].lean=(airfoil[1][i].xy[2]-airfoil[1][i-1].xy[2])/(airfoil[1][i].xy[1]-airfoil[1][i-1].xy[1]);
			}if((airfoil[0][i-1].lean>-65536.0)&&(airfoil[0][i-1].lean<65536.0)){
				continue;
			}else{
				airfoil[0][i].lean=INFINITY;
			}
		}airfoil[0][1024].lean=airfoil[0][1023].lean;
		airfoil[1][1024].lean=airfoil[1][1023].lean;
		space_foil*=FOILLT*FOILLT*1000*150;
		printf("%f\n",space_foil);
		xFB[0]=xFB[1]-(1600.0+averSpeed)*FDeltaT;
		xFB[3]=xFB[2]+(1600.0-averSpeed)*FDeltaT;
		yDU[0]=yDU[1]-1600.0*FDeltaT;
		yDU[3]=yDU[2]+1600.0*FDeltaT;
		_int_64_u space_foil_i =round(space_foil), MAXnew=round((xFB[2]-xFB[1])*(yDU[2]-yDU[1])*TWIDTH*1000);
		//创建分子引擎:此代码段有Bug 
		for(int i=0;i<THNUM;i++){
			input.thrc=i;
			input.airmol=airmol;
			pthread_create(&pap[i], NULL, (void *)&airProcessor, &input);
		}sleep(1);
		printf("启动帧循环。当前要完成128帧模拟，帧间隔为0.0002s.\n");
		int msgcheck=0;
		for(flip=0;flip<128;){
			for(int i=0;i<THNUM;i++){
				msgcheck=(msgcheck||msg[i]);
			}if(msgcheck!=PPause)continue;
			for(int i=0;i<THNUM;i++){
				msg[i]=PGoOn;
			}flip++;
			printf("正在进行：\t%d/128",flip);
		}
		sleep(10);
		for(int i=0;i<THNUM;i++){
			msg[i]=PKill;
		}sleep(5000);
		printf("完成.正在整理...\n");
		for(int i=0;i<1024;i++){
			for(int j=0;j<THNUM;j++){
				pxt_total[0]+=airfoil[0][i].pxy[j][0]+airfoil[1][i].pxy[j][0];
				pxt_total[1]+=airfoil[0][i].pxy[j][1]+airfoil[1][i].pxy[j][1];
			}
		}force[0]=pxt_total[0]/0.0256;
		force[1] =pxt_total[1]/0.0256;
		fprintf(fileOut, "%s%d%s%f%s%f%s%f", "\r\n\t\t", naca, "\t\t", anglet, "\t\t", force[1], "\t\t", force[0]);
		printf("%d%s%f%s%f%s%f%s", naca,"\t\t", anglet, "\t\t", force[1], "\t\t",force[0]);
	}
	fclose(fileOut);
    return 0;
}

void printhelp(void){
	printf("\n\t用法: \n");
	printf("\n\tstorm_vwt -选项 参数\n");
	printf("选项:\n");
	printf("-n\tNACA【四位】翼型[后加一个NACA翼型编码参数].默认为0000\n");
	printf("-a\t攻角最小值(单位:角度).\t\t\t   默认为0.0\n");
	printf("-A\t攻角最大值(单位:角度).\t\t\t   默认为0.0\n"); 
}

void drawFoil(void){
	double deltax=1.0/1024, m0=mpt[0]*1.0/100,p0=mpt[1]*1.0/10,t0=mpt[2]*1.0/100;
	double centX=0.0, centY=0.0, tempX,tempY, space_foil_t=0.0;
	space_foil=0.0;
	for(count=0;count<1024;count++){
		centX+=deltax;
		if(centX<=p0){
			tempY=2*m0*(p0-tempX)/(p0*p0);
			tempX=2*p0*centX-centX*centX;
			centY=m0/(p0*p0)*tempX;
		}else{
			tempY=2*m0*(p0-tempX)/((1-p0)*(1-p0));
			tempX=2*p0*centX-centX*centX;
			centY=m0/((1-p0)*(1-p0))*(1-2*p0+tempX);
		}tempX=5*t0*(0.2969*sqrt(centX)+(((-0.1015*centX+0.2843)*centX-0.3516)*centX-0.1260)*centX);
		drawxy[0][count][0]=centX-tempX*tempY/sqrt(1+tempY*tempY);
		drawxy[0][count][1]=centY+tempX/sqrt(1+tempY*tempY);
		drawxy[1][count][0]=centX+tempX*tempY/sqrt(1+tempY*tempY);
		drawxy[1][count][1]=centY-tempX/sqrt(1+tempY*tempY);
		if(count>1){
			space_foil_t=fabs((drawxy[0][count][1]-drawxy[1][count][1])*(drawxy[0][count][0]-drawxy[0][count-1][0])/sqrt(1+tempY*tempY));
			space_foil = space_foil+space_foil_t;
		}
	}
}

int initSpeedAirmol(struct _Airmols *airmol){
    char            WVPTitle[4];
    float               airTemp; 
    fileInit=fopen("./vxyz_init.wvp","rb");
    fseek(fileInit, 0 , SEEK_SET);
    fread(WVPTitle, 4 , 1, fileInit);
    fread(&airTemp        ,sizeof(float)    ,1,fileInit);
    fread(&averSpeed      ,sizeof(float)    ,1,fileInit);
    fread(&molSpeedSize[0],sizeof(_int_64_u),1,fileInit);
    fread(&molSpeedSize[1],sizeof(_int_64_u),1,fileInit);
    printf("温度:%f K\t风速:%f m/s\n",airTemp,averSpeed);
    for(countl=0;countl<MAXSize;countl++){
        int mmm=0;
		randomc=(randlu()%molSpeedSize[0])*sizeof(float)+28;
		fseek(fileInit, randomc, SEEK_SET);
		fread(&readSpeed, sizeof(float), 1, fileInit);
		randomc=(randlu()%molSpeedSize[1])*sizeof(float)*3 + 28 + molSpeedSize[0]*sizeof(float);
		fseek(fileInit, randomc, SEEK_SET);
		fread(&airSpin[0],sizeof(float), 1, fileInit);
		fread(&airSpin[1],sizeof(float), 1, fileInit);
		fread(&airSpin[2],sizeof(float), 1, fileInit);
		(*(airmol+countl)).vxyz[0]=readSpeed*airSpin[0]*rdsgn()+averSpeed;//浮点数问题 
		(*(airmol+countl)).vxyz[1]=readSpeed*airSpin[1];
//		(*(airmol+countl)).vxyz[2]=readSpeed*airSpin[2];                  //暂时不用。日后会加入对Z坐标的支持。 
	}
    return 0;
}

void  airProcessor(struct _input input){
	int thrc = input.thrc;
	struct _Airmols* airmol = input.airmol;
	_int_64_u molPosEdge[2], molSizeRT,mcoul;
	molPosEdge[0]=(MAXSize/(THNUM))*thrc;
	molPosEdge[1]=(MAXSize/(THNUM))+molPosEdge[0]-1;
	FILE *fileRT =fopen("./vxyz_RT.wvr"  ,"rb");
	double deltax, deltay, molx, moly, ftmpx, ftmpy, fcmpx, fcmpy, fdmpx,fdmpy;
	double xmax  , ymax  , xmin, ymin, lineK, lineB;
	double fchkx=0.0 , fchky=0.0, leank=0.0, leank2=0.0;
	double lineKF=0.0;
	float  speedx, speedy, speedRT, spinRT,  leankf=0.0, leankg=0.0, speedt=0.0;
	int    count0=0, count1=0, count2=0, count3=0;
	int    sgnDMm=0, foilstc[2][16], count4=0;
	int    smpiu[8], smpid[8], foilst[2][1024], cntsmp[8];
	char   cntMFcct, cntMFiot;
	fseek(fileRT  ,12 , SEEK_SET);
	fread(&molSizeRT, sizeof(molSizeRT), 1,fileRT);
	while(msg[thrc]!=PKill){                                            //消息判断
		if(msg[thrc]==PGoOn){                                           //启动
			msg[thrc]=PBusy;
			for(mcoul=molPosEdge[0];mcoul<molPosEdge[1];mcoul++){
				if((*(airmol+mcoul)).mode==UnAvail)continue;
				molx   = (*(airmol+mcoul)).xyz[0];
				moly   = (*(airmol+mcoul)).xyz[1];
				speedx = (*(airmol+mcoul)).vxyz[0];
				speedy = (*(airmol+mcoul)).vxyz[1];
				deltax = molx + speedx*FDeltaT;
				deltay = moly + speedx*FDeltaT;
				lineK  = speedy / speedx;
				lineB  = moly - molx * lineK;
				if((deltax<0)||(deltax>320)){
					if((*(airmol+mcoul)).mode==Unknown){
						(*(airmol+mcoul)).mode==Normal;
					}
					_int_64_u rdseed=(randlu()%molSizeRT)*sizeof(float)+28;
					fseek(fileRT, rdseed, SEEK_SET);
					fread(&speedRT, sizeof(float), 1, fileRT);
					_int_64_u spseed=(randlu()%molSpeedSize[1])*sizeof(float)*3+28+molSpeedSize[0]*sizeof(float);
					fseek(fileInit, spseed, SEEK_SET);
					fread(&spinRT,  sizeof(float), 1, fileInit);
					(*(airmol+mcoul)).vxyz[0]=speedRT*spinRT*rdsgn()+averSpeed;
					if((*(airmol+mcoul)).vxyz[0]>=0){
						(*(airmol+mcoul)).xyz[0]=0;
					}else{
						(*(airmol+mcoul)).xyz[0]=320;
					}fread(&speedRT, sizeof(float), 1, fileRT);
					(*(airmol+mcoul)).vxyz[1]=speedRT*spinRT*rdsgn();
					continue;
				}else if(deltay<0){
					if((*(airmol+mcoul)).mode==Unknown){
						(*(airmol+mcoul)).mode==Normal;
					}
					(*(airmol+mcoul)).vxyz[1]*= -1;
					(*(airmol+mcoul)).xyz[1]  = -deltay;
					continue;
				}else if(deltay>320){
					if((*(airmol+mcoul)).mode==Unknown){
						(*(airmol+mcoul)).mode==Normal;
					}
					(*(airmol+mcoul)).vxyz[1]*= -1;
					(*(airmol+mcoul)).xyz[1]  = 320-deltay;
					continue;
				}else if((molx>=xFB[0])&&(molx<=xFB[3])&&(moly>=yDU[0])&&(moly<=yDU[3])){
					if(deltax>molx){
						xmax=deltax;ymax=deltay;
						xmin=molx  ;ymin=moly;
						sgnDMm = 1;
					}else{
						xmin=deltax;ymin=deltay;
						xmax=molx  ;ymax=moly;
						sgnDMm = -1;
					}
					if((*(airmol+mcoul)).mode==Unknown){                                             //分子属性检查 
						for(int i=0;i<=1024;i++){
							if(sgnDMm==1){
								if((airfoil[0][i].xy[0]>xmin)&&(airfoil[0][i-1].xy[0]<xmin)){
									foilstc[0][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}else if((airfoil[0][i].xy[0]>xmin)&&(airfoil[0][i+1].xy[0]<xmin)){
									foilstc[0][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}
								if((airfoil[1][i].xy[0]>xmin)&&(airfoil[1][i-1].xy[0]<xmin)){
									foilstc[1][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}else if((airfoil[1][i].xy[0]>xmin)&&(airfoil[1][i+1].xy[0]<xmin)){
									foilstc[1][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}
							}else if(sgnDMm==-1){
								if((airfoil[0][i].xy[0]<xmax)&&(airfoil[0][i+1].xy[0]>xmax)){
									foilstc[0][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}else if((airfoil[0][i].xy[0]<xmax)&&(airfoil[0][i-1].xy[0]>xmax)){
									foilstc[0][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}
								if((airfoil[1][i].xy[0]<xmax)&&(airfoil[1][i+1].xy[0]>xmax)){
									foilstc[1][count2]  =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}else if((airfoil[1][i].xy[0]<xmax)&&(airfoil[1][i-1].xy[0]>xmax)){
									foilstc[1][count2] =(linecnt(airfoil[0][i].xy[0])>airfoil[0][i].xy[1]);
									count2++;
								}
							}if(i==0)continue;
						}
					}
					/*启动碰撞检查*/
					while(1){                                                    //设置这样的循环，是考虑到多次碰撞的情况。 
						lineK  = speedy / speedx;
						if((lineK>65536.0)||(lineK<-65536.0)||(isnan(lineK))){
							lineK=INFINITY;
						}
						lineB  = moly - molx * lineK;
						for(int i=0;i<1024;i++){
							if((airfoil[0][i].xy[0]>=xmin)&&(airfoil[0][i].xy[0]<=xmax)){ //计算零点 
								ftmpy = linecnt(airfoil[0][i].xy[0]);            //ftmpy被征用为工件点对应线纵坐标 
								if(ftmpy==airfoil[0][i].xy[1]){
									smpiu[count0]=i;
									foilst[0][i]=0;                              //注意，泛用途变量在使用时需要在注释中声明用途。 
									count0++;                                    //count0被征用为上表面零点计数器 
								}else if(ftmpy>airfoil[0][i].xy[1])foilst[0][i]=1;
								else{foilst[0][i] = -1;}
								if(i==0)continue;
								if(foilst[0][i]*foilst[0][i-1]<0){
									smpiu[count0]=i-1;
									count0++;
								}
							} 
							if((airfoil[1][i].xy[0]>=xmin)&&(airfoil[1][i].xy[0]<=xmax)){
							ftmpy = linecnt(airfoil[1][i].xy[0]);
								if(ftmpy==airfoil[1][i].xy[1]){
									smpid[count1]=i;
									foilst[1][i]=0;
									count1++;                                    //count1被征用为下表面零点计数器 
								}else if(ftmpy>airfoil[1][i].xy[1])foilst[1][i]=1;
								else{foilst[1][i] = -1;}
								if(i==0)continue;
								if(foilst[1][i]*foilst[1][i-1]<0){
									smpid[count1]=i-1;
									count1++;
								}
							}ftmpy=0.0;                                          //取消征用ftmpy 
						}if((*(airmol+mcoul)).mode==Unknown){                    //分子属性检查——阶段二 
							for(int i=1;i<count2;i++){
								if(foilstc[0][i]!=foilstc[0][i-1]){
									(*(airmol+mcoul)).mode=UnAvail;
								}
							}if(foilstc[0][0123]!=foilstc[1][0]){
								(*(airmol+mcoul)).mode=UnAvail;
							}for(int i=1;i<count2;i++){
								if(foilstc[1][i]!=foilstc[1][i-1]){
									(*(airmol+mcoul)).mode=UnAvail;
								}
							}
						}if((count0==0)&&count1==0)continue;                      //碰撞点检测——阶段二 
						fchkx=airfoil[0][smpiu[0]].xy[0];
						fchky=airfoil[0][smpiu[0]].xy[1];
						for(int i=1;i<=count0;i++){
							count2=smpiu[i];
							if((fchkx<airfoil[0][count2].xy[0])==(sgnDMm==1)){
								fchkx=airfoil[0][count2].xy[0];
								fchky=airfoil[0][count2].xy[1];
								count3=count2;
								count4=0;
							}
						}for(int i=1;i<=count1;i++){
							count2=smpid[i];
							if((fchkx<airfoil[0][count2].xy[0])==(sgnDMm==1)){
								fchkx=airfoil[1][count2].xy[0];
								fchky=airfoil[1][count2].xy[1];
								count3=count2;
								count4=1;
							}
						}		 		                                          //完毕。撤用count0/1. 
						if((airfoil[count4][count3].lean==INFINITY)||(isnan(airfoil[count4][count3].lean))){
							ftmpx=2.0*molx-deltax;                                  //ftmpx/y被征用为分子坐标 
							fcmpx=airfoil[count4][count3].xy[0];
							ftmpy=deltay;
							fcmpy=linecnt(fcmpx);
							speedx*=-1.0;
						}else{
							if(lineK==INFINITY){                                     //lineK  = speedy / speedx;
								lineKF=0.0;
							}else{
								lineKF=1.0/lineK;
							}
							leank=airfoil[count4][count3].lean;
							leank2=leank*leank;
							ftmpx=((1.0-leank2)*deltax-2.0*leank*(deltay-airfoil[count4][count3].xy[0]+airfoil[count4][count3].xy[1]))/(leank2+1);
							ftmpy=(1.0/leank)*(ftmpx-deltax)+deltay;
							leankg=(ftmpy-fcmpy)/(ftmpx-fcmpx);                      //leankg征用为变后斜率 
							speedt=sqrtf(speedx*speedx+speedy*speedy);
							speedx=speedt/sqrtf(1.0+leankg*leankg);
							speedy=speedx*leankg;
						}
						molx=fcmpx;  moly=fcmpy;
						deltax=ftmpx;deltay=ftmpy;
					}
					(*(airmol+mcoul)).xyz[0]=deltax;
					(*(airmol+mcoul)).xyz[1]=deltay;
					(*(airmol+mcoul)).vxyz[0]=speedx;
					(*(airmol+mcoul)).vxyz[1]=speedy;
				}
			}
		}
	}
	
	pthread_exit(NULL);
}

#endif
