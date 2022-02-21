#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>

#include <QGuiApplication>
#ifdef IMX8
#include "5.10.1/QtGui/qpa/qplatformnativeinterface.h"
#else
#include "5.14.2/QtGui/qpa/qplatformnativeinterface.h"
#endif

#include "mutex.h"
#include "RockchipRga.h"
#include "RgaUtils.h"

#define CODEC_ALIGN(x, a)   (((x)+(a)-1)&~((a)-1))

typedef struct  T_SHM_RECT_INFO
{
    int    w;
    int    h;
    int   size;
    int   fd;
    uchar *addr;
    struct wl_surface * volatile window_handle;
    struct wl_buffer* buffer;
    QWidget *pWidget;
    CMutexLock lock;
    T_SHM_RECT_INFO()
    {
        fd = 0;
        w = h = 0;
        addr = NULL;
        buffer = NULL;
        window_handle = NULL;
        pWidget = NULL;
    }

}*PT_SHM_RECT_INFO;



static QPlatformNativeInterface *native = NULL;
static struct wl_display *display_handle = NULL;

static struct wl_shm        *s_shm = NULL;

static PT_SHM_RECT_INFO create_rect_info(int w, int h)
{
    int stride = w;
    int size = stride * h * 2;
    static int tmp_file_index = 0;
    char filename[64] = {0};// "/home/data/hello-wayland-XXXXXX";
    sprintf(filename, "/tmp/hello-%d-wayland-XXXXXX", tmp_file_index++);

    if(s_shm == NULL)
    {
        return NULL;
    }


    int fd = mkstemp(filename);
    if(fd < 0)
    {
        printf("mkstemp failed : %m\n");
        return NULL;
    }

    if(fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC) < 0)
    {
        close(fd);
        return NULL;
    }
    if(ftruncate(fd, size) < 0)
    {
        close(fd);
        return NULL;
    }

    uchar *shm_data = (uchar*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_data == MAP_FAILED) {
        printf("mmap failed : %m\n");
        close(fd);
        return NULL;
    }

    unlink(filename);

    struct wl_shm_pool *pool = wl_shm_create_pool(s_shm, fd, size);
    struct wl_buffer   *buffer = wl_shm_pool_create_buffer(pool, 0, w, h, stride, WL_SHM_FORMAT_NV12);
    wl_shm_pool_destroy(pool);

    //memset(shm_data, 0x0, size);

    PT_SHM_RECT_INFO pRectInfo = new T_SHM_RECT_INFO; //2021-1122
    pRectInfo->addr = shm_data;
    pRectInfo->buffer = buffer;
    pRectInfo->w = w;
    pRectInfo->h = h;
    pRectInfo->fd = fd;
    pRectInfo->size = size;

    return pRectInfo;
}


static void handle_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    printf("handle_global : %s \n", interface);
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        s_shm = (struct wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
        uint32_t name) {
    // Who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};


int SHM_Init()
{
    printf("SHM_Init 0 \n");
    if(native == NULL)
        native = QGuiApplication::platformNativeInterface();
    if(display_handle == NULL)
        display_handle = (struct wl_display *)native->nativeResourceForWindow("display", NULL);

    printf("SHM_Init begin \n");
    struct wl_registry *registry = wl_display_get_registry(display_handle);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display_handle);

    printf("SHM_Init ok \n");
    return 0;
}
int SHM_Uinit()
{
    return 0;
}

SHM_HANDLE SHM_AddRect(QWidget *pWnd)
{
    int w = pWnd->width();
    int h = pWnd->height();

    printf("SHM_AddRect pWnd w:%d h:%d \n", w, h);

    w = CODEC_ALIGN(w, 16);
    h = CODEC_ALIGN(h, 16);

    printf("SHM_AddRect 16 ALIGN w:%d h:%d \n", w, h);

    PT_SHM_RECT_INFO pShmRectInfo = create_rect_info(w, h);
    QWidget *pWidget = new QWidget(pWnd);
    pWidget->setGeometry(0,0,pWnd->width(), pWnd->height());
    pShmRectInfo->pWidget = pWidget;
    pWidget->hide();
    //if(bo->format == DRM_FORMAT_NV12)
    {
        memset(pShmRectInfo->addr, 0x10, w * h);
        memset(pShmRectInfo->addr + w * h, 0x80, w * h * 0.5);
    }


    printf("add rect end \n");
    return pShmRectInfo;
}

int SHM_FreeRect(SHM_HANDLE hShmHandle)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }
    pShmRectInfo->lock.Lock();
    wl_buffer_destroy(pShmRectInfo->buffer);
    munmap(pShmRectInfo->addr, pShmRectInfo->size);
    close(pShmRectInfo->fd);
    delete pShmRectInfo->pWidget;
    pShmRectInfo->lock.Unlock();
    delete pShmRectInfo;
    pShmRectInfo = NULL;

    return 0;
}

int SHM_AttchWnd(SHM_HANDLE hShmHandle)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }

    if(pShmRectInfo->pWidget->isVisible())
    {
        return 0;
    }
    printf("************pShmRectInfo->pWidget=%p\n",pShmRectInfo->pWidget);
    pShmRectInfo->pWidget->show();
    pShmRectInfo->pWidget->winId();
    if(pShmRectInfo->window_handle)
    {
//        SHM_DetchWnd(hShmHandle);

        return 0;
    }
    printf("*****************%s---%d\n",__FUNCTION__,__LINE__);

    struct wl_surface *window_handle  = NULL;
    window_handle = (struct wl_surface *)native->nativeResourceForWindow("surface",
                                  pShmRectInfo->pWidget->windowHandle());
    if(window_handle == NULL)
    {

        printf("window_handle NULL \n");
        return NULL;
    }
    printf("**************window_handle=%p**pShmRectInfo->buffer=%p***%s---%d---\n",window_handle,pShmRectInfo->buffer,__FUNCTION__,__LINE__);

    pShmRectInfo->lock.Lock();
