#pragma once

#include <string>
#include <list>
#include <utility>
#include "supla/network/web_sender.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {

        struct HTMLElement
        {
        public:
            struct Attribute
            {
                Attribute(const char *name);
                Attribute(const char *name, const char *value);

                const char *name;
                const char *value;

                void render(Supla::WebSender *sender) const;
            };

        private:
            struct Tag
            {
                explicit Tag(const char *name);

                const char *name;

                void render_open(Supla::WebSender *sender, const std::list<Attribute> &attributes) const;
                void render_close(Supla::WebSender *sender) const;
            };

        public:
            const Tag tag;
            const char *content;
            std::list<HTMLElement> children;
            std::list<Attribute> attributes;

            HTMLElement(const char *tag,
                        const char *content,
                        std::list<HTMLElement> &&children = {},
                        std::list<Attribute> &&attributes = {});

            void render(Supla::WebSender *sender) const;
        };

        struct LabelElement : HTMLElement
        {
            LabelElement(const char *label);
        };

        struct DivElement : HTMLElement
        {
            DivElement(std::list<HTMLElement> &&children = {},
                       std::list<Attribute> &&attributes = {});
        };

        struct InputElement : HTMLElement
        {
            InputElement(const char *content, const char *type = "text");
        };

        struct SelectElement : HTMLElement
        {
            struct Option : HTMLElement
            {
                Option(const char *name, bool selected = false);
            };

            SelectElement(std::list<Option> &&options);
        };

        DivElement create_form_field(const HTMLElement child, const char *label = "", bool indexable = false);

    } // namespace supla_wmbus_reader
} // namespace esphome
