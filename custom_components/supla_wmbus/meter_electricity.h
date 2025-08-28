#pragma once
#include "meter_base.h"
#include "supla/sensor/electricity_meter.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        class ElectricityMeter : public MeterBase,
                                 public Supla::Sensor::ElectricityMeter
        {
        public:
            ElectricityMeter(const ConfigEntry *ce);
            static bool can_build_from(const ConfigEntry *ce);
            static const std::vector<CallbackMetadata> callback_metadata;
        };
    }
}
