#ifndef  __PMSG_PROC_H__
#define __PMSG_PROC_H__
#include <stdint.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _T_CARRIAGE_SPEAKER_VOL
{
    uint8_t value_train; //所有客室的扬声器音量值
    uint8_t value[6];       //每节客室的扬声器音量值
    uint8_t value_monitor[2]; // 司机室监听扬声器音量值
    uint8_t ascu_mute[2];  //司机室监听扬声器是否静音
} __attribute__((packed)) T_CARRIAGE_SPEAKER_VOL;

int InitPmsgproc(void);

int UninitPmsgproc(void);

int adjustCarriageSpeakerVolume(T_CARRIAGE_SPEAKER_VOL *ptCarriageSpeakerVol);

int StartNoiseMonitor(char cNoiseMonitorFlag);

int SwitchVideoSrc(char cSrc, char cType);

int GetVideoSrcMode();

int AdjustLcdVolume(char cVolume);

int AdjustDynamicMapBrightness(char cValue);

#ifdef  __cplusplus
}
#endif

#endif
