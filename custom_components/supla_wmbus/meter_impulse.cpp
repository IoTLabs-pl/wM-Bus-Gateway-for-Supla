#include "meter_impulse.h"

#include "config.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        static const char *TAG = "supla.wmbus.meter_impulse";

        const std::vector<CallbackMetadata> ImpulseCounter::callback_metadata = {
            NON_INDEXABLE_CB(ImpulseCounter, "Impulse Counter", setCounter, 1e3f),
        };

        ImpulseCounter::ImpulseCounter(const ConfigEntry *ce)
            : MeterBase(this->callback_metadata)
        {
            auto supla_fcn = get_supla_fcn(ce);

            this->channel.setDefaultFunction(supla_fcn);
            this->channel.setFlag(SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE);
            this->usedConfigTypes.set(SUPLA_CONFIG_TYPE_DEFAULT);
        }

        int ImpulseCounter::get_supla_fcn(const ConfigEntry *ce)
        {
            uint16_t fcn = 0;
            switch (ce->meter_type())
            {
            case MeterType::GasMeter:
                fcn = SUPLA_CHANNELFNC_IC_GAS_METER;
                break;
            case MeterType::WaterMeter:
                fcn = SUPLA_CHANNELFNC_IC_WATER_METER;
                break;
            case MeterType::HeatMeter:
            case MeterType::HeatCoolingMeter:
                fcn = SUPLA_CHANNELFNC_IC_HEAT_METER;
                break;
            }

            return fcn;
        }

        Supla::ApplyConfigResult ImpulseCounter::applyChannelConfig(
            TSD_ChannelConfig *result, bool)
        {
            if (result->ConfigSize == 0)
                return Supla::ApplyConfigResult::SetChannelConfigNeeded;

            return Supla::ApplyConfigResult::NotSupported;
        }

        void ImpulseCounter::fillChannelConfig(void *channelConfig, int *size, uint8_t)
        {
            ESP_LOGE(TAG, "Filling channel config for ImpulseCounter");

            auto c = TChannelConfig_ImpulseCounter{.ImpulsesPerUnit = 1000};

            *size = sizeof(c);
            *(TChannelConfig_ImpulseCounter *)channelConfig = c;
        }

        bool ImpulseCounter::can_build_from(const ConfigEntry *ce)
        {
            return ce && get_supla_fcn(ce) != 0;
        }
    }
}
