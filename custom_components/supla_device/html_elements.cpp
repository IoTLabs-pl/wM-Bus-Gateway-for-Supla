#include "html_elements.h"

namespace esphome
{
    namespace supla_device
    {

        // HTMLElement::Attribute
        HTMLElement::Attribute::Attribute(const char *name) : Attribute(name, "") {}
        HTMLElement::Attribute::Attribute(const char *name, const char *value) : name(name), value(value) {}
        void HTMLElement::Attribute::render(Supla::WebSender *sender) const
        {
            sender->send(" ");
            sender->send(name);
            if (!value[0])
                return;

            sender->send("=\"");
            sender->send(value);
            sender->send("\"");
        }

        // HTMLElement::Tag
        HTMLElement::Tag::Tag(const char *name) : name(name) {}
        void HTMLElement::Tag::render_open(Supla::WebSender *sender, const std::list<Attribute> &attributes) const
        {
            sender->send("<");
            sender->send(name);
            for (const auto &attr : attributes)
                attr.render(sender);
            sender->send(">");
        }
        void HTMLElement::Tag::render_close(Supla::WebSender *sender) const
        {
            sender->send("</");
            sender->send(name);
            sender->send(">");
        }

        // HTMLElement
        HTMLElement::HTMLElement(const char *tag,
                                 const char *content,
                                 std::list<HTMLElement> &&children,
                                 std::list<Attribute> &&attributes)
            : tag(tag), content(content), children(std::move(children)), attributes(std::move(attributes)) {}

        void HTMLElement::render(Supla::WebSender *sender) const
        {
            this->tag.render_open(sender, this->attributes);

            sender->send(content);

            for (const auto &child : children)
                child.render(sender);

            this->tag.render_close(sender);
        }

        // LabelElement
        LabelElement::LabelElement(const char *label)
            : HTMLElement("label",
                          label,
                          {})
        {
        }

        // DivElement
        DivElement::DivElement(std::list<HTMLElement> &&children, std::list<Attribute> &&attributes)
            : HTMLElement("div", "", std::move(children), std::move(attributes)) {}

        // InputElement
        InputElement::InputElement(const char *content, const char *type)
            : InputElement(content, {}, type)
        {
        }
        InputElement::InputElement(const char *content, std::list<Attribute> &&extra_attributes, const char *type)
            : HTMLElement("input",
                          "",
                          {},
                          {
                              {"type", type},
                              {"value", content},
                          })
        {
            // Merge extra_attributes into attributes
            for (auto &&attr : extra_attributes)
                this->attributes.push_back(std::move(attr));
        }

        // SelectElement::Option

        SelectElement::Option::Option(const char *name, bool selected)
            : HTMLElement("option",
                          name,
                          {})
        {
            if (selected)
                this->attributes.emplace_back("selected");
        }

        // SelectElement
        SelectElement::SelectElement(std::list<Option> &&options)
            : HTMLElement("select",
                          "",
                          {
                              std::make_move_iterator(options.begin()),
                              std::make_move_iterator(options.end()),
                          })
        {
        }

        // create_form_field
        DivElement create_form_field(const HTMLElement child, const char *label, bool indexable)
        {
            auto div = DivElement{{child}, {{"class", "form-field"}}};
            if (label[0])
            {
                auto label_element = LabelElement{label};
                if (indexable)
                    label_element.attributes.emplace_back("data-indexable");
                div.children.push_front(label_element);
            }

            return div;
        }

    } // namespace supla_device
} // namespace esphome
