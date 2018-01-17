#include "../include/Output.h"

/* TODO implement inverting output */
/* TODO implement arming for ESC type? */
/* TODO implement lower and upper limit everywhere */
/* TODO implement flight mode offset? */
/* TODO remove includes  when not using warning output in apply(x) */

static const uint16_t THROTTLE_LOW_CUTOFF = 25;

#define clamp(value, low, high) \
    ((value) = \
    ((value) < (low)  ? (low) : \
    ((value) > (high) ? (high) : (value))))

Output::Output(out_type_t type, uint8_t pin)
    : out_type    { type }
    , pin         { pin }
    , mixer       { 0.0, 0.0, 0.0, 0.0 }
    , inverted    { false }
    , upper_limit { 1000 }
    , lower_limit { 0 }
{
    output.attach(pin);
}

void Output::invert_servo_direction() {
    if(out_type == SERVO) {
        inverted = !inverted;
    }
}

bool Output::is_inverted() {
    return inverted;
}

void Output::shut_off() {
    output.writeMicroseconds(1000 + lower_limit);
}

void Output::apply(uint16_t throttle,
        float roll_stbl, float pitch_stbl, float yaw_stbl
        /*,float roll_rate, float pitch_rate, float yaw_rate*/) {

    /* Throttle cutoff to avoid spinning props due to movement when throttle is low but state is armed */
    if (out_type == ESC && throttle < THROTTLE_LOW_CUTOFF) {
        output.writeMicroseconds(1000 + lower_limit);
        return;
    }

    // TODO take this out to settings init
    if (mixer.throttle_volume > 1.0 || mixer.throttle_volume < -1.0 ||
        mixer.roll_volume     > 1.0 || mixer.roll_volume     < -1.0 ||
        mixer.pitch_volume    > 1.0 || mixer.pitch_volume    < -1.0 ||
        mixer.yaw_volume      > 1.0 || mixer.yaw_volume      < -1.0) {

        Serial.println("one of the volume parameters is out of range of [-1.0, 1.0]");

        clamp(mixer.throttle_volume, - 1.0, 1.0);
        clamp(mixer.roll_volume,     - 1.0, 1.0);
        clamp(mixer.pitch_volume,    - 1.0, 1.0);
        clamp(mixer.yaw_volume,      - 1.0, 1.0);
    }

    throttle =  (uint16_t) (throttle   * mixer.throttle_volume);

    throttle += (uint16_t) (roll_stbl  * mixer.roll_volume);

    throttle += (uint16_t) (pitch_stbl * mixer.pitch_volume);

    throttle += (uint16_t) (yaw_stbl   * mixer.yaw_volume);

    clamp(throttle, lower_limit, upper_limit);

    output.writeMicroseconds(1000 + throttle);
}

Output *Output::set_throttle_volume(float volume) {
    mixer.throttle_volume = volume;
    return this;
}

Output *Output::set_roll_volume(float volume) {
    mixer.roll_volume = volume;
    return this;
}

Output *Output::set_pitch_volume(float volume) {
    mixer.pitch_volume = volume;
    return this;
}

Output *Output::set_yaw_volume(float volume) {
    mixer.yaw_volume = volume;
    return this;
}
