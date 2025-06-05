#include "binds.h"

#include "supla/sensor/electricity_meter.h"
#include "supla/sensor/virtual_impulse_counter.h"
#include "supla/storage/config.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        const std::vector<BindField> SuplaObjectWrapperBase::bindings = {};

// Macros for concise binding config
#define FIELD(NAME, METHOD, MULTIPLIER) \
    BindField{                          \
        NAME, [](void *obj, uint8_t idx, float val) { \
            ESP_LOGW("WM", "Calling " #METHOD " with index %d and value %f", idx, val * MULTIPLIER); \
            static_cast<T *>(obj)->METHOD(idx, val * MULTIPLIER+0.5f); }, 3}

#define FIELD_NOIDX(NAME, METHOD, MULTIPLIER) \
    BindField{NAME, [](void *obj, uint8_t, float val) { \
        ESP_LOGW("WM", "Calling " #METHOD " with value %f", val * MULTIPLIER); \
        static_cast<T *>(obj)->METHOD(val * MULTIPLIER+0.5f); }, 1}

#define T Supla::Sensor::ElectricityMeter
        template <>
        const std::vector<BindField> SuplaObjectWrapper<T>::bindings = {
            FIELD_NOIDX("Forward Balanced Energy", setFwdBalancedEnergy, 1e5f),
            FIELD_NOIDX("Reverse Balanced Energy", setRvrBalancedEnergy, 1e5f),
            FIELD("Forward Active Energy", setFwdActEnergy, 1e5f),
            FIELD("Reverse Active Energy", setRvrActEnergy, 1e5f),
            FIELD("Forward Reactive Energy", setFwdReactEnergy, 1e5f),
            FIELD("Reverse Reactive Energy", setRvrReactEnergy, 1e5f),
            FIELD("Voltage", setVoltage, 1e2f),
            FIELD("Current", setCurrent, 1e3f),
            FIELD_NOIDX("Frequency", setFreq, 1e2f),
            FIELD("Active Power", setPowerActive, 1e8f),
            FIELD("Reactive Power", setPowerReactive, 1e8f),
            FIELD("Apparent Power", setPowerApparent, 1e8f),
            FIELD("Power Factor", setPowerFactor, 1e3f),
            FIELD("Phase Angle", setPhaseAngle, 1e1f)};
#undef T

#define T Supla::Sensor::VirtualImpulseCounter
        template <>
        const std::vector<BindField> SuplaObjectWrapper<T>::bindings = {
            FIELD_NOIDX("Counter", setCounter, 1e3)};
#undef T

#undef FIELD
#undef FIELD_NOIDX

        template <typename T>
        SuplaObjectWrapper<T>::SuplaObjectWrapper() : object_(new T{}) {}

        ConfigEntry::ConfigEntry(const std::string &data)
            : std::vector<std::string>{split_string(data, 3)}, bindings{this->lookup_bindings()}
        {
            size_t actual_size = this->size();
            size_t required_size = 3 + this->bindings.size();

            if (actual_size != required_size)
            {
                ESP_LOGW("WM", "ConfigEntry size mismatch: expected %zu, got %zu", required_size, actual_size);
                this->resize(3);
                this->resize(required_size);
            }
        }

        void ConfigEntry::render_html(Supla::WebSender *sender)
        {
            sender->send("<div class=\"box collapsible collapsed meter\">"
                         "<input class=meter_cfg type=hidden>"
                         "<h3></h3>");

            InputElement{"ID", this->at(0), "number"}.render(sender);
            SelectElement{"Driver", this->at(1), wmbus_common::driver_names}.render(sender);
            InputElement{"Key", this->at(2)}.render(sender);

            for (uint8_t i = 0; i < this->bindings.size(); ++i)
            {
                auto label = this->bindings[i].name;
                if (this->bindings[i].count > 1)
                    label += " (<em>%d-th</em>)";
                auto value = this->at(i + 3);

                InputElement{label, value}.render(sender);
            }

            sender->send("<div class=form-field>"
                         "<button type=button onclick=remove_meter() >Remove</button>"
                         "</div>"
                         "</div>");
        }

        std::vector<std::string> ConfigEntry::split_string(std::string serialized, size_t minimum_size)
        {
            std::list<std::string> result;
            size_t pos = 0;
            while ((pos = serialized.find(',')) != std::string::npos)
            {
                result.push_back(serialized.substr(0, pos));
                serialized.erase(0, pos + 1);
            }
            result.push_back(serialized); // Add the last part after the last comma

            if (result.size() < minimum_size)
                result.resize(minimum_size);

            return {result.cbegin(), result.cend()};
        }

        bool ConfigEntry::is_valid()
        {
            return !(this->at(0).empty() || this->at(1).empty());
        }

        const std::vector<BindField> &ConfigEntry::lookup_bindings()
        {
            DriverInfo *driver = nullptr;
            if (this->is_valid() && (driver = lookupDriver(this->at(1))))
                switch (driver->type())
                {
                case MeterType::WaterMeter:
                case MeterType::GasMeter:
                    return SuplaObjectWrapper<Supla::Sensor::VirtualImpulseCounter>::bindings;
                case MeterType::ElectricityMeter:
                    return SuplaObjectWrapper<Supla::Sensor::ElectricityMeter>::bindings;
                }
            return SuplaObjectWrapperBase::bindings;
        }

        void Config::pull()
        {
            auto cfg = Supla::Storage::ConfigInstance();
            if (!cfg)
                return;

            this->data_ = std::list<ConfigEntry>{};

            char key[SUPLA_CONFIG_MAX_KEY_SIZE];
            char value[300];
            uint8_t i = 0;

            while (true)
            {
                Supla::Config::generateKey(key, i, "meter");
                if (!cfg->getString(key, value, sizeof(value) - 1))
                    break;
                ESP_LOGW("WM", "Loaded: %s", value);
                this->data_.emplace_back(value);
                i += 1;
            }
        }

        void Config::push()
        {
            auto cfg = Supla::Storage::ConfigInstance();
            if (!cfg)
                return;

            uint8_t i;
            char key[SUPLA_CONFIG_MAX_KEY_SIZE];

            for (auto &data : this->post_data_)
            {
                Supla::Config::generateKey(key, i, "meter");
                ESP_LOGW("WM", "Saving: %s", data.c_str());
                // TODO: Push only valid entries
                cfg->setString(key, data.c_str());
                ++i;
            }

            while (true)
            {
                Supla::Config::generateKey(key, i, "meter");
                if (!cfg->eraseKey(key))
                    break;
            }

            this->data_ = {this->post_data_.begin(), this->post_data_.end()};
            this->post_data_ = {};
        }

        void Config::enqueue_entry(const std::string &data)
        {
            this->post_data_.emplace_back(data);
        }

        void Config::render_html(Supla::WebSender *sender)
        {
            if (this->data_.empty())
                ConfigEntry().render_html(sender);
            else
                for (auto &entry : this->data_)
                    entry.render_html(sender);
        }

        std::list<ConfigEntry> Config::valid_entries()
        {
            std::list<ConfigEntry> valid_entries = this->data_;
            valid_entries.remove_if([](ConfigEntry &e)
                                    { return !e.is_valid(); });

            return valid_entries;
        }

        SensorWrapper::SensorWrapper(
            wmbus_meter::Meter *meter,
            const std::string &name,
            const std::string &field,
            wmbus_gateway_gui::DisplayManager *display_manager,
            std::function<void(float)> &&callback)
        {
            Unit unit;
            extractUnit(field, &this->name_, &unit);

            this->name_ = meter->get_id() + ' ' + name;
            this->unit_ = unitToStringHR(unit);

            ESP_LOGW("WM", "Constructing sensor [%zu]: %s (%s) with unit %s",
                     this,
                     this->name_.c_str(),
                     field.c_str(),
                     this->unit_.c_str());

            this->sensor_.set_field_name(field);
            this->sensor_.set_parent(meter);
            this->sensor_.add_on_state_callback(std::move(callback));

            this->sensor_.set_unit_of_measurement(this->unit_.c_str());
            this->sensor_.set_name(this->name_.c_str());
            display_manager->add_sensor(&(this->sensor_));
        }

        std::list<Meter *> Meter::create_from_config(Config &config)
        {
            std::list<Meter *> meters;
            for (auto e : config.valid_entries())
                meters.push_back(new Meter(e));

            return meters;
        }

        Meter::Meter(ConfigEntry e) : config_(e)
        {
            this->wmbus_meter_.set_meter_params(e.at(0), e.at(1), e.at(2));
            switch (this->wmbus_meter_.get_type())
            {
            case MeterType::WaterMeter:
            case MeterType::GasMeter:
                this->supla_object_ = std::unique_ptr<SuplaObjectWrapperBase>(new SuplaObjectWrapper<Supla::Sensor::VirtualImpulseCounter>());
                break;
            case MeterType::ElectricityMeter:
                this->supla_object_ = std::unique_ptr<SuplaObjectWrapperBase>(new SuplaObjectWrapper<Supla::Sensor::ElectricityMeter>());
                break;
            default:
                this->supla_object_ = std::unique_ptr<SuplaObjectWrapperBase>(new SuplaObjectWrapperBase());
                break;
            }
        }

        std::list<SensorWrapper> Meter::build_sensors(wmbus_radio::Radio *radio, wmbus_gateway_gui::DisplayManager *display_manager)
        {
            this->wmbus_meter_.set_radio(radio);
            auto &bindings = this->supla_object_->get_bindings();

            ESP_LOGW("WM", "Building sensors for %s", this->wmbus_meter_.get_id().c_str());
            ESP_LOGW("WM", "Found %d bindings", bindings.size());

            for (uint8_t i = 0; i < bindings.size(); ++i)
            {
                auto &binding = bindings[i];
                auto &value = this->config_.at(i + 3);

                ESP_LOGW("WM", "Binding: %s (%s)", binding.name.c_str(), value.c_str());

                if (!value.empty())
                {
                    auto pos = value.find("%d");

                    for (uint8_t i = 0; i < (pos != std::string::npos ? binding.count : 1); ++i)
                    {
                        auto field_name = value;
                        auto sensor_name = binding.name;
                        if (pos != std::string::npos)
                        {
                            field_name = field_name.replace(pos, 2, std::to_string(i));
                            sensor_name += ' ' + std::to_string(i + 1);
                        }
                        this->sensors_.emplace_back(
                            &this->wmbus_meter_,
                            sensor_name,
                            field_name,
                            display_manager,
                            [&setter = binding.setter, object = this->supla_object_->get_supla_object(), i](float value)
                            { setter(object, i, value); });
                    }
                }
            }

            return this->sensors_;
        }

    } // namespace supla_wmbus_reader
} // namespace esphome
