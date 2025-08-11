#pragma once
#include "meter_base.h"
#include "supla/sensor/electricity_meter.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class ElectricityMeter : public MeterBase,
                                 public Supla::Sensor::ElectricityMeter
        {
        public:
            ElectricityMeter(bool multiphase);
            static ElectricityMeter *create(ConfigEntry *ce);

        protected:
            static const std::vector<CallbackMetadata> callback_metadata_;
        };
    }
}
