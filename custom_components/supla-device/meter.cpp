#include "meter.h"

#include "supla_wmbus.h"

#include "esphome/components/wmbus_common/component.h"
#include "esphome/components/wmbus_common/util.h"
#include "esphome/components/wmbus_meter/sensor.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        Meter *Meter::deserialize(std::string serialized)
        {
            ESP_LOGI("WM", "deserialize %s", serialized.c_str());
            Meter *meter = nullptr;

            auto parts = splitString(serialized, ',');

            if (parts.size() > 3)
            {
                auto id = parts[0];
                auto driver = parts[1];
                auto key = parts[2];

                auto drv = lookupDriver(driver);
                switch (drv->type())
                {
                case MeterType::WaterMeter:
                case MeterType::GasMeter:
                case MeterType::ElectricityMeter:
                    meter = FlowMeter::deserialize(parts);
                    break;
                }

                ESP_LOGE("WM", "%s-%s-%s", id.c_str(), driver.c_str(), key.c_str());
                meter->set_meter_params(id, driver, key);
            }

            if (meter == nullptr)
                meter = new Meter{};

            return meter;
        }

        void Meter::render_html_config(Supla::WebSender *sender)
        {
            sender->send("<div class=\"box collapsible collapsed meter\">");

            sender->send("<input class=meter_cfg type=hidden>"
                         "<h3></h3>"
                         "<div class=form-field>"
                         "<label>"
                         "ID"
                         "<input type=number class=meter_field value=\"");
            sender->send(this->id_.c_str());
            sender->send("\">"
                         "</label>"
                         "</div>"
                         "<div class=form-field>"
                         "<label>"
                         "Driver"
                         "<select class=meter_field>");

            for (auto &driver_name : wmbus_common::driver_names)
            {
                sender->send("<option");
                if (this->driver_ == driver_name)
                    sender->send(" selected");

                sender->send(">");
                sender->send(driver_name.c_str());
                sender->send("</option>");
            }
            sender->send("</select>"
                         "</label>"
                         "</div>"
                         "<div class=form-field>"
                         "<label>"
                         "Key"
                         "<input class=meter_field value=\"");
            sender->send(this->key_.c_str());
            sender->send("\">"
                         "</label>"
                         "</div>");

            ESP_LOGI("WM", "before binding render");
            this->render_binding_config(sender);
            ESP_LOGI("WM", "after binding render");

            sender->send("<div class=form-field>");
            sender->send("<button type=button onclick=remove_meter() >Remove</button>");
            sender->send("</div>");

            sender->send("</div>");
            ESP_LOGE("WM", "END");
        }

        void FlowMeter::render_binding_config(Supla::WebSender *sender)
        {
            ESP_LOGI("WM", "render_binding_config");
            sender->send("<div class=form-field>"
                         "<label>"
                         "Binding"
                         "<select class=meter_field>");
            auto drv = lookupDriver(this->driver_);

            for (auto &f : drv->defaultFields())
            {
                sender->send("<option");
                if (this->binding_field == f)
                    sender->send(" selected");

                sender->send(">");
                sender->send(f.c_str());
                sender->send("</option>");
            }

            sender->send("</select>"
                         "</label>"
                         "</div>");
        }

        std::string Meter::serialize()
        {
            return this->id_ + "," + this->driver_ + "," + this->key_;
        }

        FlowMeter *FlowMeter::deserialize(std::vector<std::string> serialized)
        {
            auto m = new FlowMeter{};
            if (serialized.size() == 4)
                m->binding_field = serialized[3];

            auto ic = new Supla::Sensor::VirtualImpulseCounter{};
            auto sensor = new wmbus_meter::Sensor{};

            std::string vname;
            Unit unit;
            extractUnit(m->binding_field, &vname, &unit);

            auto vname_str = new std::string{serialized[0] + ' ' + vname};
            auto unit_str = new std::string{unitToStringHR(unit)};

            sensor->set_unit_of_measurement(unit_str->c_str());
            sensor->set_name(vname_str->c_str());
            sensor->set_field_name(m->binding_field);
            sensor->set_parent(m);
            sensor->add_on_state_callback([ic](float val)
                                          { ic->setCounter(val * 1000.0f); });

            m->sensors.push_back(sensor);

            return m;
        }

        std::string FlowMeter::serialize()
        {
            return Meter::serialize() + ',' + this->binding_field;
        }

    }
}