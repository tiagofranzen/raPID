#include "../teensy3/Arduino.h"
#include "../teensy3/WProgram.h"

#include <stdint.h>

#include "Servo.h"
#include "I2Cdev.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif


#include "../include/error_handling.h"
#include "../include/settings.h"
#include "../include/pins.h"
#include "../include/serial_debug.h"
#include "../include/read_receiver.h"
#include "../include/imu.h"
#include "../include/pid.h"
#include "../include/watchdog.h"
#include "../include/rc_control.h"
#include "../include/state.h"
#include "../include/state_change.h"
#include "pid_coefficients.h"

#define TIMING_ANALYSIS
#ifdef TIMING_ANALYSIS
#define time(f) {\
    digitalWrite(DEBUG_PIN, HIGH); \
    f;\
    digitalWrite(DEBUG_PIN, LOW);\
}
#else
#define time(f) f
#endif

#define notime(f) f

static bool blink_state = false;

/*
   ——————————————————————————————————————————————
   ———             HARDWARE SETUP             ———
   ——————————————————————————————————————————————

     MPU6050 Breakout ----- Teensy 3.2
     3.3V ----------------- 3.3V
     GND ------------------ GND
     SDA ------------------ A4/pin 18
     SCL ------------------ A5/pin 19
     INT ------------------ Digital Pin 12 (see pins.h)

   PPM from RC RX go to pins 8, 9, 10, 11 (see pins.h)
   Output PPM to ESC's: pins 21, 22
*/

state_t state;

/* Scaled yaw_pitch_roll to [0, 1000]
 * [yaw, pitch, roll]
 */
int16_t attitude[3] = { 0 };

/* Angular Rates
 * [yaw_rate, pitch_rate, roll_rate]
 */
int16_t gyro_axis[3] = { 0 };


double pid_output_roll = 0.0;
double pid_output_roll_rate = 0.0;

Servo left_ppm;
Servo right_ppm;

uint16_t left_throttle;
uint16_t right_throttle;

uint16_t receiver_in[NUM_CHANNELS] = { 0 };

size_t flight_mode_index = 0;

extern "C" int main(void) {
    serial_begin(9600);

    pinMode(LED_PIN, OUTPUT);
    pinMode(DEBUG_PIN, OUTPUT);

    left_ppm.attach(LEFT_SERVO_PIN);
    right_ppm.attach(RIGHT_SERVO_PIN);

    time(arm_ESC());

    init_rx_interrupts();

    init_mpu6050();

    init_watchdog();

    init_pid_coefficients();

    state = DISARMED;
    while (1) {
        switch (state) {
        case ARMED:
            notime(read_receiver());

            notime(read_abs_angles(attitude));

            notime(read_angular_rates(gyro_axis));

            /*
            Serial.print(receiver_in[ROLL_CHANNEL] - 1500);
            Serial.print('\t');
            Serial.print(attitude[ROLL_ANGLE]);
            Serial.print('\t');
            Serial.println(gyro_axis[ROLL_RATE]);
            */

            notime(calculate_PID_stabilize(receiver_in[ROLL_CHANNEL] - 1000,
                                           attitude[ROLL_ANGLE], gyro_axis[ROLL_RATE]));


            //setpoint_rate = receiver_in[ROLL_CHANNEL] - 1500.0;

            notime(calculate_PID_rate(-15 * pid_output_roll, gyro_axis[ROLL_RATE]));

            left_throttle  = receiver_in[THROTTLE_CHANNEL] + pid_output_roll_rate;
            right_throttle = receiver_in[THROTTLE_CHANNEL] - pid_output_roll_rate;

            left_throttle = left_throttle < 1000 ? 1000 : left_throttle;
            right_throttle = right_throttle < 1000 ? 1000 : right_throttle;

            left_throttle = left_throttle > 2000 ? 2000 : left_throttle;
            right_throttle = right_throttle > 2000 ? 2000 : right_throttle;

            left_ppm.writeMicroseconds(left_throttle);
            right_ppm.writeMicroseconds(right_throttle);

            #define DEBUG_COL
#ifdef DEBUG_COL
            serial_print("thr:");
            serial_print(receiver_in[THROTTLE_CHANNEL]);
            serial_print("\tsetp:");
            serial_print(pid_output_roll);
            serial_print("\tr-angl:");
            serial_print(attitude[ROLL_ANGLE]);
            serial_print("\tleft:");
            serial_print(left_throttle);
            serial_print("\tright:");
            serial_print(right_throttle);
            serial_print("\tr-p-out:");
            serial_print(pid_output_roll);
            serial_print("\tr-p_rate-out:");
            serial_println(pid_output_roll_rate);
#endif

            if (check_disarm_status()) {
                break;
            }
            break;
        case DISARMED:
            notime(read_receiver());
            notime(read_abs_angles(attitude));

            if (check_arm_status()) {
                break;
            }
            break;
        case CONFIG:
            Serial.println("CONFIG!");
            state = DISARMED;
            break;
        default:
            Serial.println("Unimplemented state! Will disarm.");
            disarm();
            break;
        }
        /* Blink LED to indicate activity */
        blink_state = !blink_state;
        digitalWrite(LED_PIN, blink_state);
        feed_the_dog();
    }
}