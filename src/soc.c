#include "soc.h"
#include "math.h"


float soc_lookup(float voltage){
    if(voltage >= 4.15) return 100;
    else if((voltage >= 4.11) && (voltage < 4.15)) return 95;
    else if((voltage >= 4.08) && (voltage < 4.11)) return 90;
    else if((voltage >= 4.02) && (voltage < 4.08)) return 85;
    else if((voltage >= 3.98) && (voltage < 4.02)) return 80;
    else if((voltage >= 3.95) && (voltage < 3.98)) return 75;
    else if((voltage >= 3.91) && (voltage < 3.95)) return 70;
    else if((voltage >= 3.87) && (voltage < 3.91)) return 65;
    else if((voltage >= 3.85) && (voltage < 3.87)) return 60;
    else if((voltage >= 3.84) && (voltage < 3.85)) return 55;
    else if((voltage >= 3.82) && (voltage < 3.84)) return 50;
    else if((voltage >= 3.80) && (voltage < 3.82)) return 45;
    else if((voltage >= 3.79) && (voltage < 3.80)) return 40;
    else if((voltage >= 3.77) && (voltage < 3.79)) return 35;
    else if((voltage >= 3.75) && (voltage < 3.77)) return 30;
    else if((voltage >= 3.73) && (voltage < 3.75)) return 25;
    else if((voltage >= 3.71) && (voltage < 3.73)) return 20;
    else if((voltage >= 3.69) && (voltage < 3.71)) return 15;
    else if((voltage >= 3.61) && (voltage < 3.69)) return 10;
    else return 0;

}
double soc_init(float voltage){
    return soc_lookup(voltage);
}
double soc_cumsum(double previous_soc, float current){
    // + A/(As)(s)
    return (previous_soc - 200 * current / battery_cell_capacity);
}

double soc_cumsum_time(double previous_soc, float current, float time_elapsed){
    // + A/(As)(s)
    return (previous_soc - time_elapsed * current / battery_cell_capacity / 1000);
}