#include "meter_base.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        static const char *TAG = "supla.wmbus.meter_base";

        MeterBase::MeterBase(const std::vector<CallbackMetadata> &callback_metadata)
            : callback_metadata_(callback_metadata)
        {
        }

        std::list<Sensor> &MeterBase::create_sensors(const ConfigEntry *config)
        {
            // ...existing code from MeterBase::create_sensors...
            auto &callbacks = this->callback_metadata_;
            
            ESP_LOGD(TAG, "Building sensors for %s", this->get_id().c_str());
            ESP_LOGD(TAG, "Found %d callback protos", callbacks.size());

            for (size_t i = 0; i < callbacks.size(); ++i)
            {
                auto &callback = callbacks[i];
                auto &field = (*config)[i + 3];

                ESP_LOGV(TAG, "Callback: %s (%s)", callback.name, field.c_str());

                if (field.empty())
                    continue;

                const char placeholder[] = "%d";
                auto placeholder_position = field.find(placeholder);
                bool indexable = (placeholder_position != std::string::npos && callback.indexable);
                auto cnt = indexable ? 3 : 1;

                std::string unit_str;
                Unit unit;
                extractUnit(field, &unit_str, &unit);
                unit_str = unitToStringHR(unit);

                for (uint8_t j = 1; j <= cnt; ++j)
                {
                    std::string field_name = field;
                    std::string sensor_name = this->get_id() + ' ' + callback.name;
                    std::string str_idx = std::to_string(j);

                    if (placeholder_position != std::string::npos)
                        field_name = field_name.replace(placeholder_position, sizeof(placeholder) - 1, str_idx);

                    if (indexable)
                        sensor_name += ' ' + str_idx;

                    sensors_.emplace_back(
                        sensor_name,
                        field_name,
                        unit_str,
                        [this,
                         idx = j - 1,
                         &setter = callback.setter,
                         &multiplier = callback.multiplier](float x)
                        { setter(this, idx, x * multiplier); }
                    );
                }
            }

            for (auto &sensor : sensors_)
                sensor.set_parent(this);

            return sensors_;
        }
    }
}
