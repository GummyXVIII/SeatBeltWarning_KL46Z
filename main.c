#include "MKL46Z4.h"


volatile uint32_t ms_counter = 0;     
volatile uint8_t timer_start = 0;     
volatile uint32_t seat_timer = 0;     
volatile uint32_t led_blink_counter = 0;
//======================================================
//  (PTE29)
//======================================================
void init_LED_Red() {
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;// ENABLE clock circuit
    PORTE->PCR[29] = (1 << 8);         // GPIO
    PTE->PDDR |= (1 << 29);            // output
    PTE->PSOR = (1 << 29);             // OFF
}

//======================================================
//  (PTD5)
//======================================================
void init_LED_Green() {
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;// ENABLE clock circuit
    PORTD->PCR[5] = (1 << 8);					 // GPIO
    PTD->PDDR |= (1 << 5);						 // output
    PTD->PSOR = (1 << 5);              // OFF
}

//======================================================
// SW1 – PTC3 (INPUT, PULLUP)
//======================================================
void init_SW1() {
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;						 // ENABLE clock
		PORTC->PCR[3] = (1 << 8) | (1 << 1) | (1 << 0);// GPIO : MUX=001, PE=1, PS=1
    PTC->PDDR &= ~(1 << 3);                        // input
}

//======================================================
// SW3 – PTC12 (INPUT, PULLUP)
//======================================================
void init_SW3() {
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;							// ENABLE clock
	  PORTC->PCR[12] = (1 << 8) | (1 << 1) | (1 << 0);// GPIO 
    PTC->PDDR &= ~(1 << 12);          						  // input
}

//======================================================
//  SYSTICK (1ms)
//======================================================
void init_SysTick() {
    SysTick->LOAD = (SystemCoreClock / 1000) - 1; // 1ms interrupt
    SysTick->VAL = 0;
    SysTick->CTRL = 7; // enable interrupt + enable systick + use cpu clock
}

//======================================================
// SYSTICK HANDLER — TANG ms_counter
//======================================================
void SysTick_Handler() {
    ms_counter++;

    if (timer_start) {
        seat_timer++;
    }
}

//======================================================
// POLLING + LOGIC 
//======================================================
void polling_loop() {
    while (1) {

        int sw1 = (PTC->PDIR & (1 << 3)) ? 1 : 0;     // SW1: seating
        int sw3 = (PTC->PDIR & (1 << 12)) ? 1 : 0;    // SW3: safety seat belt

        //======================================================
        // 1. Nobody seat
        //======================================================
        if (sw1 == 1) {
            timer_start = 0;
            seat_timer = 0;

            PTE->PSOR = (1 << 29);   // RED LED OFF
            PTD->PSOR = (1 << 5);    // GREEN LED OFF
            continue;
        }

        //======================================================
        // 2. SET 3 SECOND COUNTDOWN TIMER
        //======================================================
        if (timer_start == 0) {
            timer_start = 1;
            seat_timer = 0;     // reset timer if new people seated
        }

        //======================================================
        // 3. SEAT BELT SIGNAL IS ACTIVE (sw3 == 0)
        //======================================================
        if (sw3 == 0) {
            timer_start = 0;         
            seat_timer = 0;

            PTE->PSOR = (1 << 29);   // RED LED OFF
            PTD->PCOR = (1 << 5);    // GREEN LED ON
            continue;
        }

        //======================================================
        // 4. The person is seated but has not fastened the seat belt
        //    ACTIVE THE WARNING when seat_timer >= 3000 ms
        //======================================================
        if (sw3 == 1) {

						PTD->PSOR = (1 << 5);       // GREEN LED OFF

				if (seat_timer >= 3000) {       // 3 second
						led_blink_counter++;
        if (led_blink_counter >= 500) { // 500ms 
            PTE->PTOR = (1 << 29);      // toggle RED LED 
            led_blink_counter = 0;
					}
				}
				else {
					PTE->PSOR = (1 << 29);   			// RED LED OFF
					led_blink_counter = 0;
					}
				}
			}
		}
//======================================================
// MAIN
//======================================================
int main() {

    init_LED_Red();
    init_LED_Green();
    init_SW1();
    init_SW3();
    init_SysTick();

    polling_loop();
}
