#include "meter_base.h"

#include <algorithm>

#include "config.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        static const char *TAG = "supla.wmbus.meter_base";

        MeterBase::MeterBase(const std::vector<CallbackMetadata> &callback_metadata)
            : callback_metadata_(callback_metadata)
        {
        }

        const char *MeterBase::pick_field(const wmbus::DriverSpec *spec, const CallbackMetadata &callback)
        {
            for (const char *candidate : callback.field_candidates)
            {
                std::string first_phase = candidate;
                if (auto pos = first_phase.find(PHASE_PLACEHOLDER); pos != std::string::npos)
                    first_phase.replace(pos, PHASE_PLACEHOLDER.size(), "1");

                if (std::ranges::any_of(spec->fields, [&](const auto &f) { return first_phase == f.name; }))
                    return candidate;
            }
            return nullptr;
        }

        std::list<Sensor> &MeterBase::create_sensors()
        {
            ESP_LOGD(TAG, "Building sensors for %s", this->get_id().c_str());

            for (const auto &callback : this->callback_metadata_)
            {
                const char *field = pick_field(this->state_.spec, callback);
                ESP_LOGD(TAG, "%s: %s", callback.name, field ? field : "-");

                if (!field)
                    continue;

                const std::string field_template = field;
                auto placeholder_position = field_template.find(PHASE_PLACEHOLDER);
                bool per_phase = placeholder_position != std::string::npos;

                // Field names end with the unit symbol: total_m3
                uint8_t uom = 0;
                auto suffix = field_template.substr(field_template.rfind('_') + 1);
                for (const auto &unit : UNITS)
                    if (suffix == unit.symbol)
                        uom = unit.uom;

                for (uint8_t phase = 1; phase <= (per_phase ? 3 : 1); ++phase)
                {
                    std::string field_name = field_template;
                    std::string sensor_name = this->get_id() + ' ' + callback.name;

                    if (per_phase)
                    {
                        field_name.replace(placeholder_position, PHASE_PLACEHOLDER.size(), std::to_string(phase));
                        sensor_name += ' ' + std::to_string(phase);
                    }

                    sensors_.emplace_back(
                        sensor_name,
                        field_name,
                        uom,
                        [this, idx = phase - 1, &callback](float x)
                        { callback.setter(this, idx, x * callback.multiplier); });
                }
            }

            for (auto &sensor : sensors_)
                sensor.set_parent(this);

            return sensors_;
        }
    }
}
