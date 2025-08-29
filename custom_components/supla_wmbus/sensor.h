#pragma once

#include <functional>
#include <string>
#include <memory>

#include "esphome/components/wmbus_meter/sensor.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        class Sensor : public wmbus_meter::Sensor
        {
        public:
            Sensor(const std::string &sensor_name,
                   const std::string &field_name,
                   const std::string &unit_of_measurement,
                   std::function<void(float)>&& callback);

        protected:
            std::string name_;
            std::string unit_of_measurement_;
        };
    }
}