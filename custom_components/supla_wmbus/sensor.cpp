#include "sensor.h"

namespace esphome
{
    __attribute__((weak)) const char *entity_uom_lookup(uint8_t) { return ""; }

    namespace supla_wmbus_gateway
    {
        static const char *TAG = "supla.wmbus.sensor";

        Sensor::Sensor(const std::string &sensor_name,
                       const std::string &field_name,
                       const std::string &unit_of_measurement,
                       std::function<void(float)> &&callback) : name_(sensor_name)

        {
            ESP_LOGD(TAG, "Constructing sensor: %s (%s) with unit %s",
                     sensor_name.c_str(),
                     field_name.c_str(),
                     unit_of_measurement.c_str());

            uint8_t uom_idx;
            for (uom_idx = 1; uom_idx <= UINT8_MAX; uom_idx++)
                if (unit_of_measurement == entity_uom_lookup(uom_idx))
                    break;

            this->configure_entity_(name_.c_str(), 0, uom_idx << ENTITY_FIELD_UOM_SHIFT);
            this->set_field_name(field_name);
            this->add_on_state_callback(std::move(callback));
        }
    } // namespace supla_wmbus_gateway
} // namespace esphome
