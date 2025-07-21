#include "supla_wmbus.h"

#include <functional>

#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/wmbus_common/meters.h"

namespace esphome
{
  namespace supla_wmbus_reader
  {
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
        auto meter = entry.create_meter();
        if (!meter)
          continue;

        meter->set_radio(this->radio);

        for (auto &sensor : meter->create_sensors())
          this->display_manager->add_sensor(&sensor);

        this->meters.push_back(std::move(meter));
      }

      switch (this->led_mode_.load_from_storage())
      {
      case LEDMode::LED_ALWAYS:
        this->radio->add_frame_handler(std::bind(&Component::blink, this));
        break;
      case LEDMode::LED_MATCH:
        for (const auto &meter : this->meters)
          meter->on_telegram(std::bind(&Component::blink, this));
        break;
      }
    }

    void Component::blink() const
    {
      this->blinker_script->execute();
    }

  };

}
