#pragma once

#include <functional>
#include <memory>
#include <set>
#include <tuple>
#include <vector>

#include "esphome/components/wmbus_common/component.h"
#include "esphome/components/wmbus_meter/sensor.h"
#include "esphome/components/wmbus_gateway_gui/display_manager.h"

#include "supla/sensor/electricity_meter.h"
#include "supla/sensor/virtual_impulse_counter.h"
#include "supla/storage/config.h"

#include "web_server.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        struct BindConfigBase{

            
        };

        template <typename T>
        struct BindConfig
        {
            std::string name;
            std::function<void(T&, uint8_t, float)> setter;
            uint8_t idx;
        };

#define BIND_CONFIG(NAME, METHOD, MULTIPLIER) \
    BindConfig{NAME, [](void *item, uint8_t idx, float val) { static_cast<BIND_CONFIG_TYPE *>(item)->METHOD(val * MULTIPLIER); }, 0}
#define BIND_CONFIG_PH(NAME, METHOD, MULTIPLIER) \
    BindConfig{NAME, [](void *item, uint8_t idx, float val) { static_cast<BIND_CONFIG_TYPE *>(item)->METHOD(idx, val * MULTIPLIER); }, 3}

        static std::vector<BindConfig> empty_bindings{};

#define BIND_CONFIG_TYPE Supla::Sensor::VirtualImpulseCounter
        static std::vector<BindConfig> flow_meter_bindings{
            BIND_CONFIG("Counter", setCounter, 1e3),
        };
#undef BIND_CONFIG_TYPE

#define BIND_CONFIG_TYPE Supla::Sensor::ElectricityMeter
        static std::vector<BindConfig> electricity_meter_bindings{
            BIND_CONFIG("Forward Balanced Energy", setFwdBalancedEnergy, 1e5),
            BIND_CONFIG("Reverse Balanced Energy", setRvrBalancedEnergy, 1e5),
            BIND_CONFIG_PH("Forward Active Energy", setFwdActEnergy, 1e5),
            BIND_CONFIG_PH("Reverse Active Energy", setRvrActEnergy, 1e5),
            BIND_CONFIG_PH("Forward Reactive Energy", setFwdReactEnergy, 1e5),
            BIND_CONFIG_PH("Reverse Reactive Energy", setRvrReactEnergy, 1e5),
            BIND_CONFIG_PH("Voltage", setVoltage, 1e2),
            BIND_CONFIG_PH("Current", setCurrent, 1e3),
            BIND_CONFIG("Frequency", setFreq, 1e2),
            BIND_CONFIG_PH("Active Power", setPowerActive, 1e5),
            BIND_CONFIG_PH("Reactive Power", setPowerReactive, 1e5),
            BIND_CONFIG_PH("Apparent Power", setPowerApparent, 1e5),
            BIND_CONFIG_PH("Power Factor", setPowerFactor, 1e3),
            BIND_CONFIG_PH("Phase Angle", setPhaseAngle, 1e1),
        };
