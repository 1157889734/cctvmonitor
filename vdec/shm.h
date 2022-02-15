#ifndef SHM_H
#define SHM_H

#include <QWidget>

#define H264_CODE 0
#define H265_CODE 1

#include "rockchip/mpp_common.h"
#include "rockchip/mpp_frame.h"
#include "rockchip/rk_mpi.h"

typedef void* SHM_HANDLE;
int SHM_Init();
int SHM_Uinit();
SHM_HANDLE SHM_AddRect(QWidget *);

int SHM_AttchWnd(SHM_HANDLE);
int SHM_DetchWnd(SHM_HANDLE);
int SHM_FreeRect(SHM_HANDLE hShmHandle);

int SHM_FillRect(SHM_HANDLE hShmHandle, uint32_t color);
int SHM_Display(SHM_HANDLE hPlaneHandle, MppFrame frame);


#endif
