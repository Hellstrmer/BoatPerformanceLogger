#include "analog.h"

static const int PIN_BATT = 4;
static const float DIV = (39.0f + 10.0f) / 10.0f;                // Spänningsdelare 39k + 10k, dividera med 10k för att få spänning på batteriet
static const float DIV_WATER_PRESSURE = (10.0f + 18.0f) / 18.0f; // Spänningsdelare 10k + 18k, dividera med 18k för att få spänning på vattenpressure
static const float DIV_WATER_TEMP = (10.0f + 18.0f) / 18.0f;     // Spänningsdelare 10k + 18k, dividera med 18k för att få spänning på vattentemp
static const int N_SAMPLES = 16;
static const float ALPHA_BATT = 0.2f; // Utjämning, 1 = ingen utjämning, 0 = max utjämning

// Kalibrering Trim
static const int PIN_TRIM = 2;
static const float TRIM_RATIO_DOWN = 0.05f;
static const float TRIM_RATIO_UP = 0.45f;
static const float TRIM_VAL_DOWN = 0.0f;
static const float TRIM_VAL_UP = 100.0f;
static const float TRIM_ALPHA = 0.2f; // Utjämning, 1 = ingen utjämning, 0 = max utjämning
// Kalibrering Bränsle
static const int PIN_FUEL = 1;
static const float FUEL_RATIO_DOWN = 0.05f;
static const float FUEL_RATIO_UP = 0.45f;
static const float FUEL_VAL_DOWN = 0.0f;
static const float FUEL_VAL_UP = 100.0f;
static const float FUEL_ALPHA = 0.01f; // Utjämning, 1 = ingen utjämning, 0 = max utjämning
// Kalibrering Vattenpressure
static const int PIN_WATER_PRESSURE = 10;
static const float WATER_PRESSURE_FS = 30.0f;    // Full skala, 30 psi
static const float WATER_PRESSURE_ZERO_V = 0.5f; // givarspänning vid 0 tryck, justeras efter mätning - Kalibrera med motor avstängd.
static const float WATER_PRESSURE_ALPHA = 0.3f;  // Utjämning, 1 = ingen utjämning, 0 = max utjämning
// // Kalibrering Vattentemp
// static const int PIN_WATER_TEMP = 11; // SKA ÄNDRAS
// static const float WATER_TEMP_FS = 100.0f; // Full skala, 100 grader Celsius
// static const float WATER_TEMP_ZERO_V = 0.5f; // givarspänning vid 0 grad, justeras efter mätning
// static const float WATER_TEMP_ALPHA = 0.3f; // Utjämning, 1 = ingen utjämning, 0 = max utjämning

struct Sender
{
    int pin;
    float ratioA, ratioB; // kalibreringspunkter, 0-1
    float valA, valB;     // Vad som visas på dashboard vid de två kalibreringspunkterna
    float alpha;          // utjämning, 1 = ingen utjämning, 0 = max utjämning
    float ratio;
    float value; // Utvärde
};

static Sender trim = {PIN_TRIM, TRIM_RATIO_DOWN, TRIM_RATIO_UP, TRIM_VAL_DOWN, TRIM_VAL_UP, TRIM_ALPHA, NAN, NAN};
static Sender tank = {PIN_FUEL, FUEL_RATIO_DOWN, FUEL_RATIO_UP, FUEL_VAL_DOWN, FUEL_VAL_UP, FUEL_ALPHA, NAN, NAN};

float g_battV = NAN;
float g_trim = NAN, g_trimRatio = NAN;
float g_tank = NAN, g_tankRatio = NAN;
float g_waterPressureV = NAN, g_waterPressureBar = NAN;
float g_waterTempV = NAN, g_waterTempC = NAN;

static float readVolts(int pin, float div = DIV)
{
    uint32_t mv = 0;
    for (int i = 0; i < N_SAMPLES; i++)
        mv += analogReadMilliVolts(pin);
    return mv / (float)N_SAMPLES / 1000.0f * div;
}

static float ema(float prev, float v, float alpha)
{
    return isnan(prev) ? v : prev + alpha * (v - prev);
}

static void initPin(int pin)
{
    analogRead(pin); // registrera pinnen som analog
    analogSetPinAttenuation(pin, ADC_11db);
}

static void updateSender(Sender &s, float vb, float div = DIV)
{
    if (vb < 6.0f)
    {
        s.ratio = NAN;
        s.value = NAN;
        return;
    } // för låg spänning, avbryt

    s.ratio = ema(s.ratio, readVolts(s.pin, div) / vb, s.alpha);
    float f = (s.ratio - s.ratioA) / (s.ratioB - s.ratioA);
    s.value = s.valA + constrain(f, 0.0f, 1.0f) * (s.valB - s.valA);
}

void analogInit()
{
    analogReadResolution(12);
    initPin(PIN_BATT);
    initPin(trim.pin);
    initPin(tank.pin);
    initPin(PIN_WATER_PRESSURE);
    // initPin(PIN_WATER_TEMP);
}

void analogUpdate()
{
    static unsigned long t = 0;
    if (millis() - t < 50)
        return;
    t = millis();

    float vb = readVolts(PIN_BATT);
    g_battV = ema(g_battV, vb, ALPHA_BATT);
    updateSender(trim, vb, DIV);
    updateSender(tank, vb, DIV);

    g_trim = trim.value;
    g_trimRatio = trim.ratio;
    g_tank = tank.value;
    g_tankRatio = tank.ratio;
    // Water pressure
    g_waterPressureV = ema(g_waterPressureV, readVolts(PIN_WATER_PRESSURE, DIV_WATER_PRESSURE), WATER_PRESSURE_ALPHA);
    if (g_waterPressureV < 0.3f || g_waterPressureV > 4.7f)
    {
        g_waterPressureBar = NAN; // för låg eller för hög spänning, avbryt
    }
    else
    {
        float psi = (g_waterPressureV - WATER_PRESSURE_ZERO_V) / 4.0f * WATER_PRESSURE_FS; // 0.5-4.5V -> psi
        g_waterPressureBar = max(0.0f, psi * 0.0689476f);                                  // psi -> bar
    }
    // g_waterTempV = ema(g_waterTempV, readVolts(PIN_WATER_TEMP, DIV_WATER_TEMP), WATER_TEMP_ALPHA);
}

void checkAnalogValues()
{
    static unsigned long tDbg = 0; // kalibreringsutskrift, 1 gång/s
    if (millis() - tDbg > 1000)
    {
        tDbg = millis();
        Serial.printf("Analog: batt %.2f  trim %.3f→%.0f  tank %.3f→%.0f  vatten %.2fV→%.2f bar\n",
                      g_battV, g_trimRatio, g_trim, g_tankRatio, g_tank, g_waterPressureV, g_waterPressureBar);
    }
}