//    printf("wl_surface_attach, %0x \n", window_handle);
    wl_surface_attach(window_handle, pShmRectInfo->buffer, 0, 0);
    wl_surface_commit(window_handle);
    wl_display_flush(display_handle);
    pShmRectInfo->window_handle = window_handle;
    pShmRectInfo->lock.Unlock();
    printf("*****************%s---%d\n",__FUNCTION__,__LINE__);

    return 0;
}




int SHM_DetchWnd(SHM_HANDLE hShmHandle)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }
    if(!pShmRectInfo->window_handle)
    {
        return -1;
    }
    if(!pShmRectInfo->pWidget->isVisible())
    {
        return 0;
    }
    pShmRectInfo->pWidget->hide();
    pShmRectInfo->lock.Lock();
    //wl_surface_attach(pShmRectInfo->window_handle, 0, 0, 0);
    //wl_surface_commit(pShmRectInfo->window_handle);
//    printf("wl_surface_DEttach %0x \n", pShmRectInfo->window_handle);
    //wl_display_flush(display_handle);
    pShmRectInfo->window_handle = NULL;
    pShmRectInfo->lock.Unlock();

    return 0;
}

int SHM_FillRect(SHM_HANDLE hShmHandle, uint32_t color)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }
    if(!pShmRectInfo->window_handle)
    {
        return -1;
    }
    pShmRectInfo->lock.Lock();
    memset(pShmRectInfo->addr, 0x10, pShmRectInfo->w * pShmRectInfo->h);
    memset(pShmRectInfo->addr + pShmRectInfo->w * pShmRectInfo->h, 0x80, pShmRectInfo->w * pShmRectInfo->h * 0.5);
    wl_surface_attach(pShmRectInfo->window_handle, pShmRectInfo->buffer, 0, 0);
    wl_surface_commit(pShmRectInfo->window_handle);
    //wl_display_flush(display_handle);
    pShmRectInfo->lock.Unlock();

    return 0;
}

int SHM_RkRgaBlit(MppFrame tSrcMppFrame, uint8_t *dstAddr, int w, int h)
{
    static rga_info_t rgasrc;
    static rga_info_t rgadst;

    int ret = 0;
    int srcWidth,srcHeight,srcFormat, src_h_stride,src_v_stride;
    int dstWidth,dstHeight,dstFormat;

    RockchipRga &rkRga = RockchipRga::get();

    dstWidth  = w;
    dstHeight = h;//-6
    dstFormat = RK_FORMAT_YCbCr_420_SP;

    MppBuffer buffer  = NULL;
    RK_U8*    srcAddr = NULL;

    srcFormat = RK_FORMAT_YCbCr_420_SP;
    srcWidth  =   mpp_frame_get_width(tSrcMppFrame);
    srcHeight =   mpp_frame_get_height(tSrcMppFrame);
    src_h_stride    =   mpp_frame_get_hor_stride(tSrcMppFrame);
    src_v_stride = mpp_frame_get_ver_stride(tSrcMppFrame);
    buffer = mpp_frame_get_buffer(tSrcMppFrame);
    srcAddr = (RK_U8 *)mpp_buffer_get_ptr(buffer);

    //printf("srcWidth:%d, srcHeight:%d, src_h_stride:%d \n", srcWidth, srcHeight, src_h_stride);
    //printf("dstWidth:%d, dstHeight:%d, dstWidth:%d \n", dstWidth, dstHeight, dstWidth);

    memset(&rgasrc, 0, sizeof(rga_info_t));
    rgasrc.fd = -1;
    rgasrc.mmuFlag = 1;
    rgasrc.virAddr = srcAddr;

    memset(&rgadst, 0, sizeof(rga_info_t));
    rgadst.fd = -1;
    rgadst.mmuFlag = 1;
    rgadst.virAddr = dstAddr;

    //memset(dstAddr, 0x10, w * h);
    //memset(dstAddr+ w * h, 0x80, w * h * 0.5);

    /********** set the rect_info **********/
    rga_set_rect(&rgasrc.rect, 0, 0, srcWidth, srcHeight, src_h_stride, src_v_stride, srcFormat);
    rga_set_rect(&rgadst.rect, 0, 0, dstWidth, dstHeight, dstWidth, dstHeight, dstFormat);

    /************ set the rga_mod ,rotation\composition\scale\copy .... **********/
    //rgasrc.blend = 0xff0105;

    /********** call rga_Interface **********/
    ret = rkRga.RkRgaBlit(&rgasrc, &rgadst, NULL);

    return ret;
}
int SHM_Display(SHM_HANDLE hPlaneHandle, MppFrame frame)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hPlaneHandle;
    if(pShmRectInfo == NULL)
    {
        printf("pShmRectInfo err \n");
        return -1;
    }
    int err = -1;
    err = SHM_RkRgaBlit(frame, pShmRectInfo->addr, pShmRectInfo->w, pShmRectInfo->h);
    if(err < 0)
    {
        printf("rga_blit err \n");
        return -1;
    }
    if(pShmRectInfo->window_handle == NULL)
    {
        return -1;
    }
    pShmRectInfo->lock.Lock();
    wl_surface_attach(pShmRectInfo->window_handle, pShmRectInfo->buffer, 0, 0);
    wl_surface_damage (pShmRectInfo->window_handle, 0, 0,
        pShmRectInfo->w, pShmRectInfo->h);
    wl_surface_commit(pShmRectInfo->window_handle);
    //wl_display_flush(display_handle);
    pShmRectInfo->lock.Unlock();
    return 0;
}
