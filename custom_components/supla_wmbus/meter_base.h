#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <list>

#include "esphome/core/log.h"
#include "esphome/components/wmbus_meter/wmbus_meter.h"
#include "sensor.h"

#define INDEXABLE_CB(T, name, method, multiplier, ...) \
    {name, multiplier, [](void *obj, uint8_t idx, float val) { static_cast<T *>(obj)->method(idx, val); }, {__VA_ARGS__}}
#define NON_INDEXABLE_CB(T, name, method, multiplier, ...) \
    {name, multiplier, [](void *obj, uint8_t, float val) { static_cast<T *>(obj)->method(val); }, {__VA_ARGS__}}

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        class ConfigEntry;

        inline constexpr std::string_view PHASE_PLACEHOLDER = "%d";

        struct CallbackMetadata
        {
            const char *name;
            float multiplier;
            std::function<void(void *, uint8_t, float)> setter;
            std::vector<const char *> field_candidates;
        };

        class MeterBase : public wmbus_meter::Meter
        {
        public:
            MeterBase(const std::vector<CallbackMetadata> &callback_metadata);
            std::list<Sensor> &create_sensors();
            const std::list<Sensor> &sensors() const { return sensors_; }
            const char *last_error() const { return this->state_.last_error.name(); }
            const std::vector<CallbackMetadata> &callback_metadata_;

        protected:
            static const char *pick_field(const wmbus::DriverSpec *spec, const CallbackMetadata &callback);
            std::list<Sensor> sensors_;
        };
    }
}
