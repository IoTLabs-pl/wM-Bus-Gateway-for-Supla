#include "supla_wmbus.h"

#include <functional>

#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/wmbus_common/meters.h"

namespace esphome
{
  namespace supla_wmbus_gateway
  {

    static const char *TAG = "supla_wmbus";

    const Component::LEDModeOption Component::led_mode_{
        "led_mode",
        "LED Mode",
        {
            {LEDMode::LED_OFF, "OFF"},
            {LEDMode::LED_ALWAYS, "ON"},
            {LEDMode::LED_MATCH, "ON METER MATCH"},
        }};

    void Component::setup()
    {
      auto config_entries = this->config_.pull();
      for (auto &entry : config_entries)
      {
        auto meter = entry.build_meter(this->radio, this->display_manager);
        if (meter)
          this->meters.push_back(meter);
      }

      switch (this->led_mode_.load_from_storage())
      {
      case LEDMode::LED_ALWAYS:
        this->radio->add_frame_handler(std::bind(&Component::blink, this));
        break;
      case LEDMode::LED_MATCH:
        for (const auto meter : this->meters)
          meter->on_telegram(std::bind(&Component::blink, this));
        break;
      }
    }

    void Component::blink() const
    {
      this->blinker_script->execute();
    }

    void Component::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Supla wM-Bus Gateway:");
      ESP_LOGCONFIG(TAG, "  Meters configured: %zu", this->meters.size());
      ESP_LOGCONFIG(TAG, "  LED Mode: %s", this->led_mode_.current_value_c_str());

      for (const auto meter : this->meters)
        meter->dump_config();
    }

  };

}
