#pragma once
#include "esphome/core/component.h"

#include "esphome/components/wmbus_common/meters.h"
#include "esphome/components/wmbus_radio/component.h"
#include "esphome/components/wmbus_reader/wmbus_reader.h"

#include <SuplaDevice.h>

#include <supla/sensor/general_purpose_meter.h>
#include <supla/sensor/virtual_impulse_counter.h>
#include <supla/network/html_element.h>

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class Component;
        class Meter;

        class Frontend : public Supla::HtmlElement
        {
        public:
            Frontend(Component *parent) : Supla::HtmlElement(Supla::HTML_SECTION_PROTOCOL),
                                          parent_{parent} {};
            void send(Supla::WebSender *sender) override;
            bool handleResponse(const char *key, const char *value) override;
            void onProcessingEnd() override;

        protected:
            Component *parent_;
            std::vector<Meter *> post_meters;
        };

        class Component : public esphome::Component
        {
            friend class Frontend;
            friend class Meter;

        public:
            Component() : frontend{this} {};
            void setup() override;
            void loop() override;
            void set_radio(wmbus_radio::Radio *radio) { this->radio = radio; };
            void set_display_manager(wmbus_reader::DisplayScreenManager *manager) { this->display_manager = manager; };

            float get_setup_priority() const { return setup_priority::AFTER_CONNECTION - 1; }

        protected:
            std::vector<Meter *> get_stored_meters();

            std::vector<Meter *> meters;
            wmbus_radio::Radio *radio;
            wmbus_reader::DisplayScreenManager *display_manager;
            Frontend frontend;
        };

    }
}