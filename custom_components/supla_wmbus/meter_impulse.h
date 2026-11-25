#pragma once
#include "meter_base.h"
#include "supla/sensor/virtual_impulse_counter.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        class ImpulseCounter : public MeterBase,
                               public Supla::Sensor::VirtualImpulseCounter
        {
        public:
            ImpulseCounter(const ConfigEntry *ce);
            static const std::vector<CallbackMetadata> volume_callbacks;
            static const std::vector<CallbackMetadata> energy_callbacks;

            Supla::ApplyConfigResult applyChannelConfig(TSD_ChannelConfig *result, bool) override;
            void fillChannelConfig(void *channelConfig, int *size, uint8_t) override;

        protected:
            static int get_supla_fcn(const ConfigEntry *ce);
        };
    }
}
