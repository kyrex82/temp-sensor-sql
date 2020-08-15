#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_

typedef enum timeFormat {
	CELCIUS,
	FARENHEIT
} timeFormat;

struct user_options {
	int NUM_SECONDS;
	int sensor1Enabled;
	int sensor2Enabled;
	int temperatureAlarmMin;
	int temperatureAlarmMax;
	int notifyPerioidMinutes;
};


#endif /* TEMP_SENS_H_ */
