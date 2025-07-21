#pragma once

#include <map>
#include <cstring>

#include "esphome/core/log.h"

#include "supla/network/html_element.h"
#include "supla/storage/config.h"

#include "html_elements.h"

namespace esphome
{
    namespace supla_device
    {
        static const char *TAG = "supla_device.aux_config";

        template <typename T>
        class EnumOption : public Supla::HtmlElement
        {

        public:
            EnumOption(const char *key,
                       const char *label,
                       std::map<T, const char *> &&options) : key(key),
                                                              label(label),
                                                              options(std::move(options))
            {
            }

            T load_from_storage() const
            {
                if (auto cfg = Supla::Storage::ConfigInstance())
                {
                    uint8_t storage_value = 0;
                    if (cfg->getUInt8(key, &storage_value))
                        return (T)storage_value;
                }
                return (T)0;
            }

            void send(Supla::WebSender *sender) override
            {
                this->as_html().render(sender);
            }

            bool handleResponse(const char *key, const char *value) override
            {
                if (std::strcmp(this->key, key) == 0)
                {
                    this->store(value);
                    return true;
                }
                return false;
            }

        protected:
            const char *key;
            const char *label;
            std::map<T, const char *> options;

            HTMLElement as_html() const
            {
                std::list<SelectElement::Option> html_options;
                for (const auto &option : this->options)
                    html_options.emplace_back(option.second, option.first == this->load_from_storage());

                return create_form_field(SelectElement{std::move(html_options)}, this->label);
            }

            void store(const char *val)
            {
                if (auto cfg = Supla::Storage::ConfigInstance())
                    for (const auto &option : this->options)
                        if (std::strcmp(option.second, val) == 0)
                        {
                            cfg->setUInt8(key, option.first);
                            ESP_LOGD(TAG, "Stored %s->%d", key, option.first);
                        }
            }
        };

    }
}