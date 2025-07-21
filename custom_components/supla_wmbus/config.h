#pragma once

#include <list>
#include <string>
#include <vector>

#include "esphome/components/supla_device/html_elements.h"

#include "supla/network/html_element.h"

#include "sensor_wrapper.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        class Meter;

        class ConfigEntry : public std::vector<std::string>
        {
            using HTMLElement = supla_device::HTMLElement;
            using SelectElement = supla_device::SelectElement;
            using InputElement = supla_device::InputElement;
            using DivElement = supla_device::DivElement;
            static std::vector<std::string> split_string(std::string serialized, size_t minimum_size = 0);
            const std::vector<const BindMetadata *> bind_metadata;

        public:
            ConfigEntry(const std::string &data = "");
            HTMLElement as_html() const;
            std::string serialized() const;
            Meter *create_meter() const;
        };

        class Config
        {
            class Frontend : public Supla::HtmlElement
            {
            public:
                Frontend() : Supla::HtmlElement(Supla::HTML_SECTION_PROTOCOL) {};
                void send(Supla::WebSender *sender) override;
                bool handleResponse(const char *key, const char *value) override;
                void onProcessingEnd() override;

            protected:
                using DivElement = supla_device::DivElement;

                size_t post_data_counter_ = 0;
            };

        public:
            static std::list<ConfigEntry> pull();

        protected:
            Frontend frontend_;
        };

    } // namespace supla_wmbus_reader
} // namespace esphome