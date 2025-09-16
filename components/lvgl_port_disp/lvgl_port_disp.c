/*********************
 *      INCLUDES
 *********************/
#include "lvgl_port_disp.h"
#include "gc9a01.h"
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/*********************
 *      DEFINES
 *********************/

#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 240

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

#define TICK_TASK_STACK_SIZE (2048)
#define HANDLER_TASK_STACK_SIZE (2 * 2048)
#define BUFF_HEIGHT 80

static SemaphoreHandle_t lvgl_mux;
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);
static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void tick_task(void *p)
{
    for (;;)
    {
        lv_tick_inc(1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void handler_task(void *arg)
{
    (void)arg;

    for (;;)
    {
        if (lvgl_port_lock())
        {
            lv_timer_handler();
            lvgl_port_unlock();

            vTaskDelay(pdMS_TO_TICKS(5));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    vTaskDelete(NULL);
}

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_2_1[MY_DISP_HOR_RES * BUFF_HEIGHT * BYTE_PER_PIXEL];

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_2_2[MY_DISP_HOR_RES * BUFF_HEIGHT * BYTE_PER_PIXEL];
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

bool lvgl_port_lock()
{
    return xSemaphoreTakeRecursive(lvgl_mux, -1);
}

void lvgl_port_unlock()
{
    xSemaphoreGiveRecursive(lvgl_mux);
}

static void disp_init(void)
{
    lvgl_mux = xSemaphoreCreateRecursiveMutex();

    gc9a01_init();
    lv_init();

    static StackType_t tickTaskStack[TICK_TASK_STACK_SIZE];
    static StaticTask_t tickTaskControlBlock;

    static StackType_t handlerTaskStack[HANDLER_TASK_STACK_SIZE];
    static StaticTask_t handlerTaskControlBlock;

    xTaskCreateStatic(
        tick_task,            // Task function
        "tick_task",          // Name of the task
        TICK_TASK_STACK_SIZE, // Stack size in words
        NULL,                 // Task parameter
        1,                    // Task priority
        tickTaskStack,        // Stack array
        &tickTaskControlBlock // Task control block
    );

    xTaskCreateStatic(
        handler_task,            // Task function
        "handler_task",          // Name of the task
        HANDLER_TASK_STACK_SIZE, // Stack size in words
        NULL,                    // Task parameter
        1,                       // Task priority
        handlerTaskStack,        // Stack array
        &handlerTaskControlBlock // Task control block
    );
}

volatile bool disp_flush_enabled = true;

void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    if (disp_flush_enabled)
    {
        lv_draw_sw_rgb565_swap(px_map, lv_area_get_size(area));
        gc9a01_draw(area, px_map);
    }

    lv_display_flush_ready(disp_drv);
}
