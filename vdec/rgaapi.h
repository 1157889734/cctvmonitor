#ifndef _RGAAPI_H_
#define _RGAAPI_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "./mutex.h"
#include "debugout/debug.h"

#define H264_CODE 0
#define H265_CODE 1

#include "rockchip/mpp_common.h"
#include "rockchip/mpp_frame.h"
#include "rockchip/rk_mpi.h"

#include "rga_api.h"

typedef void* RGA_HANDLE;

#define MAX_MPP_FRAME_NUM  10

#define CODEC_ALIGN(x, a)   (((x)+(a)-1)&~((a)-1))

inline void msleep(int x){
    usleep(x*1000);
}


typedef struct _T_RGA_INFO
{
    RgaCtx ctx;
    int iFrameIndex;
    MppBuffer atMppBuffer[MAX_MPP_FRAME_NUM];
    MppFrame  atMppFrame[MAX_MPP_FRAME_NUM];
}T_RGA_INFO;

RGA_HANDLE rga_create(int iWidth, int iHeight);
int rga_destroy(RGA_HANDLE hRga);
int rga_blit(RGA_HANDLE hRga, MppFrame tSrcMppFrame, MppFrame *tDstFrame);
int rga_set_cur_frame(RGA_HANDLE hRga, int iWidth, int iHeight);

#ifdef __cplusplus
}
#endif

#endif
