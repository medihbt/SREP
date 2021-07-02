#ifndef _STORM_VWT_H_
#define _STORM_VWT_H_

//定义风洞尺寸(m)
#define TLENGTH     32.0
#define THEIGHT     16.0
#define TWIDTH      15.0

//定义风洞处理单元模式
#define PPause       0x0 //处理单元暂停, 不允许执行
#define PGoOn        0x1 //处理线程允许运行
#define PBusy        0x2 //处理单元正在运行, 锁定主线程
#define PFinish      0x3 //处理已完成一阶段，锁定并呼唤主线程
#define PKill        0xF

//定义帧间隔(s)
#define FDeltaT   0.0002

//定义空气分子
#define Mass     0.00129464285714286 //质量(kg)
#define MAXSize  7680000

//定义分子模式
#define Unknown      0x0 //未知 
#define Normal       0x1 //正常
#define UnAvail      0x2 //分子不可用

int strtonuum(char* str, int bit){
	int num=0;
	for(int i=0;i<bit;i++){
		if((str[i]<'0')||(str[i]>'9')){
			return -1;
		}else{
			num = num*10 + (str[i]-'0');
		}
	}
	return num;
}

#define rdsgn() (rand()%2==0?1:-1)
#define linecnt(x) (lineK*x+lineB)

#endif
