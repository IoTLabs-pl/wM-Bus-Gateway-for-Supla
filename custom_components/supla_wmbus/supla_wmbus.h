#pragma once
#include "esphome/core/component.h"

#include "esphome/components/wmbus_radio/component.h"
#include "esphome/components/wmbus_gateway_gui/display_manager.h"

#include "SuplaDevice.h"
#include "supla/network/html_element.h"

#include "binds.h"


namespace esphome
{
    namespace supla_wmbus_reader
    {
        class Component;

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
        };

        class Component : public esphome::Component
        {
            friend class Frontend;

        public:
            Component() : frontend{this} {};
            void setup() override;
            void loop() override;
            void set_radio(wmbus_radio::Radio *radio) { this->radio = radio; };
            void set_display_manager(wmbus_gateway_gui::DisplayManager *manager) { this->display_manager = manager; };

        protected:
            wmbus_radio::Radio *radio;
            Config config_;
            wmbus_gateway_gui::DisplayManager *display_manager;
            Frontend frontend;
        };

    }
}