#pragma once
#include "esphome/core/component.h"

#include "esphome/components/supla_device/aux_config.h"
#include "esphome/components/wmbus_radio/component.h"
#include "esphome/components/wmbus_gateway/display_manager.h"
#include "esphome/components/script/script.h"

#include "config.h"
#include "meter.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class Component : public esphome::Component
        {
        public:
            void setup() override;
            void set_radio(wmbus_radio::Radio *radio) { this->radio = radio; };
            void set_display_manager(wmbus_gateway::DisplayManager *manager) { this->display_manager = manager; };
            void set_blinker_script(script::Script<> *script) { this->blinker_script = script; };

        protected:
            void blink() const;

            enum LEDMode : uint8_t
            {
                LED_OFF = 0,
                LED_ALWAYS = 1,
                LED_MATCH = 2,
            };

            using LEDModeOption = supla_device::EnumOption<LEDMode>;

            std::list<Meter *> meters;
            wmbus_radio::Radio *radio;
            script::Script<> *blinker_script;
            Config config_;
            static const LEDModeOption led_mode_;
            wmbus_gateway::DisplayManager *display_manager;
        };

    }
}