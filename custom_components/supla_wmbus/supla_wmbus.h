#pragma once
#include "esphome/core/component.h"

#include "esphome/components/wmbus_radio/component.h"
#include "esphome/components/wmbus_gateway/display_manager.h"
#include "esphome/components/script/script.h"

#include "SuplaDevice.h"

#include "aux_config.h"
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
            void loop() override;
            void set_radio(wmbus_radio::Radio *radio) { this->radio = radio; };
            void set_display_manager(wmbus_gateway::DisplayManager *manager) { this->display_manager = manager; };
            void set_blinker_script(script::Script<> *script) { this->blinker_script = script; };

        protected:
            void blink() const;

            std::list<Meter *> meters;
            wmbus_radio::Radio *radio;
            script::Script<> *blinker_script;
            Config config_;
            AuxConfig aux_config_;
            wmbus_gateway::DisplayManager *display_manager;
        };

    }
}