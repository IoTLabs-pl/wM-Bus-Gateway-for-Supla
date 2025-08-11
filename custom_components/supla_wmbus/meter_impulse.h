#pragma once
#include "meter_base.h"
#include "supla/sensor/virtual_impulse_counter.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class ImpulseCounter : public MeterBase,
                               public Supla::Sensor::VirtualImpulseCounter
        {
        public:
            ImpulseCounter(uint16_t fcn);
            static ImpulseCounter *create(ConfigEntry *ce);
            Supla::ApplyConfigResult applyChannelConfig(TSD_ChannelConfig *result, bool) override;
            void fillChannelConfig(void *channelConfig, int *size, uint8_t) override;

        protected:
            static const std::vector<CallbackMetadata> callback_metadata_;
        };
    }
}
