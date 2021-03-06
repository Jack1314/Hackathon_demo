/**
 * LED Matrix library for http://www.seeedstudio.com/depot/ultrathin-16x32-red-led-matrix-panel-p-1582.html
 * The LED Matrix panel has 32x16 pixels. Several panel can be combined together as a large screen.
 *
 * Coordinate & Connection (mbed -> panel 0 -> panel 1 -> ...)
 *   (0, 0)                                     (0, 0)
 *     +--------+--------+--------+               +--------+--------+
 *     |   5    |    3   |    1   |               |    1   |    0   |
 *     |        |        |        |               |        |        |<----- mbed
 *     +--------+--------+--------+               +--------+--------+
 *     |   4    |    2   |    0   |                              (64, 16)
 *     |        |        |        |<----- mbed
 *     +--------+--------+--------+
 *                             (96, 32)
 *
 */


#include "mbed.h"
#include "matrix.h"
#include "stm32f1xx.h"
#include "cmsis_nvic.h"
#include "font.h"
#include "I2CSlave.h"


#define WIDTH   64
#define HEIGHT  32

#define SLAVE_I2C_ADDRESS 0x52

//I2C i2c_port(PB_11, PB_10);

//I2CSlave slave(I2C_SDA, I2C_SCL);
//I2CSlave slave(PB_11, PB_10);

//Serial uart(PB_10, PB_11);
Matrix matrix;
uint32_t *red_matrix_buf;
uint32_t *green_matrix_buf;
TIM_HandleTypeDef timer_handle;

