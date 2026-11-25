#include "config.h"

#include <algorithm>

#include "supla/sensor/electricity_meter.h"
#include "supla/sensor/virtual_impulse_counter.h"
#include "supla/storage/config.h"
#include "supla/storage/storage.h"

#include "esphome/core/alloc_helpers.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/wmbus_common/meter.h"

#include "resources.h"
#include "meter_impulse.h"
#include "meter_electricity.h"

#define GENERATE_KEY(varname, index)         \
    char varname[SUPLA_CONFIG_MAX_KEY_SIZE]; \
    Supla::Config::generateKey(varname, index, "meter");

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        static const char *TAG = "supla.wmbus.config";

        ConfigEntry ConfigEntry::parse(const std::string &serialized)
        {
            ConfigEntry entry;
            size_t start = 0;
            for (auto *part : {&entry.id, &entry.driver, &entry.key})
            {
                const size_t end = serialized.find(',', start);
                *part = serialized.substr(start, end - start);
                if (end == std::string::npos)
                    break;
                start = end + 1;
            }
            return entry;
        }

        void ConfigEntry::renderHtml(Supla::WebSender *sender, const ReceivedMeters &received,
                                     const ReceivedMeters::Entry *heard, bool added) const
        {
            const bool switchable = added || this->meter_kind() != MeterKind::NONE;

            sender->tag("div").attr("class", "form-field meter").body([&]()
                                                                      {
                sender->tag("h3").body([&]() {
                    sender->sendSafe(this->id.empty() ? "New Meter" : (this->id + ' ' + this->driver).c_str());
                    if (switchable)
                        sender->tag("label").body([&]() {
                            sender->tag("span").attr("class", "switch").body([&]() {
                                sender->checkboxInput("meter", nullptr, added, (this->id + ',' + this->driver).c_str());
                                sender->tag("span").attr("class", "slider").body("");
                            });
                        });
                });
                received.send_status(sender, heard, this->normalized_id());
                if (switchable && (!this->key.empty() || !heard || heard->needs_key))
                    sender->voidTag("input")
                        .attr("type", "text")
                        .attr("name", ("key_" + this->id).c_str())
                        .attr("placeholder", "Key")
                        .attr("value", this->key.c_str())
                        .attr("pattern", "[0-9a-fA-F]{0,32}"); });
        }

        std::string ConfigEntry::normalized_id() const
        {
            return str_sprintf("%08lx", strtoul(this->id.c_str(), nullptr, 16));
        }

        MeterKind ConfigEntry::meter_kind() const
        {
            for (const auto &driver : DRIVERS)
                if (this->driver == driver.name)
                    return driver.kind;
            return MeterKind::NONE;
        }

        const wmbus::DriverSpec *ConfigEntry::spec() const
        {
            for (const auto *spec : wmbus::registered_drivers)
                if (this->driver == spec->name)
                    return spec;
            return nullptr;
        }

        MeterBase *ConfigEntry::build_meter(wmbus_radio::Radio *radio, wmbus_gateway::DisplayManager *display_manager) const
        {
            MeterBase *meter;
            switch (this->meter_kind())
            {
            case MeterKind::NONE:
                // e.g. a driver dropped from wmbus_common since the meter was stored
                ESP_LOGW(TAG, "Skipping meter %s: no Supla channel for driver '%s'", this->id.c_str(), this->driver.c_str());
                return nullptr;
            case MeterKind::ELECTRICITY:
                meter = new ElectricityMeter{this};
                break;
            default:
                meter = new ImpulseCounter{this};
            }

            std::optional<std::array<uint8_t, 16>> key;
            if (!this->key.empty())
            {
                key.emplace();
                if (!parse_hex(this->key, key->data(), key->size()))
                {
                    ESP_LOGW(TAG, "Ignoring invalid key of meter %s", this->id.c_str());
                    key.reset();
                }
            }

            // DRIVERS lists only registered drivers, so the spec is always found
            meter->set_driver(this->spec(), strtoul(this->id.c_str(), nullptr, 16), key);

            auto &sensors = meter->create_sensors();

            if (radio)
                meter->set_radio(radio);

            if (display_manager)
                for (auto &sensor : sensors)
                    display_manager->add_sensor(&sensor);

            ESP_LOGD(TAG, "Built meter with ID: %s, Driver: %s", meter->get_id().c_str(), this->driver.c_str());
            return meter;
        }

        std::list<ConfigEntry> Config::pull()
        {
            auto cfg = Supla::Storage::ConfigInstance();
            if (!cfg)
                return {};

            std::list<ConfigEntry> result;
            char value[300];
            uint8_t i = 0;

            while (true)
            {
                GENERATE_KEY(key, i);
                if (!cfg->getString(key, value, sizeof(value) - 1))
                    break;
                ESP_LOGD(TAG, "Loaded: %s", value);
                result.push_back(ConfigEntry::parse(value));
                ++i;
            }
            return result;
        }

        void Config::send(Supla::WebSender *sender)
        {
            const auto entries = Config::pull();
            const auto heard = this->received_.snapshot();

            const auto heard_entry = [&](const std::string &id) -> const ReceivedMeters::Entry *
            {
                auto it = std::ranges::find(heard, id, &ReceivedMeters::Entry::id);
                return it == heard.end() ? nullptr : &*it;
            };
            const auto configured = [&](const std::string &id)
            { return std::ranges::any_of(entries, [&](const ConfigEntry &e) { return e.normalized_id() == id; }); };

            sender->tag("div").attr("class", "box").body([&]()
                                                         {
                sender->tag("script").body([&](){
                    sender->send(frontend_script);
                });
                sender->tag("style").body([&]() {
                    sender->send(".form-field.meter{display:block}"
                                 ".meter h3{display:flex;justify-content:space-between;align-items:center}"
                                 ".meter h3 label{width:auto;margin:0}");
                });
                sender->tag("h3").body("wM-Bus Meters");

                for (const auto &entry : entries)
                    entry.renderHtml(sender, this->received_, heard_entry(entry.normalized_id()), true);

                for (const auto &meter : heard)
                    if (!configured(meter.id))
                        ConfigEntry{meter.id, meter.driver ? meter.driver->name : "unknown", ""}
                            .renderHtml(sender, this->received_, &meter, false);

                sender->tag("template").attr("id", "meter_template").body([&]() {
                    ConfigEntry{}.renderHtml(sender, this->received_, nullptr, true);
                });
                sender->tag("dialog").attr("id", "meter_dialog").attr("closedby", "any").body([&]() {
                    sender->formField([&]() {
                        sender->voidTag("input").attr("type", "text").attr("placeholder", "Meter ID").attr("required", "").attr("pattern", "[0-9a-fA-F]{1,8}");
                    });
                    sender->formField([&]() {
                        sender->selectInput(nullptr, nullptr, [&]() {
                            for (const auto &driver : DRIVERS)
                                sender->selectOption(driver.name, driver.name, false);
                        });
                    });
                    sender->tag("button").attr("type", "button").body("Add");
                });
                sender->tag("button").attr("type", "button").attr("id", "new_meter").body("New Meter"); });
        }

        // A switched-on meter posts "meter=<id>,<driver>", then its "key_<id>"
        bool Config::handleResponse(const char *key, const char *value)
        {
            if (strcmp(key, "meter") == 0)
            {
                this->store_pending();
                this->pending_ = std::string(value) + ',';
                return true;
            }

            if (strncmp(key, "key_", 4) == 0 && this->pending_.starts_with(std::string(key + 4) + ','))
            {
                this->pending_ += value;
                return true;
            }

            return false;
        }

        void Config::store_pending()
        {
            if (this->pending_.empty())
                return;

            GENERATE_KEY(storage_key, this->post_data_counter_);
            Supla::Storage::ConfigInstance()->setString(storage_key, this->pending_.c_str());
            ESP_LOGD(TAG, "Stored %s->%s", storage_key, this->pending_.c_str());

            this->post_data_counter_ += 1;
            this->pending_.clear();
        }

        void Config::onProcessingEnd()
        {
            this->store_pending();

            if (auto cfg = Supla::Storage::ConfigInstance())
                while (true)
                {
                    GENERATE_KEY(storage_key, this->post_data_counter_);
                    if (!cfg->eraseKey(storage_key))
                        break;
                    this->post_data_counter_ += 1;
                }

            this->post_data_counter_ = 0;
        }

    } // namespace supla_wmbus_gateway
} // namespace esphome