#undef BIND_CONFIG_TYPE

        class Config;
        class Meter;

        class ConfigEntry
        {
            friend class Config;
            friend class Meter;

        public:
            ConfigEntry(const std::string &data = "")
            {
                this->data_ = split_string(data);

                if (this->data_.size() >= 2)
                {
                    auto driver = lookupDriver(this->data_.at(1));

                    switch (driver ? driver->type() : MeterType::UnknownMeter)
                    {
                    case MeterType::WaterMeter:
                    case MeterType::GasMeter:
                        this->bindings_ = flow_meter_bindings;
                        break;
                    case MeterType::ElectricityMeter:
                        this->bindings_ = electricity_meter_bindings;
                        break;
                    }
                }

                if (this->data_.size() != this->bindings_.size() + 3)
                {
                    this->data_.resize(3);
                    this->data_.resize(this->bindings_.size() + 3);
                }
            }
            // Iterator over (BindConfig&, std::string&) pairs
            class iterator
            {
            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type = std::pair<BindConfig &, std::string &>;
                using difference_type = std::ptrdiff_t;
                using pointer = value_type *;
                using reference = value_type;

                iterator(std::vector<BindConfig> *bindings, std::vector<std::string> *data, size_t pos)
                    : bindings_(bindings), data_(data), pos_(pos) {}

                reference operator*() const
                {
                    return reference((*bindings_)[pos_], (*data_)[pos_ + 3]);
                }

                iterator &operator++()
                {
                    ++pos_;
                    return *this;
                }

                bool operator!=(const iterator &other) const
                {
                    return pos_ != other.pos_ || bindings_ != other.bindings_ || data_ != other.data_;
                }

            protected:
                std::vector<BindConfig> *bindings_;
                std::vector<std::string> *data_;
                size_t pos_;
            };

            iterator begin()
            {
                return iterator(&bindings_, &data_, 0);
            }

            iterator end()
            {
                return iterator(&bindings_, &data_, bindings_.size());
            }

            void render_html_config(Supla::WebSender *sender)
            {
                sender->send("<div class=\"box collapsible collapsed meter\">");

                sender->send("<input class=meter_cfg type=hidden>"
                             "<h3></h3>");

                InputElement{"ID", this->data_.at(0), "number"}.render(sender);
                SelectElement{"Driver", this->data_.at(1), wmbus_common::driver_names}.render(sender);
                InputElement{"Key", this->data_.at(2)}.render(sender);

                // auto [fields, per_phase_fields] = driver_available_fields();

                for (auto p : *this)
                {
                    InputElement el = {p.first.name, p.second};
                    if (p.first.idx > 1)
                        el.label += " (<em>%d-th</em> phase)";

                    el.render(sender);
                }

                sender->send("<div class=form-field>"
                             "<button type=button onclick=remove_meter() >Remove</button>"
                             "</div>"
                             "</div>");
            }
            std::string serialized() const
            {
                std::string serialized = this->data_.at(0);

                for (size_t i = 1; i < this->data_.size(); ++i)
                    serialized += ',' + this->data_.at(i);

                return serialized;
            }
            static std::vector<std::string> split_string(std::string serialized)
            {
                std::vector<std::string> result;
                size_t pos = 0;
                while ((pos = serialized.find(',')) != std::string::npos)
                {
                    result.push_back(serialized.substr(0, pos));
                    serialized.erase(0, pos + 1);
                }
                if (!serialized.empty())
                    result.push_back(serialized);
                return result;
            }
            std::vector<std::string> data_;
            std::vector<BindConfig> &bindings_ = empty_bindings;
        };

        class Config
        {
            friend class Meter;

        public:
            void pull()
            {
                if (!this->is_dirty_)
                    return;

                auto cfg = Supla::Storage::ConfigInstance();
                if (!cfg)
                    return;

                this->data_ = {};

                char key[SUPLA_CONFIG_MAX_KEY_SIZE];
                char value[300];
                uint8_t i = 0;

                while (true)
                {
                    Supla::Config::generateKey(key, i, "meter");
                    if (!cfg->getString(key, value, sizeof(value) - 1))
                        break;
                    this->data_.emplace_back(value);
                    i += 1;
                }

                this->is_dirty_ = false;
            }

            void push()
            {
                auto cfg = Supla::Storage::ConfigInstance();
                if (!cfg)
                    return;

                uint8_t i;
                char key[SUPLA_CONFIG_MAX_KEY_SIZE];

                for (i = 0; i < this->post_data_.size(); i++)
                {
                    Supla::Config::generateKey(key, i, "meter");
                    auto data = this->post_data_[i];
                    ESP_LOGW("WM", "Saving: %s", data.c_str());

                    cfg->setString(key, data.c_str());
                }

                while (true)
                {
                    Supla::Config::generateKey(key, i, "meter");
                    if (!cfg->eraseKey(key))
                        break;
                }

                this->post_data_ = {};
                this->is_dirty_ = true;
            }

            void add_entry(const std::string &data)
            {
                this->post_data_.emplace_back(data);
            }

            void render_html(Supla::WebSender *sender)
            {
                if (this->data_.empty())
                    ConfigEntry().render_html_config(sender);
                else
                    for (auto &entry : this->data_)
                        entry.render_html_config(sender);
            }

        protected:
            std::vector<ConfigEntry> data_;
            std::vector<std::string> post_data_;
            bool is_dirty_ = true;
        };

        class SensorWrapper
        {
        public:
            SensorWrapper(
                wmbus_meter::Meter *meter,
                void *supla_object,
                const std::string &field,
                const BindConfig &b,
                int8_t i)
            {
                Unit unit;
                extractUnit(field, &this->name_, &unit);

                this->name_ = meter->get_id() + ' ' + b.name;
                if (b.idx > 0)
                    this->name_ += ' ' + std::to_string(i);

                this->unit_ = unitToStringHR(unit);

                this->sensor_.set_unit_of_measurement(this->unit_.c_str());
                this->sensor_.set_name(this->name_.c_str());
                this->sensor_.set_field_name(field);
                this->sensor_.set_parent(meter);
                this->sensor_.add_on_state_callback([supla_object, b, i](float val)
                                                    { b.setter(supla_object, i, val); });
            }

            void install_on_display(wmbus_gateway_gui::DisplayManager *display_manager)
            {
                display_manager->add_sensor(&this->sensor_);
            }

        protected:
            wmbus_meter::Sensor sensor_;
            std::string name_;
            std::string unit_;
        };

        class Meter
        {
        public:
            static std::vector<Meter *> create_from_config(Config &config)
            {
                std::vector<Meter *> meters;
                for (auto &e : config.data_)
                {
                    if (e.data_.size() < 3)
                        continue; // Invalid entry, skip it

                    meters.push_back(new Meter(e));
                }
                return meters;
            }

            Meter(ConfigEntry &e)
            {
                this->wmbus_meter_.set_meter_params(e.data_.at(0), e.data_.at(1), e.data_.at(2));
                switch (this->wmbus_meter_.get_type())
                {
                case MeterType::WaterMeter:
                case MeterType::GasMeter:
                    this->supla_object_ = std::unique_ptr<void>(new Supla::Sensor::VirtualImpulseCounter{});
                    break;
                case MeterType::ElectricityMeter:
                    this->supla_object_ = std::unique_ptr<void>(new Supla::Sensor::ElectricityMeter{});
                    break;
                }
                this->build_sensors(e);
            }

            void attach_hardware(wmbus_radio::Radio *radio, wmbus_gateway_gui::DisplayManager *display_manager)
            {
                this->wmbus_meter_.set_radio(radio);
                for (auto sensor : this->sensors_)
                    sensor.install_on_display(display_manager);
            }

        protected:
            void build_sensors(ConfigEntry &e)
            {
                for (const auto &p : e)
                {
                    auto &b = p.first;
                    auto &val = p.second;

                    if (!val.empty())
                    {
                        auto pos = val.find("%d");

                        for (uint8_t i = 0; i < (pos != std::string::npos ? b.idx : 1); ++i)
                        {
                            auto field_name = val;
                            if (pos != std::string::npos)
                                field_name = field_name.replace(pos, 2, std::to_string(i));

                            this->sensors_.emplace_back(&this->wmbus_meter_,
                                                        this->supla_object_.get(),
                                                        field_name,
                                                        b,
                                                        i);
                        }
                    }
                }
            }

            wmbus_meter::Meter wmbus_meter_;
            std::unique_ptr<void> supla_object_;
            std::vector<SensorWrapper> sensors_;
        };

    };
};