const uint8_t love[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1E, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1F, 0x7F, 0x8F, 0x80, 0x00,
0x00, 0x00, 0x3F, 0x3E, 0x7F, 0x9F, 0xC0, 0x00, 0x00, 0x00, 0x7F, 0xBE, 0x7F, 0xFF, 0xE0, 0x00,
0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x7F, 0xFF, 0xE0, 0x00,
0x00, 0x00, 0x7F, 0xFF, 0x3F, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0x3F, 0xFF, 0xC0, 0x00,
0x00, 0x00, 0x3F, 0xFF, 0x9F, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x1F, 0xFF, 0x0F, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x0F, 0xFF, 0x0F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFE, 0x07, 0xFC, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFC, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x01, 0xC0, 0x00, 0x00,
0x00, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t love_2[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xFC, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x7F, 0x80, 0x00, 0x00,
0x00, 0x00, 0x01, 0xF8, 0xFF, 0x80, 0xF8, 0x00, 0x00, 0x0F, 0x03, 0xF9, 0xFF, 0xC1, 0xFC, 0x00,
0x00, 0x1F, 0xC3, 0xF9, 0xFF, 0xC7, 0xFE, 0x00, 0x00, 0x3F, 0xE7, 0xF1, 0xFF, 0xEF, 0xFF, 0x00,
0x00, 0x7F, 0xF7, 0xF9, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0x00,
0x00, 0x7F, 0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0x00,
0x00, 0x7F, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0xFC, 0x7F, 0xFF, 0xFE, 0x00,
0x00, 0x3F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x00, 0x00, 0x1F, 0xFF, 0xFE, 0x3F, 0xFF, 0xFC, 0x00,
0x00, 0x0F, 0xFF, 0xFC, 0x1F, 0xFF, 0xF8, 0x00, 0x00, 0x07, 0xFF, 0xFC, 0x0F, 0xFF, 0xE0, 0x00,
0x00, 0x03, 0xFF, 0xF8, 0x0F, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0xFF, 0xF0, 0x07, 0xFF, 0x80, 0x00,
0x00, 0x00, 0x3F, 0xF0, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x01, 0xF8, 0x00, 0x00,
0x00, 0x00, 0x03, 0xC0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x40, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t logo1 [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x00, 0x00, 0x00, 0x40, 0x60, 0x00, 0x00, 0x00,
0x40, 0x20, 0x00, 0x00, 0x00, 0xC0, 0x30, 0x00, 0x00, 0x01, 0xC0, 0x30, 0x00, 0x00, 0x01, 0x80,
0x38, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x00, 0x03, 0x80, 0x18, 0x08, 0x01, 0x03, 0x80, 0x1C,
0x08, 0x01, 0x03, 0x80, 0x1C, 0x08, 0x01, 0x03, 0x80, 0x1C, 0x08, 0x01, 0x03, 0x80, 0x1C, 0x18,
0x01, 0x83, 0x80, 0x1C, 0x18, 0x01, 0x83, 0x80, 0x18, 0x18, 0x01, 0xC1, 0x80, 0x18, 0x38, 0x01,
0xC1, 0x80, 0x18, 0x30, 0x00, 0xE1, 0x80, 0x30, 0x70, 0x00, 0xF0, 0xC0, 0x30, 0xE0, 0x00, 0x70,
0xC0, 0x20, 0xE0, 0x00, 0x38, 0x40, 0x61, 0xC0, 0x00, 0x1C, 0x20, 0x43, 0x80, 0x00, 0x0E, 0x00,
0x07, 0x00, 0x00, 0x07, 0x00, 0x0E, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x07, 0xFF, 0xFF, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF
};

const uint8_t logo2 [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x60, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x30, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x38, 0x20, 0x00, 0x00,
0x00, 0x00, 0x18, 0x70, 0x38, 0x20, 0x00, 0x00, 0x00, 0x00, 0x18, 0x70, 0x38, 0x20, 0x00, 0x00,
0x00, 0x00, 0x18, 0x70, 0x38, 0x60, 0x00, 0x00, 0x00, 0x00, 0x18, 0x70, 0x38, 0x60, 0x00, 0x00,
0x00, 0x00, 0x1C, 0x70, 0x38, 0x60, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x70, 0x38, 0xE0, 0x00, 0x00,
0x00, 0x00, 0x0E, 0x30, 0x38, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x30, 0x31, 0xC0, 0x00, 0x00,
0x00, 0x00, 0x07, 0x18, 0x33, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x98, 0x67, 0x80, 0x00, 0x00,
0x00, 0x00, 0x01, 0xC8, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x1E, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x30, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void tick_init(uint32_t period_us);

void tick_handler()
{
    // Clear Update interrupt flag
    __HAL_TIM_CLEAR_FLAG(&timer_handle, TIM_FLAG_UPDATE);
        
    matrix.scan();
}

int main()
{
	char messageBuf[4];
	const char data[4] = {0x01, 0x11};
	uint8_t i = 0;
//	slave.address(SLAVE_I2C_ADDRESS);
	matrix.begin(WIDTH, HEIGHT);
    matrix.setBrightness(2);
    
    red_matrix_buf = matrix.getRedBuffer();
    green_matrix_buf = matrix.getGreenBuffer();
    
    tick_init(1000);
    
//     wait(3);
//     matrix.clear();
    
//     matrix.drawImage(40, 0, 8, 16, digitals + 16 * 8);
//     matrix.requestUpdate();
    
    uint8_t count = 0;
    const char *digitals = "0123456789";
	const char *seeed = "Seeed";
    while(1) {
/*		
		i2c_port.start();

	//	i2c_port.write(0xAA, data, 2, 0);

	    i2c_port.write(0xAA);

	    i2c_port.write(0x00);

	    i2c_port.stop();
*/

/*		
		int i = slave.receive();

        switch (i) {

        case I2CSlave::ReadAddressed:

            matrix.line(0, 28, 30, 28, 1, green_matrix_buf);
			matrix.requestUpdate();
			
            break;

        case I2CSlave::WriteGeneral:

            matrix.line(0, 27, 30, 27, 1, green_matrix_buf);
			matrix.requestUpdate();

            break;

        case I2CSlave::WriteAddressed:

            matrix.line(0, 26, 30, 26, 1, green_matrix_buf);
			matrix.requestUpdate();

            break;

		default:
			matrix.drawImage(32, 0, 32, 32, sad2, red_matrix_buf);
      	  matrix.requestUpdate();
        

			break;

        }

		slave.read(messageBuf, 2);
		switch( messageBuf[0] )
			 {

	        case 0x00:

	            matrix.line(0, 28, 30, 28, 1, green_matrix_buf);
				matrix.requestUpdate();
				
	            break;

	        case 0x01:

	            matrix.line(0, 27, 30, 27, 1, green_matrix_buf);
				matrix.requestUpdate();

	            break;

	        case 0x02:

	            matrix.line(0, 26, 30, 26, 1, green_matrix_buf);
				matrix.requestUpdate();

	            break;

			default:
				matrix.drawImage(32, 0, 32, 32, sad2, red_matrix_buf);
	      	  matrix.requestUpdate();
	        

				break;

			}
*/

/*
        //matrix.putChar(40, 0,  count, FONT_6X8, red_matrix_buf);
        matrix.putString(32, 8, digitals + count, FONT_6X8, red_matrix_buf);
        matrix.putChar(8, 2, 'a' + count, FONT_6X8, green_matrix_buf);
 //       matrix.putChar(16, 16, 'k' + count, FONT_6X8, green_matrix_buf);
 		matrix.putChar(16, 16, '0' + count, FONT_6X8, red_matrix_buf);
// 		matrix.line(0, 28, 30, 28, 1, green_matrix_buf);
		matrix.line(0, 25, 30, 28, 1, green_matrix_buf);
		matrix.circle(50, 24, 3, 1, green_matrix_buf);
		matrix.fillRect(20, 1, 10, 5, 1, green_matrix_buf);
*/
		
//		matrix.drawImage(10, 0, 44, 32, logo1, green_matrix_buf);
//        matrix.requestUpdate();
        
//        wait(1.0);

		for(i = 1; i < 23; i++)
		{
			matrix.drawImage(0, 0, 64, i, logo2, green_matrix_buf);
	        matrix.requestUpdate();
	        wait(0.1);
		}

//		matrix.drawImage(0, 0, 64, 22, logo2, green_matrix_buf);
		matrix.putString(16, 24, seeed, FONT_6X8, green_matrix_buf);
		matrix.requestUpdate();
		wait(1.5);
		
		matrix.clear();

		matrix.drawImage(0, 0, 64, 32, love, red_matrix_buf);
//		matrix.drawImage(0, 0, 32, 32, sad, green_matrix_buf);
//		matrix.drawImage(32, 0, 32, 32, sad2, red_matrix_buf);
        matrix.requestUpdate();
        
        wait(1.0);

		matrix.clear();
		
		matrix.drawImage(0, 0, 64, 32, love_2, red_matrix_buf);
        matrix.requestUpdate();
        wait(0.5);
		matrix.clear();
		wait(0.5);
		matrix.drawImage(0, 0, 64, 32, love_2, red_matrix_buf);
        matrix.requestUpdate();
        wait(0.5);
		matrix.clear();
		wait(0.5);
		matrix.drawImage(0, 0, 64, 32, love_2, red_matrix_buf);
        matrix.requestUpdate();
        wait(0.5);
		matrix.clear();

		matrix.putString(14, 8, "WRB", FONT_12X16, green_matrix_buf);
		matrix.requestUpdate();
        wait(2.0);
				matrix.clear();

		wait(5);
//        count = (count + 1) & 0x7;

    }
}
		  
void tick_init(uint32_t period_us)
{
    // Enable timer clock
    __TIM2_CLK_ENABLE();

    // Reset timer
    __TIM2_FORCE_RESET();
    __TIM2_RELEASE_RESET();

    // Update the SystemCoreClock variable
    SystemCoreClockUpdate();

    // Configure time base
    timer_handle.Instance = TIM2;
    timer_handle.Init.Period        = period_us;
    timer_handle.Init.Prescaler     = (uint32_t)(SystemCoreClock / 1000000) - 1; // 1 us tick
    timer_handle.Init.ClockDivision = 0;
    timer_handle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&timer_handle);
    
    __HAL_TIM_SetAutoreload(&timer_handle, period_us);


    // Configure interrupts
    // Update interrupt used for 32-bit counter
    NVIC_SetVector(TIM2_IRQn, (uint32_t)tick_handler);
    NVIC_EnableIRQ(TIM2_IRQn);

    // Enable interrupts
    __HAL_TIM_ENABLE_IT(&timer_handle, TIM_IT_UPDATE); // For 32-bit counter

    // Enable timer
    HAL_TIM_Base_Start(&timer_handle);
}

