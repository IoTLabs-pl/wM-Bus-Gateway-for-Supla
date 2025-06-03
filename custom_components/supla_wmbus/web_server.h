#pragma once

#include <string>
#include <vector>

#include "supla/network/web_sender.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        struct HTMLElement
        {
            virtual void render(Supla::WebSender *sender) = 0;

        protected:
            const char *header = "<div class=form-field>"
                                 "<label style=\"width:100%;display:flex;align-items:center;\">"
                                 "<div style=\"width:250px\">";
            const char *meter_field_class = " class=meter_field";
            const char *footer = "</label>"
                                 "</div>";
        };

        struct InputElement : HTMLElement
        {
            InputElement(const std::string &label, const std::string &content, const std::string &type = "")
                : label(label), content(content), type(type) {}
            std::string label;
            std::string content;
            std::string type;

            void render(Supla::WebSender *sender) override
            {
                sender->send(this->header);
                sender->send(this->label.c_str());
                sender->send("</div>"
                             "<input");
                sender->send(this->meter_field_class);

                if (!this->type.empty())
                    sender->send((" type=" + this->type).c_str());

                sender->send(" value=\"");
                sender->send(this->content.c_str());
                sender->send("\">");
                sender->send(this->footer);
            }
        };

        struct SelectElement : HTMLElement
        {
            SelectElement(const std::string &label, const std::string &selected_option, const std::vector<std::string> &options)
                : label(label), selected_option(selected_option), options(options) {}

            std::string label;
            std::string selected_option;
            std::vector<std::string> options;

            void render(Supla::WebSender *sender) override
            {

                sender->send(this->header);
                sender->send(this->label.c_str());
                sender->send("</div>"
                             "<select");
                sender->send(this->meter_field_class);
                sender->send(">");

                if (this->selected_option.empty())
                    this->render_option(sender, "", true);

                for (auto &o : this->options)
                    this->render_option(sender, o, o == this->selected_option);

                sender->send("</select>");
                sender->send(this->footer);
            }

        protected:
            static void render_option(Supla::WebSender *sender, const std::string &option, bool selected = false)
            {
                sender->send("<option");
                if (selected)
                    sender->send(" selected");
                sender->send(">");
                sender->send(option.c_str());
                sender->send("</option>");
            }
        };
    }
}
