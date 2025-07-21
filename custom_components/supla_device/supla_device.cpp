#include "supla_device.h"

#include "esphome/core/application.h"
#include "esphome/core/log.h"

#include "supla/network/html/device_info.h"
#include "supla/network/html/protocol_parameters.h"
#include "supla/network/html/wifi_parameters.h"
#include "supla/device/supla_ca_cert.h"

#include "esp_idf_web_server.h"
#include "esp_idf_wifi.h"
#include "nvs_config.h"

namespace esphome
{
    namespace supla_device
    {
        const SuplaDeviceComponent::ConfigModeOption SuplaDeviceComponent::config_mode_{
            "config_mode",
            "Config Mode",
            {
                {CONFIG_MODE_DEFAULT, "DEFAULT"},
                {CONFIG_MODE_ALWAYS_ON, "ALWAYS ON"},
            }};

        void SuplaDeviceComponent::setup()
        {
            esp_log_level_set("SUPLA", ESP_LOG_DEBUG);

            new Supla::Html::DeviceInfo(&SuplaDevice);
            new Supla::Html::WifiParameters;
            new Supla::Html::ProtocolParameters;

            new Supla::Clock;

            // Nvs based device configuration storage
            new Supla::NvsConfig;

            // Cfg mode web server
            new Supla::EspIdfWebServer;
            new Supla::EspIdfWifi;

            SuplaDevice.setSuplaCACert(suplaCACert);
            SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

#ifdef SUPLA_DEVICE_NAME
            SuplaDevice.setName(SUPLA_DEVICE_NAME);
#endif
#ifdef SUPLA_DEVICE_SW_VERSION
            SuplaDevice.setSwVersion(SUPLA_DEVICE_SW_VERSION);
#endif
#ifdef SUPLA_DEVICE_HOSTNAME_PREFIX
            SuplaDevice.setCustomHostnamePrefix(SUPLA_DEVICE_HOSTNAME_PREFIX);
#endif
            SuplaDevice.setMacLengthInHostname(3);

            SuplaDevice.begin();

            if (this->config_mode_.load_from_storage() == CONFIG_MODE_ALWAYS_ON)
                this->set_timeout(100, []()
                                  { Supla::WebServer::Instance()->start(); });
        }

        void SuplaDeviceComponent::loop()
        {
            SuplaDevice.iterate();

            auto status = SuplaDevice.getCurrentStatus();
            std::string status_str = "SuplaDevice status: " + std::to_string(status);
            switch (status)
            {
            case STATUS_OFFLINE_MODE:
            case STATUS_REGISTERED_AND_READY:
            case STATUS_SUPLA_PROTOCOL_DISABLED:
                this->status_clear_error();
                this->status_clear_warning();
                break;

            case STATUS_INITIALIZED:
            case STATUS_NETWORK_DISCONNECTED:
            case STATUS_REGISTER_IN_PROGRESS:
            case STATUS_SOFTWARE_RESET:
            case STATUS_SW_DOWNLOAD:
            case STATUS_CONFIG_MODE:
                this->status_clear_error();
                this->status_set_warning(status_str.c_str());
                break;

            case STATUS_UNKNOWN:
            case STATUS_ALREADY_INITIALIZED:
            case STATUS_MISSING_NETWORK_INTERFACE:
            case STATUS_UNKNOWN_SERVER_ADDRESS:
            case STATUS_UNKNOWN_LOCATION_ID:
            case STATUS_ALL_PROTOCOLS_DISABLED:
            case STATUS_SERVER_DISCONNECTED:
            case STATUS_ITERATE_FAIL:
            case STATUS_TEMPORARILY_UNAVAILABLE:
            case STATUS_INVALID_GUID:
            case STATUS_CHANNEL_LIMIT_EXCEEDED:
            case STATUS_PROTOCOL_VERSION_ERROR:
            case STATUS_BAD_CREDENTIALS:
            case STATUS_LOCATION_CONFLICT:
            case STATUS_CHANNEL_CONFLICT:
            case STATUS_DEVICE_IS_DISABLED:
            case STATUS_LOCATION_IS_DISABLED:
            case STATUS_DEVICE_LIMIT_EXCEEDED:
            case STATUS_REGISTRATION_DISABLED:
            case STATUS_MISSING_CREDENTIALS:
            case STATUS_INVALID_AUTHKEY:
            case STATUS_NO_LOCATION_AVAILABLE:
            case STATUS_UNKNOWN_ERROR:
            default:
                this->status_clear_warning();
                this->status_set_error(status_str.c_str());
                break;
            }
        };

    }
}