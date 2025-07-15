#pragma once

#include <array>
#include <map>
#include <vector>

#include "supla/network/html_element.h"
#include "supla/storage/config.h"

#include "html_elements.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        static const char *TAG = "supla.wmbus.aux_config";

        class AuxConfig
        {
        public:
            enum LEDMode
            {
                LED_ALWAYS,
                LED_MATCH,
                LED_OFF,
            };

            enum ConfigMode
            {
                CONFIG_MODE_DEFAULT,
                CONFIG_MODE_ALWAYS_ON
            };

        protected:
            struct EnumConfigOption
            {
                const char *key;
                const char *label;
                std::map<uint8_t, const char *> options;

                EnumConfigOption(const char *key,
                                 const char *label,
                                 std::map<uint8_t, const char *> &&options);

                HTMLElement as_html() const;
                void store(const char *val);
                uint8_t load_from_storage() const;
            };

            class Frontend : public Supla::HtmlElement
            {
            public:
                Frontend();

                void send(Supla::WebSender *sender) override;
                bool handleResponse(const char *key, const char *value) override;
            };

            struct Data
            {
                std::array<uint8_t, 2> values;

                LEDMode get_led_mode() const;
                ConfigMode get_config_mode() const;
            };

        public:
            static Data pull();

        protected:
            static std::array<EnumConfigOption, 2> options;
            Frontend frontend_;
        };
    }
}