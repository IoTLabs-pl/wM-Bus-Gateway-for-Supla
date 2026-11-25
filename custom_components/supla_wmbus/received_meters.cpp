#include "received_meters.h"

#include <algorithm>
#include <cinttypes>
#include <climits>
#include <cstdlib>
#include <functional>

#include "esphome/core/alloc_helpers.h"
#include "esphome/core/hal.h"

namespace esphome
{
    namespace supla_wmbus_gateway
    {
        ReceivedMeters::ReceivedMeters(const std::list<MeterBase *> &meters)
            : meters_(meters)
        {
        }

        void ReceivedMeters::handle_frame(wmbus_radio::Frame *frame)
        {
            auto id = frame->meter_id();
            if (id.empty())
                return;

            const auto rank = [this](const std::string &meter_id, int8_t rssi)
            { return this->configured_meter(meter_id) ? INT_MAX : rssi; };

            LockGuard guard{this->mutex_};

            auto entry = std::ranges::find(this->entries_, id, &Entry::id);
            if (entry == this->entries_.end())
            {
                if (this->entries_.size() < MAX_ENTRIES)
                    entry = this->entries_.emplace(this->entries_.end());
                else if (auto weakest = std::ranges::min_element(this->entries_, {}, [&](const Entry &e)
                                                                 { return rank(e.id, e.rssi); });
                         rank(weakest->id, weakest->rssi) < rank(id, frame->rssi()))
                    entry = weakest;
                else
                    return;

                wmbus::MeterState probe{nullptr, static_cast<uint32_t>(strtoul(id.c_str(), nullptr, 16)), std::nullopt};
                probe.handle_telegram(frame->data());
                entry->id = id;
                entry->driver = probe.spec;
                entry->needs_key = probe.last_error.code == wmbus::DecodeError::NoKey ||
                                   probe.last_error.code == wmbus::DecodeError::DecryptionFailed;
            }

            entry->rssi = frame->rssi();
            entry->last_seen = millis();
            entry->frame = frame->data();
        }

        std::vector<ReceivedMeters::Entry> ReceivedMeters::snapshot() const
        {
            std::vector<Entry> entries;
            {
                LockGuard guard{this->mutex_};
                entries = this->entries_;
            }
            std::ranges::sort(entries, std::ranges::greater{}, &Entry::rssi);
            return entries;
        }

        void ReceivedMeters::send_status(Supla::WebSender *sender, const Entry *entry, const std::string &id) const
        {
            sender->tag("div").attr("class", "hint").body([&]()
                                                          {
                if (!entry)
                {
                    sender->send("Not heard");
                    return;
                }
                const auto decoder_url = str_sprintf("https://iotlabs.pl/decoder?telegram=%s&rssi=%d",
                                                     format_hex(entry->frame).c_str(), entry->rssi);
                sender->send(str_sprintf("%d dBm, %" PRIu32 " s ago, ", entry->rssi, (millis() - entry->last_seen) / 1000).c_str());
                sender->tag("a").attr("href", decoder_url.c_str()).attr("target", "_blank").body("decoder"); });

            const auto *meter = this->configured_meter(id);
            if (!meter || !entry)
                return;

            if (*meter->last_error())
                sender->tag("div").attr("class", "hint").body(meter->last_error());
            else
                for (const auto &sensor : meter->sensors())
                    if (sensor.has_state())
                        sender->tag("div").attr("class", "hint").body(
                            str_sprintf("%s: %.3f", sensor.get_field_name().c_str(), sensor.state).c_str());
        }

        const MeterBase *ReceivedMeters::configured_meter(const std::string &id) const
        {
            for (const auto *meter : this->meters_)
                if (meter->get_id() == id)
                    return meter;
            return nullptr;
        }
    }
}
