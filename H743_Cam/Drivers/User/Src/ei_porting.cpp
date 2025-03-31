#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "dcmi_ov2640.h"  

#include "ei_classifier_porting.h"
#include "ei_classifier_types.h"
#include "ei_run_classifier.h"
#include "model_metadata.h"
#include "numpy_types.h"


extern EI_IMPULSE_ERROR ei_sleep(int32_t time_ms);


/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
uint16_t *image_buf = (uint16_t *)Camera_BufferB;
uint8_t *snapshot_buf = (uint8_t *)Camera_BufferC;

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);


extern "C"
{
	void ei_printf(char *format, ...)
	{
		va_list myargs;
		va_start(myargs, format);
		vprintf(format, myargs);
		va_end(myargs);
	}
	
	void ei_edge_impulse()
	{
		// convert RGB565 to RGB888
		for(int i = 0; i < EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT; i++)
		{
        uint16_t pixel = image_buf[i];
        snapshot_buf[3*i]   = (pixel >> 11) << 3;  					// R
        snapshot_buf[3*i+1] = ((pixel >> 5) & 0x3F) << 2;		// G
        snapshot_buf[3*i+2] = (pixel & 0x1F) << 3;					// B
    }
		
		ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    /*EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK)
		{
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // print the predictions
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);*/
	}
}

void *ei_malloc(size_t size)
{
	return pvPortMalloc(size);
}
void *ei_calloc(size_t nitems, size_t size)
{
	return pvPortCalloc(nitems, size);
}
void ei_free(void *ptr)
{
	vPortFree(ptr);
}
uint64_t ei_read_timer_ms()
{
    return (uint64_t)xTaskGetTickCount();
}
uint64_t ei_read_timer_us()
{
	return (xTaskGetTickCount() * 1000);
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    // we already have a RGB888 buffer, so recalculate offset into pixel index
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // Swap BGR to RGB here
        // due to https://github.com/espressif/esp32-camera/issues/379
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];

        // go to the next pixel
        out_ptr_ix++;
        pixel_ix+=3;
        pixels_left--;
    }
    // and done!
    return 0;
}
