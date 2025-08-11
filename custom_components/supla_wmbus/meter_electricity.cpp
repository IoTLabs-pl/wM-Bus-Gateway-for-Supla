#include "meter_electricity.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        const std::vector<CallbackMetadata> ElectricityMeter::callback_metadata_ = {
            NON_INDEXABLE_CB(ElectricityMeter, "Forward Balanced Energy", setFwdBalancedEnergy, 1e5f),
            NON_INDEXABLE_CB(ElectricityMeter, "Reverse Balanced Energy", setRvrBalancedEnergy, 1e5f),
            INDEXABLE_CB(ElectricityMeter, "Forward Active Energy", setFwdActEnergy, 1e5f),
            INDEXABLE_CB(ElectricityMeter, "Reverse Active Energy", setRvrActEnergy, 1e5f),
            INDEXABLE_CB(ElectricityMeter, "Forward Reactive Energy", setFwdReactEnergy, 1e5f),
            INDEXABLE_CB(ElectricityMeter, "Reverse Reactive Energy", setRvrReactEnergy, 1e5f),
            INDEXABLE_CB(ElectricityMeter, "Voltage", setVoltage, 1e2f),
            INDEXABLE_CB(ElectricityMeter, "Current", setCurrent, 1e3f),
            NON_INDEXABLE_CB(ElectricityMeter, "Frequency", setFreq, 1e2f),
            INDEXABLE_CB(ElectricityMeter, "Active Power", setPowerActive, 1e8f),
            INDEXABLE_CB(ElectricityMeter, "Reactive Power", setPowerReactive, 1e8f),
            INDEXABLE_CB(ElectricityMeter, "Apparent Power", setPowerApparent, 1e8f),
            INDEXABLE_CB(ElectricityMeter, "Power Factor", setPowerFactor, 1e3f),
            INDEXABLE_CB(ElectricityMeter, "Phase Angle", setPhaseAngle, 1e1f),
        };

        ElectricityMeter::ElectricityMeter(bool multiphase) : MeterBase(this->callback_metadata_)
        {
            if (!multiphase)
                this->extChannel.setFlag(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                                         SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
        }

        ElectricityMeter *ElectricityMeter::create(ConfigEntry *ce)
        {
            if (ce->meter_type() != MeterType::ElectricityMeter)
                return nullptr;

            auto multiphase = false;

            for (const auto &entry : *ce)
                if (entry.find("%d") != std::string::npos)
                {
                    multiphase = true;
                    break;
                }

            return new ElectricityMeter{multiphase};
        }
    }
}
