#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <driver/gpio.h>

#include "lvgl_port_disp.h"
#include "twi.h"
#include "ina.h"

#include "lv_font_montserrat_70_num.h"

#define MAIN_TASK_STACK_DEPTH (2 * 1024)
#define TIME_TASK_STACK_DEPTH (2 * 1024)

static const float AKK_MIN = 50.0;
static const float AKK_MAX = 66.8;
static const float DIVISOR_COEFF = 2.0;

inline int calculate_percent(float min, float max, float current)
{
    if (min == max)
        return 100;

    float percentage = ((current - min) / (max - min)) * 100.0f;

    if (percentage < 0.0f)
        percentage = 0.0f;
    else if (percentage > 100.0f)
        percentage = 100.0f;

    return (int)(percentage + 0.5f);
}

static void main_task(void *p)
{
    lvgl_port_lock();
    lv_obj_t *screen = lv_screen_active();

    lv_obj_t *arc = lv_arc_create(screen);
    lv_obj_set_size(arc, 240, 240);
    lv_arc_set_rotation(arc, 145);
    lv_arc_set_bg_angles(arc, 0, 250);
    lv_arc_set_angles(arc, 0, 250);
    lv_arc_set_range(arc, 0, 100);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc, 50, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 50, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
    lv_obj_center(arc);

    lv_obj_t *label = lv_label_create(screen);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_70_num, LV_PART_MAIN);
    lv_obj_center(label);

    lv_obj_t *label_v = lv_label_create(screen);
    lv_obj_set_style_text_color(label_v, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_v, &lv_font_montserrat_30, LV_PART_MAIN);
    lv_obj_align(label_v, LV_ALIGN_BOTTOM_MID, 0, -40);

    lvgl_port_unlock();

    float voltage = 0.0;
    uint32_t percent_cur = 0;
    uint32_t percent_old = 1000;
    lv_color_t color = {0};

    for (;;)
    {
        voltage = ina_get_voltage() * DIVISOR_COEFF;
        percent_cur = calculate_percent(AKK_MIN, AKK_MAX, voltage);

        if (percent_old != percent_cur)
        {
            percent_old = percent_cur;

            if (percent_cur >= 0 && percent_cur <= 33)
                color = lv_palette_main(LV_PALETTE_RED);
            else if (percent_cur >= 34 && percent_cur <= 66)
                color = lv_palette_main(LV_PALETTE_DEEP_ORANGE);
            else if (percent_cur >= 67 && percent_cur <= 90)
                color = lv_palette_main(LV_PALETTE_ORANGE);
            else
                color = lv_palette_main(LV_PALETTE_GREEN);

            lvgl_port_lock();

            lv_arc_set_value(arc, percent_cur);
            lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);

            lv_label_set_text_fmt(label, "%ld", percent_cur);
            lv_label_set_text_fmt(label_v, "%.1f V", voltage);

            lvgl_port_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void time_task(void *arg)
{
    lvgl_port_lock();
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -15);
    lvgl_port_unlock();

    struct timeval tv = {0};
    struct tm timeinfo;
    char buff[20];

    settimeofday(&tv, 0);

    for (;;)
    {
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &timeinfo);

        strftime(buff, sizeof(buff), "%H:%M:%S", &timeinfo);

        lvgl_port_lock();
        lv_label_set_text(label, buff);
        lvgl_port_unlock();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    lv_port_disp_init();

    twi_init();
    ina_init();

    static StackType_t main_task_stack[MAIN_TASK_STACK_DEPTH];
    static StaticTask_t main_task_tcb;
    static StackType_t time_task_stack[TIME_TASK_STACK_DEPTH];
    static StaticTask_t time_task_tcb;

    (void)xTaskCreateStatic(
        main_task,
        "main_task",
        MAIN_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1,
        main_task_stack,
        &main_task_tcb);

    (void)xTaskCreateStatic(
        time_task,
        "time_task",
        TIME_TASK_STACK_DEPTH,
        NULL,
        tskIDLE_PRIORITY + 1,
        time_task_stack,
        &time_task_tcb);
}
