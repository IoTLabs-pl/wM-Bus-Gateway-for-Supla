#include "meter.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        static const char *TAG = "supla.wmbus.meter";

        Meter::Meter(const ConfigEntry *e,
                     std::unique_ptr<WrappedSuplaBase> supla_object)
            : config_(e),
              supla_object_(std::move(supla_object))
        {
            this->set_meter_params((*e)[0], (*e)[1], (*e)[2]);
        }

        std::list<Sensor> &Meter::create_sensors()
        {
            auto bindings = this->supla_object_->prepare_bindings();
            auto &config = this->config_;

            ESP_LOGD(TAG, "Building sensors for %s", this->get_id().c_str());
            ESP_LOGD(TAG, "Found %d bindings", bindings.size());

            for (size_t i = 0; i < bindings.size(); ++i)
            {
                auto &binding = bindings[i];
                auto &field = (*config)[i + 3];

                ESP_LOGV(TAG, "Binding: %s (%s)", binding.name, field.c_str());

                if (field.empty())
                    continue;

                const char placeholder[] = "%d";
                auto placeholder_position = field.find(placeholder);
                bool indexable = (placeholder_position != std::string::npos && binding.indexable);
                auto cnt = indexable ? 3 : 1;

                std::string unit_str;
                Unit unit;
                extractUnit(field, &unit_str, &unit);
                unit_str = unitToStringHR(unit);

                for (uint8_t j = 1; j <= cnt; ++j)
                {
                    std::string field_name = field;
                    std::string sensor_name = this->get_id() + ' ' + binding.name;
                    std::string str_idx = std::to_string(j);

                    if (placeholder_position != std::string::npos)
                        field_name = field_name.replace(placeholder_position, sizeof(placeholder) - 1, str_idx);

                    if (indexable)
                        sensor_name += ' ' + str_idx;

                    this->sensors_.emplace_back(
                        sensor_name,
                        field_name,
                        unit_str,
                        std::bind(binding.setter, j-1, std::placeholders::_1));
                }
            }

            for (auto &sensor : this->sensors_)
                sensor.set_parent(this);

            return this->sensors_;
        }

        const std::vector<const BindMetadata *> Meter::bind_metadata() const
        {
            return this->supla_object_->bind_metadata();
        }

    } // namespace supla_wmbus_reader
} // namespace esphome
