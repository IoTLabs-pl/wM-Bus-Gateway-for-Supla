#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <list>

#include "esphome/core/log.h"
#include "esphome/components/wmbus_common/meters.h"
#include "sensor.h"

#define INDEXABLE_CB(T, name, method, multiplier) \
    {name, true, multiplier, [](void *obj, uint8_t idx, float val) { static_cast<T *>(obj)->method(idx, val); }}
#define NON_INDEXABLE_CB(T, name, method, multiplier) \
    {name, false, multiplier, [](void *obj, uint8_t, float val) { static_cast<T *>(obj)->method(val); }}

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        class ConfigEntry;
        
        struct CallbackMetadata
        {
            const char *name;
            bool indexable;
            float multiplier;
            std::function<void(void *, uint8_t, float)> setter;
        };

        class MeterBase : public wmbus_meter::Meter
        {
        public:
            MeterBase(const std::vector<CallbackMetadata> &callback_metadata);
            std::list<Sensor> &create_sensors(const ConfigEntry *config);
            const std::vector<CallbackMetadata> &callback_metadata_;

        protected:
            std::list<Sensor> sensors_;
        };
    }
}
