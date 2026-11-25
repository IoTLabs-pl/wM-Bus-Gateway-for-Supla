#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <vector>

#include "esphome/core/helpers.h"
#include "esphome/components/wmbus_radio/component.h"

#include "supla/network/web_sender.h"

#include "meter_base.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        class ReceivedMeters
        {
        public:
            struct Entry
            {
                std::string id;
                const wmbus::DriverSpec *driver;
                bool needs_key;
                int8_t rssi;
                uint32_t last_seen;
                std::vector<uint8_t> frame;
            };

            explicit ReceivedMeters(const std::list<MeterBase *> &meters);
            void handle_frame(wmbus_radio::Frame *frame);
            std::vector<Entry> snapshot() const;
            void send_status(Supla::WebSender *sender, const Entry *entry, const std::string &id) const;

        protected:
            static constexpr size_t MAX_ENTRIES = 20;

            const MeterBase *configured_meter(const std::string &id) const;

            const std::list<MeterBase *> &meters_;
            std::vector<Entry> entries_;
            mutable Mutex mutex_;
        };
    }
}
