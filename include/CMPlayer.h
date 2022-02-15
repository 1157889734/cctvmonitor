#ifndef CMPLAYER_H
#define CMPLAYER_H


#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

typedef void* CMPHandle;

typedef enum _E_CMP_ERRCODE
{
    CMP_MEDIAHANDLE_NOTEXIST = -1,      //ý����������
    CMP_VDECINIT_ERR = -2,   //��Ƶ�����ʼ������
    CMP_VDECCH_CREATE_ERR = -3, 		//��Ƶ����ͨ������ʧ��
    CMP_WND_SETPOS_ERR = -4, 			//���Ŵ���λ������ʧ��
    CMP_VDECCH_DESTORY_ERR = -5,        //��Ƶ����ͨ������ʧ��
    CMP_VDECSTART_ERR = -6,				//������Ƶ����ʧ��
    CMP_MEDIAPLAY_ERR = -7,             //ý�岥��ʧ��
    CMP_VDECCH_ENABLE_ERR = -8, 		//����ͨ��ʹ��ʧ��
    CMP_STOPPLAY_ERR = -9,        		//ֹͣ����ʧ��
    CMP_DEVLOGIN_FAIL = -20,     	    //�豸��¼ʧ��
    CMP_CREATEDECWIN_ERR = -21,    		    //����ͨ�����ڴ���ʧ��
    CMP_PLAY_ERR = -22,	  			    //����ʧ��
    CMP_DESTORYDECWIN_ERR = -23,   		    //����ͨ����������ʧ��
    CMP_MEM_ERR = -25,					//�ڴ����ʧ��
    CMP_ERR_OTHER = -26   				    //��������
} _E_CMP_ERRCODE;

enum STREAM_TYPE
{
    CMP_TCP = 0,
    CMP_UDP
};

enum CMPPLAY_STATE
{
    CMP_STATE_IDLE = 0,        // ����
    CMP_STATE_PLAY,            // ����
    CMP_STATE_FAST_FORWARD,    // ���
    CMP_STATE_SLOW_FORWARD,    // ����
    CMP_STATE_PAUSE,           // ��ͣ
    CMP_STATE_STOP             // ֹͣ״̬
};

typedef enum _E_CMP_VO_TYPE    //��Ƶ�������
{
    CMP_VO_TYPE_HDMI,
    CMP_VO_TYPE_VGA,
    CMP_VO_TYPE_CVBS,
    CMP_VO_TYPE_MAX
} E_CMP_VO_TYPE;

typedef struct _T_VIDEO_INFO
{
    E_CMP_VO_TYPE eVoType;	//��Ƶ�������
    int iScreenWidth;	//����
    int iScreenHeight;	//����
} T_VIDEO_INFO,*PT_VIDEO_INFO;


/*************************************************
  ��������:     CMP_Init
  ��������:     ģ���ʹ��
  �������:     ��
  �������:     ��
  ����ֵ:       0-�ɹ�������ʧ��
  ���ߣ�        dingjq
  ����:         2015-09-10
*************************************************/
int CMP_Init(PT_VIDEO_INFO ptVdecInfo);

/*************************************************
  ��������:     CMP_UnInit
  ��������:     ģ�鷴��ʹ��
  �������:     ��
  �������:     ��
  ����ֵ:       0���ɹ��� <0:ʧ��
  ���ߣ�        
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_UnInit(void);

/*************************************************
  ��������:     CMP_SetBlackBackground
  
  ��������: ���˿�ı����ڻ�
  �������:     ix iy iWidth iHeight
  �������:     ��
  ����ֵ:       0���ɹ��� <0:ʧ��
  ���ߣ�        
  ����:         2018-11-21
  �޸�:
*************************************************/
int CMP_SetBlackBackground(int ix,int iy,int iWidth,int iHeight);


