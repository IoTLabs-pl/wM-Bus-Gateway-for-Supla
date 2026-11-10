#include "sensor.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        static const char *TAG = "supla.wmbus.sensor";

        Sensor::Sensor(const std::string &sensor_name,
                       const std::string &field_name,
                       uint8_t uom,
                       std::function<void(float)> &&callback) : name_(sensor_name)

        {
            ESP_LOGD(TAG, "Constructing sensor: %s (%s) with unit %s",
                     sensor_name.c_str(),
                     field_name.c_str(),
                     entity_uom_lookup(uom));

            this->configure_entity_(name_.c_str(), 0, uom << ENTITY_FIELD_UOM_SHIFT);
            this->set_field_name(field_name);
            this->add_on_state_callback(std::move(callback));
        }
    } // namespace supla_wmbus_gateway
} // namespace esphome
