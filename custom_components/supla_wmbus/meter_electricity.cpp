#include "meter_electricity.h"

#include <algorithm>

#include "config.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        const std::vector<CallbackMetadata> ElectricityMeter::callback_metadata = {
            INDEXABLE_CB(ElectricityMeter, "Forward Active Energy", setFwdActEnergy, 1e5f,
                         "total_energy_consumption_kwh", "positive_total_active_energy_kwh",
                         "total_active_positive_3phase_kwh", "total_kwh"),
            INDEXABLE_CB(ElectricityMeter, "Reverse Active Energy", setRvrActEnergy, 1e5f,
                         "total_energy_production_kwh", "negative_total_active_energy_kwh"),
            INDEXABLE_CB(ElectricityMeter, "Forward Reactive Energy", setFwdReactEnergy, 1e5f,
                         "total_reactive_energy_consumption_kvarh", "positive_total_reactive_energy_kvarh"),
            INDEXABLE_CB(ElectricityMeter, "Reverse Reactive Energy", setRvrReactEnergy, 1e5f,
                         "total_reactive_energy_production_kvarh", "negative_total_reactive_energy_kvarh"),
            INDEXABLE_CB(ElectricityMeter, "Voltage", setVoltage, 1e2f,
                         "voltage_at_phase_%d_v", "voltage_l%d_n_v", "voltage_%d_n_v", "voltage_v"),
            INDEXABLE_CB(ElectricityMeter, "Current", setCurrent, 1e3f,
                         "current_at_phase_%d_a", "current_l%d_a", "amperage_at_phase_%d_a", "amperage_a"),
            NON_INDEXABLE_CB(ElectricityMeter, "Frequency", setFreq, 1e2f,
                             "frequency_hz"),
            INDEXABLE_CB(ElectricityMeter, "Active Power", setPowerActive, 1e8f,
                         "current_power_consumption_phase%d_kw", "current_power_consumption_%d_kw",
                         "active_consumption_l%d_kw", "active_power_l%d_kw", "active_power_at_phase_%d_kw",
                         "power_l%d_kw", "current_power_consumption_kw", "power_consumption_kw",
                         "active_consumption_kw", "three_phase_total_active_power_kw",
                         "active_positive_3phase_kw", "active_power_overall_kw", "power_kw"),
            INDEXABLE_CB(ElectricityMeter, "Reactive Power", setPowerReactive, 1e8f,
                         "reactive_consumption_l%d_kw", "reactive_power_l%d_kvar", "reactive_power_at_phase_%d_kw",
                         "reactive_power_l%d_kw", "reactive_consumption_kw", "three_phase_total_reactive_power_kvar",
                         "reactive_positive_3phase_kw", "reactive_power_overall_kw"),
            INDEXABLE_CB(ElectricityMeter, "Apparent Power", setPowerApparent, 1e8f,
                         "apparent_consumption_l%d_kw", "apparent_power_l%d_kva", "apparent_consumption_kw",
                         "three_phase_total_apparent_power_kva"),
            INDEXABLE_CB(ElectricityMeter, "Power Factor", setPowerFactor, 1e3f,
                         "power_l%d_factor", "at_phase_%d_power_factor", "power_factor", "three_phase_power_factor"),
            INDEXABLE_CB(ElectricityMeter, "Phase Angle", setPhaseAngle, 1e1f,
                         "phase_angle_power_l%d_deg", "power_phase_angle_deg"),
        };

        ElectricityMeter::ElectricityMeter(const ConfigEntry *ce)
            : MeterBase(this->callback_metadata)
        {
            bool multiphase = std::ranges::any_of(callback_metadata, [&](const auto &callback)
                                                  {
                const char *field = pick_field(ce->spec(), callback);
                return field && std::string_view(field).find(PHASE_PLACEHOLDER) != std::string_view::npos; });

            if (!multiphase)
                this->extChannel.setFlag(SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED |
                                         SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED);
        }
    }
}