/*************************************************
  ��������:     CMP_CreateMedia
  ��������:     ����ý����
  �������:     tWinInfo��ý����ʾ����λ����Ϣ
  ����ֵ:       ý����
  ���ߣ�      
  ����:         2016-11-01
*************************************************/
CMPHandle CMP_CreateMedia(void *pWinHandle);

/*************************************************
  ��������:     CMP_DestroyMedia
  ��������:     ����ý����
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       0���ɹ��� <0:ʧ��
  ���ߣ�     
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_DestroyMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_SetWndDisplayEnable
  ��������:     ���ò��Ŵ�����ʾʹ��,���ú�Ҳ�ر��˽���
  �������:     hPlay��ý����
                iEnable: ʹ�ܱ�־��1-ʹ�ܣ�0-��ʹ��
  �������:     ��
  ����ֵ:
  ���ߣ�       
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_SetWndDisplayEnable(CMPHandle hPlay, int iDispEnable,int iDecEnable );


/*************************************************
  ��������:     CMP_SetWnd
  ��������:     �ı䲥�Ŵ���
  �������:     hPlay��ý���� tWnd �ı���Ŀ�괰��
  �������:     ��
  ����ֵ:
  ���ߣ�    
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_ChangeWnd(CMPHandle hPlay, void *pWinHandle);


/*************************************************
  ��������:     CMP_OpenMediaPreview
  ��������:     ��Ԥ��ý��
  �������:     hPlay��ý������ pcRtspUrl��rtsp��ַ�� iTcpFlag��CMP_TCP or CMP_UDP
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�    
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag);

/*************************************************
  ��������:     CMP_OpenMediaFile
  ��������:     �򿪵㲥ý��
  �������:     hPlay��ý������ pcRtspFile:rtsp�ļ���ַ �� iTcpFlag��CMP_TCP or CMP_UDP
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�    
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag);

/*************************************************
  ��������:     CMP_CloseMedia
  ��������:     �ر�ý��
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       0���ɹ��� -1:δ�ҵ���Ӧý����
  ���ߣ�  
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_CloseMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_GetPlayStatus
  ��������:     ��ȡ����״̬
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       ����ý�岥��״̬
  ���ߣ�  
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_GetPlayStatus(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_PlayMedia
  ��������:     ��ʼ����ý����
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:
  ���ߣ�    
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_PlayMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_PauseMedia
  ��������:     ��ͣ����ý����
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:
  ���ߣ�   
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_PauseMedia(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_SetPosition
  ��������:     ���ò���λ�ã��룩
  �������:     hPlay��ý����
  				nPosTime: ��תʱ��(��λS)
  �������:     ��
  ����ֵ:
  ���ߣ�     
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_SetPosition(CMPHandle hPlay, int nPosTime);

/*************************************************
  ��������:     CMP_SetPlaySpeed
  ��������:     ���ò����ٶ�
  �������:     hPlay��ý����
  				dSpeed: �ٶ�ֵ
  �������:     ��
  ����ֵ:
  ���ߣ�     
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed);

/*************************************************
  ��������:     CMP_GetPlayRange
  ��������:     ��ȡ����ʱ��
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       �ļ�������ʱ��������Ϊ��λ
  ���ߣ�     
  ����:         2016-11-01
  �޸�:
*************************************************/
int CMP_GetPlayRange(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_GetCurrentPlayTime
  ��������:     ��ȡ¼��ǰ����ʱ��
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       ¼��ǰ����ʱ�䣬����Ϊ��λ
  ���ߣ�     
  ����:         2017-5-12
  �޸�:
*************************************************/
int CMP_GetCurrentPlayTime(CMPHandle hPlay);

/*************************************************
  ��������:     CMP_GetStreamState
  ��������:     ��ȡ����״̬(�Ƿ�����)
  �������:     hPlay��ý����
  �������:     ��
  ����ֵ:       �Ƿ�������״̬��1-������0-����
  ���ߣ�     
  ����:         2017-5-12
  �޸�:
*************************************************/
int CMP_GetStreamState(CMPHandle hPlay);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif // CMPLAYER_H
