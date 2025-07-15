#pragma once

#include <list>
#include <string>
#include <vector>

#include "supla/network/html_element.h"

#include "sensor_wrapper.h"
#include "html_elements.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class Meter;

        class ConfigEntry : public std::vector<std::string>
        {
        public:
            ConfigEntry(const std::string &data = "");
            HTMLElement as_html() const;
            std::string serialized() const;
            Meter *create_meter() const;

        protected:
            static std::vector<std::string> split_string(std::string serialized, size_t minimum_size = 0);
            const std::vector<const BindMetadata *> bind_metadata;
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
                size_t post_data_counter_ = 0;
            };

        public:
            static std::list<ConfigEntry> pull();

        protected:
            Frontend frontend_;
        };

    } // namespace supla_wmbus_reader
} // namespace esphome