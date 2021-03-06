#include <golos/plugins/worker_api/worker_api_queries.hpp>
#include <golos/protocol/validate_helper.hpp>

using golos::protocol::validate_account_name;
using golos::protocol::validate_permlink;

namespace golos { namespace plugins { namespace worker_api {

    void worker_request_query::validate() const {
        GOLOS_CHECK_LIMIT_PARAM(limit, 100);

        if (!!start_author) {
            GOLOS_CHECK_PARAM_ACCOUNT(*start_author);

            GOLOS_CHECK_PARAM(start_permlink,
                GOLOS_CHECK_VALUE(!!start_permlink, "start_author without start_permlink is useless"));
        }
        if (!!start_permlink) {
            GOLOS_CHECK_PARAM(*start_permlink, validate_permlink(*start_permlink));

            GOLOS_CHECK_PARAM(start_author,
                GOLOS_CHECK_VALUE(!!start_author, "start_permlink without start_author is useless"));
        }

        for (auto& select_author : select_authors) {
            GOLOS_CHECK_PARAM_ACCOUNT(select_author);
        }
    }

} } } // golos::plugins::worker_api

