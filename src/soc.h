

#define battery_cell_capacity 13320 //in As
#define max_voltage 4.2f
#define cut_off_voltage 2.75f

float soc_lookup(float voltage);
double soc_init(float voltage);
double soc_cumsum(double previous_soc, float current);
double soc_cumsum_time(double previous_soc, float current, float time_elapsed);