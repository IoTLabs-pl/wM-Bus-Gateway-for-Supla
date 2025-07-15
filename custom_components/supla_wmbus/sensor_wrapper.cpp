#include "sensor_wrapper.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

#define INDEXABLE_BIND(T, name, method, multiplier) \
    {name, true, [](std::shared_ptr<T> obj, uint8_t idx, float val) { obj->method(idx, val * multiplier + 0.5f); }}
#define NON_INDEXABLE_BIND(T, name, method, multiplier) \
    {name, false, [](std::shared_ptr<T> obj, uint8_t, float val) { obj->method(val * multiplier + 0.5f); }}

        template <>
        const std::vector<MetaBind<EM>> ElectricityMeter::meta_binds = {
            NON_INDEXABLE_BIND(EM, "Forward Balanced Energy", setFwdBalancedEnergy, 1e5f),
            NON_INDEXABLE_BIND(EM, "Reverse Balanced Energy", setRvrBalancedEnergy, 1e5f),
            INDEXABLE_BIND(EM, "Forward Active Energy", setFwdActEnergy, 1e5f),
            INDEXABLE_BIND(EM, "Reverse Active Energy", setRvrActEnergy, 1e5f),
            INDEXABLE_BIND(EM, "Forward Reactive Energy", setFwdReactEnergy, 1e5f),
            INDEXABLE_BIND(EM, "Reverse Reactive Energy", setRvrReactEnergy, 1e5f),
            INDEXABLE_BIND(EM, "Voltage", setVoltage, 1e2f),
            INDEXABLE_BIND(EM, "Current", setCurrent, 1e3f),
            NON_INDEXABLE_BIND(EM, "Frequency", setFreq, 1e2f),
            INDEXABLE_BIND(EM, "Active Power", setPowerActive, 1e8f),
            INDEXABLE_BIND(EM, "Reactive Power", setPowerReactive, 1e8f),
            INDEXABLE_BIND(EM, "Apparent Power", setPowerApparent, 1e8f),
            INDEXABLE_BIND(EM, "Power Factor", setPowerFactor, 1e3f),
            INDEXABLE_BIND(EM, "Phase Angle", setPhaseAngle, 1e1f),
        };

        template <>
        const std::vector<MetaBind<VIC>> FlowMeter::meta_binds = {
            NON_INDEXABLE_BIND(VIC, "Impulse Counter", setCounter, 1e3f),
        };
    }
}