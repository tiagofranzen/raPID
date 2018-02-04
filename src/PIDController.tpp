template<typename T>
PIDController<T>::PIDController(PIDParams<T> *params)
    : enabled ( true )

    , p_gain ( params->p_gain )
    , i_gain ( params->i_gain )
    , d_gain ( params->d_gain )

    , integral       ( 0 )
    , integral_limit ( params->integral_limit )

    , derivative     ( 0 )
    , last_error     ( 0 )
    , last_setpoint  ( 0 )
    , last_measured  ( 0 )

    , output_limit   ( params->output_limit )

    , last_time      ( 0 ) {}

template<typename T>
void PIDController<T>::set_enabled(bool enable) {
    enabled = enable;
}

template<typename T>
T PIDController<T>::compute(const T measured, const T setpoint) {
    if (!enabled) {
        return setpoint;
    }

    /* TODO: handle overflows for 'now' using rollover or somesuch. */
    uint64_t now = millis();

    uint32_t elapsed_time = now - last_time;

    T error = measured - setpoint;

    /* Give me some P! */
    /* Proportional term */
    T p_term = this->p_gain * error;

    /* Give me some I! */
    /* Integral term */
    this->integral += elapsed_time * error * this->i_gain;
    /* Integral windup limit */
    clamp(integral, -integral_limit, integral_limit);

    /* Give me some D! */
    T d_term = 0;
    switch (d_type) {
        case ERROR:
            /* Derivative term on error */
            d_term = ((error - last_error) / elapsed_time) * d_gain;
            break;

        case SETPOINT:
            /* Derivative term on setpoint */
            d_term = ((setpoint - last_setpoint) / elapsed_time) * d_gain;
            break;

        case MEASURED:
            /* Derivative term on measurement */
            d_term = ((measured - last_measured) / elapsed_time) * d_gain;
            break;
    }

    if (derivative_filter_enabled) {
        d_term = deriv_filter.next(d_term);
    }

    this->last_time     = now;

    this->last_error    = error;
    this->last_setpoint = setpoint;
    this->last_measured = measured;

    T result = p_term + integral + d_term;

    /* Output limit */
    clamp(result, -output_limit, output_limit);

    return result;
}

template<typename T>
void PIDController<T>::set_p(const T _p_gain) {
    this->p_gain = _p_gain;
}

template<typename T>
void PIDController<T>::set_i(const T _i_gain) {
    this->i_gain = _i_gain;
}

template<typename T>
void PIDController<T>::set_d(const T _d_gain) {
    this->d_gain = _d_gain;
}

template<typename T>
void PIDController<T>::set_params(const PIDParams<T> *params) {
    p_gain = params->p_gain;
    i_gain = params->i_gain;
    d_gain = params->d_gain;
}

template<typename T>
void PIDController<T>::integral_reset() {
    integral = 0;
}

template<typename T>
void PIDController<T>::set_derivative_type(derivative_type type) {
    this->d_type = type;
}

template<typename T>
void PIDController<T>::enable_derivative_filter(bool enable) {
    this->derivative_filter_enabled = enable;
}