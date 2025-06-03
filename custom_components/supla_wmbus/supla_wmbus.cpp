#include "supla_wmbus.h"

#include "esphome/core/log.h"
#include "esphome/components/wmbus_common/meters.h"

#include "supla/network/html/device_info.h"
#include "supla/network/html/protocol_parameters.h"
#include "supla/network/html/wifi_parameters.h"
#include "supla/device/supla_ca_cert.h"

#include "esp_idf_web_server.h"
#include "esp_idf_wifi.h"
#include "nvs_config.h"

#include "resources.h"

namespace esphome
{
  namespace supla_wmbus_reader
  {

    void Component::setup()
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

      uint8_t i;
      static char name[] = ESPHOME_PROJECT_NAME;
      for (i = 0; i < strlen(name); i++)
        if (name[i] == '.')
          break;

      name[i] = ' ';

      SuplaDevice.setName(name);
      SuplaDevice.setSwVersion("v" ESPHOME_PROJECT_VERSION);
      SuplaDevice.setCustomHostnamePrefix(name + i + 1);
      SuplaDevice.setMacLengthInHostname(3);
      SuplaDevice.begin();

      this->config_.pull();
      auto meters = Meter::create_from_config(this->config_);

      ESP_LOGE("WM", "Found %d meters", meters.size());

      for (auto meter : meters)
          meter->attach_hardware(this->radio, this->display_manager);
      
    }

    void Component::loop()
    {
      SuplaDevice.iterate();
      Supla::WebServer::Instance()->start();

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

    void Frontend::send(Supla::WebSender *sender)
    {
      sender->send("<div class=\"box\">");
      sender->send(frontend_script);
      sender->send("<h3>wM-Bus Meters</h3>");

      this->parent_->config_.pull();
      this->parent_->config_.render_html(sender); 

      sender->send(
          "<button type=button onclick=add_meter() >Add Meter</button>");
      sender->send("</div>");
    }

    bool Frontend::handleResponse(const char *key, const char *value)
    {
      ESP_LOGD("WM", "POST: Got %s->%s", key, value);

      if (strncmp(key, "meter_", sizeof("meter_") - 1) == 0)
      {
        this->parent_->config_.add_entry(value);
        return true;
      }
      return false;
    }

    void Frontend::onProcessingEnd()
    {
      this->parent_->config_.push();
    }
  };

}
