#include "config.h"

#include "supla/sensor/electricity_meter.h"
#include "supla/sensor/virtual_impulse_counter.h"
#include "supla/storage/config.h"

#include "esphome/core/log.h"
#include "esphome/components/wmbus_common/component.h"
#include "esphome/components/wmbus_common/meters.h"

#include "resources.h"
#include "meter.h"

#define GENERATE_KEY(varname, index)         \
    char varname[SUPLA_CONFIG_MAX_KEY_SIZE]; \
    Supla::Config::generateKey(varname, index, "meter");

namespace esphome
{
    namespace supla_wmbus_reader
    {
        static const char *TAG = "supla.wmbus.config";

        ConfigEntry::ConfigEntry(const std::string &data)
            : std::vector<std::string>{split_string(data, 3)},
              bind_metadata{[this]()
                            {
                                auto m = std::unique_ptr<Meter>{this->create_meter()};
                                return m ? m->bind_metadata() : std::vector<const BindMetadata *>{};
                            }()}
        {
            ESP_LOGD(TAG, "ConfigEntry created with %zu fields", this->size());
            size_t actual_size = this->size();
            size_t required_size = 3 + this->bind_metadata.size();

            if (actual_size != required_size)
            {
                ESP_LOGD(TAG, "ConfigEntry size mismatch: expected %zu, got %zu", required_size, actual_size);
                this->resize(3);
                this->resize(required_size);
            }
        }

        HTMLElement ConfigEntry::as_html() const
        {

            std::list<SelectElement::Option> driver_options;
            for (const auto &driver_name : wmbus_common::driver_names)
                driver_options.emplace_back(driver_name.c_str(), driver_name == (*this)[1]);

            std::list<HTMLElement> fields = {
                InputElement{"", "hidden"},
                {"h3", ""},
                create_form_field(InputElement{(*this)[0].c_str(), "number"}, "ID"),
                create_form_field(SelectElement{std::move(driver_options)}, "Driver"),
                create_form_field(InputElement{(*this)[2].c_str()}, "Key"),
                create_form_field(HTMLElement{"button", "Remove", {}, {{"type", "button"}}})};

            auto &bindings = this->bind_metadata;
            for (uint8_t i = 0; i < bindings.size(); ++i)
                fields.insert(
                    std::prev(fields.end()),
                    create_form_field(InputElement{(*this)[i + 3].c_str()},
                                      bindings[i]->name,
                                      bindings[i]->indexable));

            return DivElement{
                std::move(fields),
                {
                    {"class", "box collapsible collapsed meter"},
                },
            };
        };

        std::vector<std::string> ConfigEntry::split_string(std::string serialized, size_t minimum_size)
        {
            std::list<std::string> result;
            size_t pos = 0;
            while ((pos = serialized.find(',')) != std::string::npos)
            {
                result.push_back(serialized.substr(0, pos));
                serialized.erase(0, pos + 1);
            }
            result.push_back(serialized); // Add the last part after the last comma

            if (result.size() < minimum_size)
                result.resize(minimum_size);

            return {result.cbegin(), result.cend()};
        }

        Meter *ConfigEntry::create_meter() const
        {
            auto di = lookupDriver((*this)[1]);
            auto type = (!(*this)[0].empty() && di) ? di->type() : MeterType::UnknownMeter;

            switch (type)
            {
            case MeterType::WaterMeter:
            case MeterType::GasMeter:
                ESP_LOGD(TAG, "Creating FlowMeter");
                return new Meter(this, std::unique_ptr<FlowMeter>{new FlowMeter{}});
            case MeterType::ElectricityMeter:
                ESP_LOGD(TAG, "Creating ElectricityMeter");
                return new Meter(this, std::unique_ptr<ElectricityMeter>{new ElectricityMeter{}});
            default:
                return nullptr;
            }
        }

        std::string ConfigEntry::serialized() const
        {
            std::string result;
            for (const auto &field : *this)
            {
                if (!result.empty())
                    result += ',';
                result += field;
            }
            return result;
        }

        std::list<ConfigEntry> Config::pull()
        {
            auto cfg = Supla::Storage::ConfigInstance();
            if (!cfg)
                return {};

            std::list<ConfigEntry> result;
            char value[300];
            uint8_t i = 0;

            while (true)
            {
                GENERATE_KEY(key, i);
                if (!cfg->getString(key, value, sizeof(value) - 1))
                    break;
                ESP_LOGD(TAG, "Loaded: %s", value);
                result.emplace_back(value);
                ++i;
            }
            return result;
        }

        void Config::Frontend::send(Supla::WebSender *sender)
        {
            auto div = DivElement({
                                      {"script", frontend_script},
                                      {"h3", "wM-Bus Meters"},
                                      {"button", "New Meter", {}, {{"type", "button"}, {"id", "add_meter"}}},
                                  },
                                  {
                                      {"class", "box"},
                                  });

            auto entries = Config::pull();
            if (entries.empty())
                entries.emplace_back();

            for (const auto &entry : entries)
                div.children.insert(std::prev(div.children.end()), entry.as_html());

            div.render(sender);
        }

        bool Config::Frontend::handleResponse(const char *key, const char *value)
        {
            if (strncmp(key, "meter", sizeof("meter") - 1) == 0 &&
                value &&
                value[0] != ',')
                if (auto cfg = Supla::Storage::ConfigInstance())
                {
                    GENERATE_KEY(storage_key, this->post_data_counter_);
                    cfg->setString(storage_key, value);

                    ESP_LOGD(TAG, "Stored %s->%s", storage_key, value);

                    this->post_data_counter_ += 1;

                    return true;
                }

            return false;
        }

        void Config::Frontend::onProcessingEnd()
        {
            if (auto cfg = Supla::Storage::ConfigInstance())
                while (true)
                {
                    GENERATE_KEY(storage_key, this->post_data_counter_);
                    if (!cfg->eraseKey(storage_key))
                        break;
                    this->post_data_counter_ += 1;
                }

            this->post_data_counter_ = 0;
        }

    } // namespace supla_wmbus_reader
} // namespace esphome
