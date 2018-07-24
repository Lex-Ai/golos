#pragma once

#include <golos/plugins/private_message/private_message_operations.hpp>
#include <golos/chain/evaluator.hpp>

namespace golos { namespace chain {
    class database;
} } // golos::chain

namespace golos { namespace plugins { namespace private_message {

    class private_message_plugin;

    using golos::chain::evaluator_impl;
    using golos::chain::database;

    class private_message_evaluator:
        public evaluator_impl<private_message_evaluator, private_message_plugin_operation>
    {
    public:
        using operation_type = private_message_operation;

        private_message_evaluator(database& db, private_message_plugin* plugin)
            : evaluator_impl<private_message_evaluator, private_message_plugin_operation>(db),
              plugin_(plugin)
        {}

        void do_apply(const private_message_operation& o);

        private_message_plugin* plugin_;
    };

    class private_settings_evaluator:
        public evaluator_impl<private_settings_evaluator, private_message_plugin_operation>
    {
    public:
        using operation_type = private_settings_operation;

        private_settings_evaluator(database& db, private_message_plugin* plugin)
            : evaluator_impl<private_settings_evaluator, private_message_plugin_operation>(db),
              plugin_(plugin)
        {}

        void do_apply(const private_settings_operation& o);

        private_message_plugin* plugin_;
    };

    class private_contact_evaluator:
        public evaluator_impl<private_contact_evaluator, private_message_plugin_operation>
    {
    public:
        using operation_type = private_contact_operation;

        private_contact_evaluator(database& db, private_message_plugin* plugin)
            : evaluator_impl<private_contact_evaluator, private_message_plugin_operation>(db),
              plugin_(plugin)
        {}

        void do_apply(const private_contact_operation& o);

        private_message_plugin* plugin_;
    };

} } }
