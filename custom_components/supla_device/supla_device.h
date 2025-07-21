#pragma once

#include "aux_config.h"

#include "esphome/core/component.h"

#include "SuplaDevice.h"

namespace esphome
{
    namespace supla_device
    {

        class SuplaDeviceComponent : public Component
        {
        public:
            void setup() override;
            void loop() override;

        protected:
            enum ConfigMode : uint8_t
            {
                CONFIG_MODE_DEFAULT = 0,
                CONFIG_MODE_ALWAYS_ON = 1,
            };
            using ConfigModeOption = EnumOption<ConfigMode>;
            static const ConfigModeOption config_mode_;
        };

    }
}