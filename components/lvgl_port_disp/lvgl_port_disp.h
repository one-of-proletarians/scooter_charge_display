

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************
     *      INCLUDES
     *********************/

#include "lvgl.h"
    /*********************
     *      DEFINES
     *********************/

    /**********************
     *      TYPEDEFS
     **********************/

    /**********************
     * GLOBAL PROTOTYPES
     **********************/
    /* Initialize low level display driver */
    void lv_port_disp_init(void);

    /* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
     */
    void disp_enable_update(void);

    /* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
     */
    void disp_disable_update(void);

    bool lvgl_port_lock();

    void lvgl_port_unlock();

    /**********************
     *      MACROS
     **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/
