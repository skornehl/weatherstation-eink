// WIFI
#define WIFI_SSID "SSID"
#define WIFI_PASS "key"

// MQTT
String MQTT_SERVER  = "192.168.188.20";
String MQTT_PORT    = "1883";

String bedroomTopic     = "tele/schlafzimmer/SENSOR";
String livingroomTopic  = "tele/wohnzimmer/SENSOR";
String nurseryTopic     = "tele/kinderzimmer/SENSOR";
String kitchenTopic     = "tele/kueche/schrank/SENSOR";

// OpenWeather
String APP_ID = "key";

// Field size {x, y, w, h}
int openWeatherBox[]  = {35, 15, 230, 120};
int livingroomBox[]   = {270, 3, 120, 90};
int bedroomBox[]      = {5, 152, 120, 90};
int nurseryBox[]      = {140, 152, 120, 90};
int kitchenBox[]      = {270, 152, 120, 90};
int clockBox[]        = {5, 250, 150, 45};
int wifiBox[]         = {343, 250, 50, 45};
