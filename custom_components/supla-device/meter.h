#pragma once

#include "esphome/components/wmbus_common/meters.h"
#include "esphome/components/wmbus_meter/wmbus_meter.h"
#include "esphome/components/wmbus_meter/sensor.h"

#include <supla/network/web_sender.h>

namespace esphome
{
    namespace supla_wmbus_reader
    {
        class Component;

        class Meter : public wmbus_meter::Meter
        {
        public:
            static Meter *deserialize(std::string serialized);
            virtual std::string serialize();

            void render_html_config(Supla::WebSender *sender);

            std::vector<wmbus_meter::Sensor *> sensors;
        protected:
            virtual void render_binding_config(Supla::WebSender *sender) {};
        };

        class FlowMeter : public Meter
        {
        public:
            static FlowMeter *deserialize(std::vector<std::string> serialized);
            std::string serialize() override;

        protected:
            void render_binding_config(Supla::WebSender *sender) override;
            std::string binding_field;
        };
    }
}