#include "aux_config.h"
#include <cstring>
#include "esphome/core/log.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        AuxConfig::EnumConfigOption::EnumConfigOption(const char *key,
                                                      const char *label,
                                                      std::map<uint8_t, const char *> &&options)
            : key(key),
              label(label),
              options(std::move(options))
        {
        }

        HTMLElement AuxConfig::EnumConfigOption::as_html() const
        {
            std::list<SelectElement::Option> html_options;
            for (const auto &option : this->options)
                html_options.emplace_back(option.second, option.first == this->load_from_storage());

            return create_form_field(SelectElement{std::move(html_options)}, this->label);
        }

        void AuxConfig::EnumConfigOption::store(const char *val)
        {
            if (auto cfg = Supla::Storage::ConfigInstance())
                for (const auto &option : this->options)
                    if (std::strcmp(option.second, val) == 0)
                    {
                        cfg->setUInt8(key, option.first);
                        ESP_LOGD(TAG, "Stored %s->%d", key, option.first);
                    }
        }

        uint8_t AuxConfig::EnumConfigOption::load_from_storage() const
        {
            if (auto cfg = Supla::Storage::ConfigInstance())
            {
                uint8_t storage_value = 0;
                if (cfg->getUInt8(key, &storage_value))
                    return storage_value;
            }
            return 0;
        }

        AuxConfig::Frontend::Frontend()
            : Supla::HtmlElement(Supla::HTML_SECTION_FORM) {}

        void AuxConfig::Frontend::send(Supla::WebSender *sender)
        {
            for (const auto &opt : AuxConfig::options)
                opt.as_html().render(sender);
        }

        bool AuxConfig::Frontend::handleResponse(const char *key, const char *value)
        {
            for (auto &opt : AuxConfig::options)
                if (std::strcmp(opt.key, key) == 0)
                {
                    opt.store(value);
                    return true;
                }
            return false;
        }

        std::array<AuxConfig::EnumConfigOption, 2> AuxConfig::options = {
            EnumConfigOption{"led_mode", "LED Mode", {{LED_OFF, "OFF"}, {LED_ALWAYS, "ON"}, {LED_MATCH, "ON METER MATCH"}}},
            EnumConfigOption{"config_mode", "Config Mode", {{CONFIG_MODE_DEFAULT, "DEFAULT"}, {CONFIG_MODE_ALWAYS_ON, "ALWAYS ON"}}},
        };

        AuxConfig::Data AuxConfig::pull()
        {
            return {AuxConfig::options[0].load_from_storage(),
                    AuxConfig::options[1].load_from_storage()};
        }

        AuxConfig::LEDMode AuxConfig::Data::get_led_mode() const
        {
            return static_cast<LEDMode>(this->values[0]);
        }

        AuxConfig::ConfigMode AuxConfig::Data::get_config_mode() const
        {
            return static_cast<ConfigMode>(this->values[1]);
        }

    } // namespace supla_wmbus_reader
} // namespace esphome
