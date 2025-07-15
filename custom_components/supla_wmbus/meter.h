#pragma once

#include <list>
#include <vector>
#include <memory>

#include "esphome/components/wmbus_meter/sensor.h"

#include "config.h"
#include "sensor_wrapper.h"
#include "sensor.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        class Meter : public wmbus_meter::Meter
        {
        public:
            Meter(const ConfigEntry *e, std::unique_ptr<WrappedSuplaBase> supla_object);

            std::list<Sensor> &create_sensors();

            const std::vector<const BindMetadata *> bind_metadata() const;

        protected:
            const ConfigEntry *config_;
            std::list<Sensor> sensors_;
            std::unique_ptr<WrappedSuplaBase> supla_object_;
        };

    }
}
