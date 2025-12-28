#include "DistanceSensor.h"

void DistanceSensor::setLight(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t l4)
{
    std::vector<uint8_t> data;
    if (l1 > 100)
        l1 = 100;
    if (l2 > 100)
        l2 = 100;
    if (l3 > 100)
        l3 = 100;
    if (l4 > 100)
        l4 = 100;

    data.push_back(l1);
    data.push_back(l2);
    data.push_back(l3);
    data.push_back(l4);

    port_.writeData(LIGHT_MODE, data);
}

float DistanceSensor::getDistance()
{
    return port_.getValue(0, 0);
}
