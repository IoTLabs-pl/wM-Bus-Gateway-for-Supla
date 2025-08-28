#include "sensor.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        static const char *TAG = "supla.wmbus.sensor";

        Sensor::Sensor(const std::string &sensor_name,
                       const std::string &field_name,
                       const std::string &unit_of_measurement,
                       std::function<void(float)> &&callback) : name_(sensor_name),
                                                                unit_of_measurement_(unit_of_measurement)
        {
            ESP_LOGD(TAG, "Constructing sensor: %s (%s) with unit %s",
                     sensor_name.c_str(),
                     field_name.c_str(),
                     unit_of_measurement.c_str());

            this->set_name(this->name_.c_str());
            this->set_unit_of_measurement(this->unit_of_measurement_.c_str());
            this->set_field_name(field_name);
            this->add_on_state_callback(std::move(callback));
        }
    } // namespace supla_wmbus_gateway
} // namespace esphome
