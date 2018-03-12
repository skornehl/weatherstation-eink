// WIFI
#define WIFI_SSID "SSID"
#define WIFI_PASS "key"

// MQTT
MQTTClient mqtt;
const String MQTT_SERVER  = "192.168.188.20";
const String MQTT_PORT    = "1883";

const String bedroomTopic     = "tele/schlafzimmer/SENSOR";
const String livingroomTopic  = "tele/wohnzimmer/SENSOR";
const String nurseryTopic     = "tele/kinderzimmer/SENSOR";
const String kitchenTopic     = "tele/kueche/schrank/SENSOR";

// OpenWeather
String APP_ID = "key";

// Field size {x, y, w, h}
const int openWeatherBox[]  = {35, 15, 230, 130};
const int livingroomBox[]   = {270, 3, 120, 90};
const int bedroomBox[]      = {5, 152, 120, 90};
const int nurseryBox[]      = {140, 152, 120, 90};
const int kitchenBox[]      = {270, 152, 120, 90};
const int clockBox[]        = {5, 250, 150, 45};
const int wifiBox[]         = {343, 250, 50, 45};

// Display
GxIO_Class io(SPI, SS, 0, 2);  //SPI,SS,DC,RST
GxEPD_Class display(io, 2, 4);  //io,RST,BUSY

// Other
const String hostname = "WeatherStation01";

int minute = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float bedroomData[]     = {0, 0};
float livingroomData[]  = {0, 0};
float nurseryData[]     = {0, 0};
float kitchenData[]     = {0, 0};
