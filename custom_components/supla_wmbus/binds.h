#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "esphome/components/wmbus_common/component.h"
#include "esphome/components/wmbus_common/meters.h"
#include "esphome/components/wmbus_meter/sensor.h"
#include "esphome/components/wmbus_gateway_gui/display_manager.h"

#include "web_server.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        struct BindField
        {
            std::string name;
            std::function<void(void *, uint8_t, float)> setter;
            uint8_t count;
        };

        class SuplaObjectWrapperBase
        {
        public:
            static const std::vector<BindField> bindings;
            virtual ~SuplaObjectWrapperBase() = default;

            // TODO: Remove virtual method in favour of passing reference
            // to bindings in constructor
            virtual const std::vector<BindField> &get_bindings() const { return this->bindings; }
            virtual void* get_supla_object(){return nullptr;}
        };

        template <typename T>
        class SuplaObjectWrapper : public SuplaObjectWrapperBase
        {
        public:
            SuplaObjectWrapper();
            static const std::vector<BindField> bindings;
            const std::vector<BindField> &get_bindings() const override { return this->bindings; }
            void* get_supla_object() override { return this->object_.get(); }
        private:
            std::unique_ptr<T> object_;
        };

        class ConfigEntry : public std::vector<std::string>
        {
        public:
            ConfigEntry(const std::string &data = "");
            void render_html(Supla::WebSender *sender);
            static std::vector<std::string> split_string(std::string serialized, size_t minimum_size = 0);
            bool is_valid();

        protected:
            const std::vector<BindField> &bindings;
            const std::vector<BindField> &lookup_bindings();
        };

        // TODO: Inherit from list<ConfigEntry>
        class Config
        {
        public:
            void pull();
            void push();
            void enqueue_entry(const std::string &data);
            void render_html(Supla::WebSender *sender);
            std::list<ConfigEntry> valid_entries();

        protected:
            std::list<ConfigEntry> data_;
            std::list<std::string> post_data_;
        };

        class SensorWrapper
        {
        public:
            SensorWrapper(
                wmbus_meter::Meter *meter,
                const std::string &name,
                const std::string &field,
                wmbus_gateway_gui::DisplayManager *display_manager,
                std::function<void(float)> &&callback);

        protected:
            wmbus_meter::Sensor sensor_;
            std::string name_;
            std::string unit_;
        };

        class Meter
        {
        public:
            static std::list<Meter *> create_from_config(Config &config);
            Meter(ConfigEntry e);
            std::list<SensorWrapper> build_sensors(wmbus_radio::Radio *radio, wmbus_gateway_gui::DisplayManager *display_manager);

        protected:
            ConfigEntry config_;
            std::unique_ptr<SuplaObjectWrapperBase> supla_object_;
            wmbus_meter::Meter wmbus_meter_;
            std::list<SensorWrapper> sensors_;
        };

    } // namespace supla_wmbus_reader
} // namespace esphome