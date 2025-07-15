#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "esphome/core/log.h"

#include "supla/sensor/electricity_meter.h"
#include "supla/sensor/virtual_impulse_counter.h"

namespace esphome
{
    namespace supla_wmbus_reader
    {
        struct BindMetadata
        {
            const char *name;
            bool indexable;
        };

        template <typename SuplaT>
        struct MetaBind : public BindMetadata
        {
            std::function<void(std::shared_ptr<SuplaT>, uint8_t, float)> setter;
        };

        struct Bind : public BindMetadata
        {
            template <typename T>
            Bind(MetaBind<T> meta, std::shared_ptr<T> obj)
                : BindMetadata{meta.name, meta.indexable},
                  setter(std::bind(meta.setter, obj, std::placeholders::_1, std::placeholders::_2))
            {
            }

            std::function<void(uint8_t, float)> setter;
        };

        class WrappedSuplaBase
        {
        public:
            virtual std::vector<Bind> prepare_bindings() = 0;
            virtual const std::vector<const BindMetadata *> bind_metadata() const = 0;
        };

        template <typename SuplaT>
        class WrappedSupla : public WrappedSuplaBase
        {
            std::vector<Bind> prepare_bindings() override
            {
                std::vector<Bind> bindings;
                bindings.reserve(this->meta_binds.size());
                this->supla_object = std::shared_ptr<SuplaT>{new SuplaT{}};

                for (const MetaBind<SuplaT> &meta_bind : meta_binds)
                    bindings.emplace_back(meta_bind, this->supla_object);
                return bindings;
            }

            const std::vector<const BindMetadata *> bind_metadata() const override
            {
                std::vector<const BindMetadata *> result;
                result.reserve(this->meta_binds.size());
                for (const auto &item : this->meta_binds)
                    result.push_back(&item);
                return result;
            }

        protected:
            std::shared_ptr<SuplaT> supla_object = nullptr;
            static const std::vector<MetaBind<SuplaT>> meta_binds;
        };
        using EM = Supla::Sensor::ElectricityMeter;
        using VIC = Supla::Sensor::VirtualImpulseCounter;
        using ElectricityMeter = WrappedSupla<EM>;
        using FlowMeter = WrappedSupla<VIC>;
    } // namespace supla_wmbus_reader

}
