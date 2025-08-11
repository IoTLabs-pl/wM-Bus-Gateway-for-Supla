#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "esphome/components/supla_device/html_elements.h"
#include "esphome/components/wmbus_common/meters.h"
#include "esphome/components/wmbus_gateway/display_manager.h"

#include "supla/network/html_element.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class MeterBase;

        class ConfigEntry : public std::vector<std::string>
        {
            using HTMLElement = supla_device::HTMLElement;
            using SelectElement = supla_device::SelectElement;
            using InputElement = supla_device::InputElement;
            using DivElement = supla_device::DivElement;
            static std::vector<std::string> split_string(std::string serialized, size_t minimum_size = 0);

            std::unique_ptr<MeterBase> meter_;

        public:
            ConfigEntry(const std::string &data = "");
            HTMLElement as_html() const;
            std::string serialized() const;
            MeterBase *build_meter(wmbus_radio::Radio *radio, wmbus_gateway::DisplayManager *display_manager);
            MeterType meter_type() const;
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