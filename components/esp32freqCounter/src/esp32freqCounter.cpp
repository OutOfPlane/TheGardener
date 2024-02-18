#include "esp32freqCounter.hpp"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_timer.h"
#include "soc/pcnt_struct.h"

using namespace gardener;
pcnt_isr_handle_t user_isr_handle = NULL; // user's ISR service handle

// static void IRAM_ATTR pcnt_intr_handler(void *arg)
// {
//     const char *_name = "PCNT ISR";
//     uint32_t intr_status = PCNT.int_st.val;

//     for (int i = 0; i < PCNT_UNIT_MAX; i++)
//     {
//         if (intr_status & (BIT(i)))
//         {
//             //G_LOGI("PCNT: %08X", PCNT.status_unit[i].val);
//             PCNT.int_clr.val = BIT(i);
//         }
//     }
// }

esp32freqCounter::esp32freqCounter(const char *name, uint8_t pinNumber)
    : gardenObject(name)
{
    if (pinNumber < GPIO_NUM_MAX)
    {
        /* Prepare configuration for the PCNT unit */
        pcnt_config_t pcnt_config = {
            // Set PCNT input signal and control GPIOs
            .pulse_gpio_num = pinNumber,
            .ctrl_gpio_num = -1,
            .lctrl_mode = PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            .hctrl_mode = PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            // What to do on the positive / negative edge of pulse input?
            .pos_mode = PCNT_COUNT_INC, // Count up on the positive edge
            .neg_mode = PCNT_COUNT_DIS, // Keep the counter value on the negative edge
            // Set the maximum limit value
            .counter_h_lim = 32767,
            .counter_l_lim = -32767,

            .unit = PCNT_UNIT_0,
            .channel = PCNT_CHANNEL_0
        };
        /* Initialize PCNT unit */
        pcnt_unit_config(&pcnt_config);

        /* Configure and enable the input filter */
        // pcnt_set_filter_value(PCNT_TEST_UNIT, 100);
        // pcnt_filter_enable(PCNT_TEST_UNIT);

        /* Set threshold 0 and 1 values and enable events to watch */
        /* Enable events on zero, maximum and minimum limit values */
       
        // pcnt_event_enable(PCNT_TEST_UNIT, PCNT_EVT_L_LIM);

        /* Initialize PCNT's counter */
        pcnt_counter_pause(PCNT_UNIT_0);
        pcnt_counter_clear(PCNT_UNIT_0);

        /* Register ISR handler and enable interrupts for PCNT unit */
       // pcnt_isr_register(pcnt_intr_handler, NULL, 0, &user_isr_handle);
        //pcnt_intr_enable(PCNT_UNIT_0);

        /* Everything is set up, now go to counting */
        pcnt_counter_resume(PCNT_UNIT_0);
    }
}

esp32freqCounter::~esp32freqCounter()
{
}

g_err gardener::esp32freqCounter::get(uint8_t &val)
{
    int16_t cnt;
    pcnt_get_counter_value(PCNT_UNIT_0, &cnt);
    val = 0;

    if(!_lastUpdate)
    {
        pcnt_counter_clear(PCNT_UNIT_0);
        _lastUpdate = esp_timer_get_time();
        return G_OK;
    }

    int64_t delta = esp_timer_get_time() - _lastUpdate;
    
    int64_t pulsePerSecond = (cnt * 1000000)/delta;

    G_LOGI("Freq: %lld Hz", pulsePerSecond);
    pcnt_counter_clear(PCNT_UNIT_0);
    _lastUpdate = esp_timer_get_time();

    val = 0;
    return G_OK;
}
