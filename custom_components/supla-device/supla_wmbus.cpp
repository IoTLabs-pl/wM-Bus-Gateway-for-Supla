#include "supla_wmbus.h"

#include "esphome/core/log.h"
#include "esphome/components/wmbus_common/meters.h"

#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/device/supla_ca_cert.h>

#include <esp_idf_web_server.h>
#include <esp_idf_wifi.h>
#include <esp_log.h>
#include <nvs_config.h>

#include "meter.h"
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

      for (auto &meter : this->get_stored_meters())
        if (!meter->get_id().empty())
        {
          meter->set_radio(this->radio);
          for (auto &sensor : meter->sensors)
            this->display_manager->add_sensor(sensor);
          this->meters.push_back(meter);
        }
      
      this->display_manager->sync();
      
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

    std::vector<Meter *> Component::get_stored_meters()
    {
      uint8_t i = 0;
      std::vector<Meter *> meters;

      char key[SUPLA_CONFIG_MAX_KEY_SIZE];
      char value[100];

      auto cfg = Supla::Storage::ConfigInstance();

      while (cfg)
      {
        Supla::Config::generateKey(key, i, "meter");
        if (!cfg->getString(key, value, sizeof(value) - 1))
          break;
        meters.push_back(Meter::deserialize(value));
        i += 1;
      }

      return meters;
    }

    void Frontend::send(Supla::WebSender *sender)
    {
      sender->send("<div class=\"box\">");
      sender->send(frontend_script);
      sender->send("<h3>wM-Bus Meters</h3>");

      auto meters = this->parent_->get_stored_meters();
      if (meters.size() == 0)
        meters.push_back(new Meter{});

      for (auto &meter : meters)
      {
        meter->render_html_config(sender);
        delete meter;
      }

      ESP_LOGI("WM", "after loop");

      sender->send(
          "<button type=button onclick=add_meter() >Add Meter</button>");
      sender->send("</div>");
    }

    bool Frontend::handleResponse(const char *key, const char *value)
    {
      ESP_LOGD("WM", "Got %s->%s", key, value);

      if (strncmp(key, "meter_", sizeof("meter_") - 1) == 0)
      {
        key += sizeof("meter_") - 1;

        auto id = atoi(key);

        ESP_LOGI("WM", "Processing meter %d", id);

        this->post_meters.push_back(Meter::deserialize(value));
        return true;
      }
      return false;
    }

    void Frontend::onProcessingEnd()
    {
      auto cfg = Supla::Storage::ConfigInstance();
      if (cfg)
      {
        uint8_t i;
        char key[SUPLA_CONFIG_MAX_KEY_SIZE];

        for (i = 0; i < this->post_meters.size(); i++)
        {
          Supla::Config::generateKey(key, i, "meter");
          cfg->setString(key, this->post_meters[i]->serialize().c_str());
        }

        while (true)
        {
          Supla::Config::generateKey(key, i, "meter");
          if (!cfg->eraseKey(key))
            break;
        }
      }

      for (auto &meter : this->post_meters)
        delete meter;
      this->post_meters = {};
    }
  };

}
