#pragma once

#include <fc/fixed_string.hpp>

#include <golos/protocol/authority.hpp>
#include <golos/protocol/steem_operations.hpp>

#include <golos/chain/steem_object_types.hpp>
#include <golos/chain/witness_objects.hpp>
#include <golos/chain/shared_authority.hpp>

#include <boost/multi_index/composite_key.hpp>

#include <numeric>

namespace golos { namespace chain {

using golos::protocol::authority;

class account_object
        : public object<account_object_type, account_object> {
public:
    account_object() = delete;

    template<typename Constructor, typename Allocator>
    account_object(Constructor&& c, allocator<Allocator> a) {
        c(*this);
    };

    id_type id;

    account_name_type name;
    public_key_type memo_key;
    account_name_type proxy;

    time_point_sec last_account_update;

    time_point_sec created;
    bool mined = true;
    bool owner_challenged = false;
    bool active_challenged = false;
    time_point_sec last_owner_proved = time_point_sec::min();
    time_point_sec last_active_proved = time_point_sec::min();
    account_name_type recovery_account;
    account_name_type reset_account = STEEMIT_NULL_ACCOUNT;
    time_point_sec last_account_recovery;
    uint32_t comment_count = 0;
    uint32_t lifetime_vote_count = 0;
    uint32_t post_count = 0;

    bool can_vote = true;
    uint16_t voting_power = STEEMIT_100_PERCENT;   ///< current voting power of this account, it falls after every vote
    uint16_t posts_capacity = STEEMIT_POSTS_WINDOW;
    uint16_t comments_capacity = STEEMIT_COMMENTS_WINDOW;
    uint16_t voting_capacity = STEEMIT_VOTES_WINDOW;
    time_point_sec last_vote_time; ///< used to increase the voting power of this account the longer it goes without voting.

    asset balance = asset(0, STEEM_SYMBOL);  ///< total liquid shares held by this account
    asset savings_balance = asset(0, STEEM_SYMBOL);  ///< total liquid shares held by this account

    /**
     *  SBD Deposits pay interest based upon the interest rate set by witnesses. The purpose of these
     *  fields is to track the total (time * sbd_balance) that it is held. Then at the appointed time
     *  interest can be paid using the following equation:
     *
     *  interest = interest_rate * sbd_seconds / seconds_per_year
     *
     *  Every time the sbd_balance is updated the sbd_seconds is also updated. If at least
     *  STEEMIT_MIN_COMPOUNDING_INTERVAL_SECONDS has past since sbd_last_interest_payment then
     *  interest is added to sbd_balance.
     *
     *  @defgroup sbd_data SBD Balance Data
     */
    ///@{
    asset sbd_balance = asset(0, SBD_SYMBOL); /// total sbd balance
    uint128_t sbd_seconds; ///< total sbd * how long it has been hel
    time_point_sec sbd_seconds_last_update; ///< the last time the sbd_seconds was updated
    time_point_sec sbd_last_interest_payment; ///< used to pay interest at most once per month


    asset savings_sbd_balance = asset(0, SBD_SYMBOL); /// total sbd balance
    uint128_t savings_sbd_seconds; ///< total sbd * how long it has been hel
    time_point_sec savings_sbd_seconds_last_update; ///< the last time the sbd_seconds was updated
    time_point_sec savings_sbd_last_interest_payment; ///< used to pay interest at most once per month

    uint8_t savings_withdraw_requests = 0;
    ///@}

    share_type benefaction_rewards = 0;
    share_type curation_rewards = 0;
    share_type delegation_rewards = 0;
    share_type posting_rewards = 0;

    asset vesting_shares = asset(0, VESTS_SYMBOL); ///< total vesting shares held by this account, controls its voting power
    asset delegated_vesting_shares = asset(0, VESTS_SYMBOL); ///<
    asset received_vesting_shares = asset(0, VESTS_SYMBOL); ///<

    asset vesting_withdraw_rate = asset(0, VESTS_SYMBOL); ///< at the time this is updated it can be at most vesting_shares/104
    time_point_sec next_vesting_withdrawal = fc::time_point_sec::maximum(); ///< after every withdrawal this is incremented by 1 week
    share_type withdrawn = 0; /// Track how many shares have been withdrawn
    share_type to_withdraw = 0; /// Might be able to look this up with operation history.
    uint16_t withdraw_routes = 0;

    fc::array<share_type, STEEMIT_MAX_PROXY_RECURSION_DEPTH> proxied_vsf_votes;// = std::vector<share_type>( STEEMIT_MAX_PROXY_RECURSION_DEPTH, 0 ); ///< the total VFS votes proxied to this account

    uint16_t witnesses_voted_for = 0;
    bool witness_vote_staked = false;

    time_point_sec last_comment;
    time_point_sec last_post;

    account_name_type referrer_account;
    uint16_t referrer_interest_rate = 0;
    time_point_sec referral_end_date = time_point_sec::min();
    asset referral_break_fee = asset(0, STEEM_SYMBOL);

    /// This function should be used only when the account votes for a witness directly
    share_type witness_vote_weight() const {
        return std::accumulate(proxied_vsf_votes.begin(),
                proxied_vsf_votes.end(),
                vesting_shares.amount);
    }

    share_type proxied_vsf_votes_total() const {
        return std::accumulate(proxied_vsf_votes.begin(),
                proxied_vsf_votes.end(),
                share_type());
    }

    /// vesting shares used in voting and bandwidth calculation
    asset effective_vesting_shares() const {
        return vesting_shares - delegated_vesting_shares + received_vesting_shares;
    }

    /// vesting shares, which can be used for delegation (incl. create account) and withdraw operations
    asset available_vesting_shares(bool consider_withdrawal = false) const {
        auto have = vesting_shares - delegated_vesting_shares;
        return consider_withdrawal ? have - asset(to_withdraw - withdrawn, VESTS_SYMBOL) : have;
    }
};

class account_authority_object
        : public object<account_authority_object_type, account_authority_object> {
public:
    account_authority_object() = delete;

    template<typename Constructor, typename Allocator>
    account_authority_object(Constructor &&c, allocator<Allocator> a)
            : owner(a), active(a), posting(a) {
        c(*this);
    }

    id_type id;

    account_name_type account;

    shared_authority owner;   ///< used for backup control, can set owner or active
    shared_authority active;  ///< used for all monetary operations, can set active or posting
    shared_authority posting; ///< used for voting and posting

    time_point_sec last_owner_update;
};

class account_bandwidth_object
        : public object<account_bandwidth_object_type, account_bandwidth_object> {
public:
    template<typename Constructor, typename Allocator>
    account_bandwidth_object(Constructor &&c, allocator<Allocator> a) {
        c(*this);
    }

    account_bandwidth_object() {
    }

    id_type id;

    account_name_type account;
    bandwidth_type type;
    share_type average_bandwidth;
    share_type lifetime_bandwidth;
    time_point_sec last_bandwidth_update;
};

class account_metadata_object : public object<account_metadata_object_type, account_metadata_object> {
public:
    account_metadata_object() = delete;

    template<typename Constructor, typename Allocator>
    account_metadata_object(Constructor&& c, allocator<Allocator> a) : json_metadata(a) {
        c(*this);
    }

    id_type id;
    account_name_type account;
    shared_string json_metadata;
};

class vesting_delegation_object: public object<vesting_delegation_object_type, vesting_delegation_object> {
public:
    template<typename Constructor, typename Allocator>
    vesting_delegation_object(Constructor&& c, allocator<Allocator> a) {
        c(*this);
    }

    vesting_delegation_object() {
    }

    id_type id;
    account_name_type delegator;
    account_name_type delegatee;
    asset vesting_shares;
    uint16_t interest_rate = 0;
    protocol::delegator_payout_strategy payout_strategy = protocol::to_delegator;
    time_point_sec min_delegation_time;
};

class vesting_delegation_expiration_object: public object<vesting_delegation_expiration_object_type, vesting_delegation_expiration_object> {
public:
    template<typename Constructor, typename Allocator>
    vesting_delegation_expiration_object(Constructor&& c, allocator<Allocator> a) {
        c(*this);
    }

    vesting_delegation_expiration_object() {
    }

    id_type id;
    account_name_type delegator;
    asset vesting_shares;
    time_point_sec expiration;
};

class owner_authority_history_object
        : public object<owner_authority_history_object_type, owner_authority_history_object> {
public:
    owner_authority_history_object() = delete;

    template<typename Constructor, typename Allocator>
    owner_authority_history_object(Constructor &&c, allocator<Allocator> a)
            :previous_owner_authority(shared_authority::allocator_type(a.get_segment_manager())) {
        c(*this);
    }

    id_type id;

    account_name_type account;
    shared_authority previous_owner_authority;
    time_point_sec last_valid_time;
};

class account_recovery_request_object
        : public object<account_recovery_request_object_type, account_recovery_request_object> {
public:
    account_recovery_request_object() = delete;

    template<typename Constructor, typename Allocator>
    account_recovery_request_object(Constructor &&c, allocator<Allocator> a)
            :new_owner_authority(shared_authority::allocator_type(a.get_segment_manager())) {
        c(*this);
    }

    id_type id;

    account_name_type account_to_recover;
    shared_authority new_owner_authority;
    time_point_sec expires;
};

class change_recovery_account_request_object
        : public object<change_recovery_account_request_object_type, change_recovery_account_request_object> {
public:
    template<typename Constructor, typename Allocator>
    change_recovery_account_request_object(Constructor &&c, allocator<Allocator> a) {
        c(*this);
    }

    id_type id;

    account_name_type account_to_recover;
    account_name_type recovery_account;
    time_point_sec effective_on;
};

struct by_name;
struct by_next_vesting_withdrawal;

/**
 * @ingroup object_index
 */
typedef multi_index_container<
    account_object,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<account_object, account_id_type, &account_object::id> >,
                ordered_unique<tag<by_name>,
                        member<account_object, account_name_type, &account_object::name>,
                        protocol::string_less>,
                ordered_unique<tag<by_next_vesting_withdrawal>,

                composite_key < account_object,
                    member<account_object, time_point_sec, &account_object::next_vesting_withdrawal>,
                    member<account_object, account_id_type, &account_object::id>
                > > >,
    allocator<account_object>
>
account_index;

struct by_account;
struct by_last_valid;

typedef multi_index_container<
        owner_authority_history_object,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<owner_authority_history_object, owner_authority_history_id_type, &owner_authority_history_object::id>>,
                ordered_unique<tag<by_account>,
                        composite_key < owner_authority_history_object,
                        member<owner_authority_history_object, account_name_type, &owner_authority_history_object::account>,
                        member<owner_authority_history_object, time_point_sec, &owner_authority_history_object::last_valid_time>,
                        member<owner_authority_history_object, owner_authority_history_id_type, &owner_authority_history_object::id>
                >,
                composite_key_compare <
                std::less<account_name_type>, std::less<time_point_sec>, std::less<owner_authority_history_id_type>>
>
>,
allocator<owner_authority_history_object>
>
owner_authority_history_index;


using account_metadata_index = multi_index_container<
    account_metadata_object,
    indexed_by<
        ordered_unique<
            tag<by_id>, member<account_metadata_object, account_metadata_id_type, &account_metadata_object::id>
        >,
        ordered_unique<
            tag<by_account>, member<account_metadata_object, account_name_type, &account_metadata_object::account>
        >
    >,
    allocator<account_metadata_object>
>;

struct by_last_owner_update;

typedef multi_index_container<
        account_authority_object,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<account_authority_object, account_authority_id_type, &account_authority_object::id>>,
                ordered_unique<tag<by_account>,
                        composite_key < account_authority_object,
                        member<account_authority_object, account_name_type, &account_authority_object::account>,
                        member<account_authority_object, account_authority_id_type, &account_authority_object::id>
                >,
                composite_key_compare <
                std::less<account_name_type>, std::less<account_authority_id_type>>
>,
ordered_unique<tag<by_last_owner_update>,
        composite_key < account_authority_object,
        member<account_authority_object, time_point_sec, &account_authority_object::last_owner_update>,
        member<account_authority_object, account_authority_id_type, &account_authority_object::id>
>,
composite_key_compare <std::greater<time_point_sec>, std::less<account_authority_id_type>>
>
>,
allocator<account_authority_object>
>
account_authority_index;


struct by_account_bandwidth_type;

typedef multi_index_container<
        account_bandwidth_object,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<account_bandwidth_object, account_bandwidth_id_type, &account_bandwidth_object::id>>,
                ordered_unique<tag<by_account_bandwidth_type>,
                        composite_key < account_bandwidth_object,
                        member<account_bandwidth_object, account_name_type, &account_bandwidth_object::account>,
                        member<account_bandwidth_object, bandwidth_type, &account_bandwidth_object::type>
                >
        >
>,
allocator<account_bandwidth_object>
>
account_bandwidth_index;

struct by_delegation;
struct by_received;

using vesting_delegation_index = multi_index_container<
    vesting_delegation_object,
    indexed_by<
        ordered_unique<
            tag<by_id>,
            member<vesting_delegation_object, vesting_delegation_id_type, &vesting_delegation_object::id>>,
        ordered_unique<
            tag<by_delegation>,
            composite_key<
                vesting_delegation_object,
                member<vesting_delegation_object, account_name_type, &vesting_delegation_object::delegator>,
                member<vesting_delegation_object, account_name_type, &vesting_delegation_object::delegatee>
            >,
            composite_key_compare<protocol::string_less, protocol::string_less>
        >,
        ordered_unique<
            tag<by_received>,
            composite_key<
                vesting_delegation_object,
                member<vesting_delegation_object, account_name_type, &vesting_delegation_object::delegatee>,
                member<vesting_delegation_object, account_name_type, &vesting_delegation_object::delegator>
            >,
            composite_key_compare<protocol::string_less, protocol::string_less>
        >
    >,
    allocator<vesting_delegation_object>
>;

struct by_expiration;
struct by_account_expiration;

using vesting_delegation_expiration_index = multi_index_container<
    vesting_delegation_expiration_object,
    indexed_by<
        ordered_unique<
            tag<by_id>,
            member<vesting_delegation_expiration_object, vesting_delegation_expiration_id_type, &vesting_delegation_expiration_object::id>>,
        ordered_unique<
            tag<by_expiration>,
            composite_key<
                vesting_delegation_expiration_object,
                member<vesting_delegation_expiration_object, time_point_sec, &vesting_delegation_expiration_object::expiration>,
                member<vesting_delegation_expiration_object, vesting_delegation_expiration_id_type, &vesting_delegation_expiration_object::id>
            >,
            composite_key_compare<std::less<time_point_sec>, std::less<vesting_delegation_expiration_id_type>>
        >,
        ordered_unique<
            tag<by_account_expiration>,
            composite_key<
                vesting_delegation_expiration_object,
                member<vesting_delegation_expiration_object, account_name_type, &vesting_delegation_expiration_object::delegator>,
                member<vesting_delegation_expiration_object, time_point_sec, &vesting_delegation_expiration_object::expiration>,
                member<vesting_delegation_expiration_object, vesting_delegation_expiration_id_type, &vesting_delegation_expiration_object::id>
            >,
            composite_key_compare<std::less<account_name_type>, std::less<time_point_sec>, std::less<vesting_delegation_expiration_id_type>>
        >
    >,
    allocator<vesting_delegation_expiration_object>
>;

struct by_expiration;

typedef multi_index_container<
        account_recovery_request_object,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id>>,
                ordered_unique<tag<by_account>,
                        composite_key < account_recovery_request_object,
                        member<account_recovery_request_object, account_name_type, &account_recovery_request_object::account_to_recover>,
                        member<account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id>
                >,
                composite_key_compare <
                std::less<account_name_type>, std::less<account_recovery_request_id_type>>
>,
ordered_unique<tag<by_expiration>,
        composite_key < account_recovery_request_object,
        member<account_recovery_request_object, time_point_sec, &account_recovery_request_object::expires>,
        member<account_recovery_request_object, account_recovery_request_id_type, &account_recovery_request_object::id>
>,
composite_key_compare <std::less<time_point_sec>, std::less<account_recovery_request_id_type>>
>
>,
allocator<account_recovery_request_object>
>
account_recovery_request_index;

struct by_effective_date;

typedef multi_index_container<
        change_recovery_account_request_object,
        indexed_by<
                ordered_unique<tag<by_id>,
                        member<change_recovery_account_request_object, change_recovery_account_request_id_type, &change_recovery_account_request_object::id>>,
                ordered_unique<tag<by_account>,
                        composite_key <
                        change_recovery_account_request_object,
                        member<change_recovery_account_request_object, account_name_type, &change_recovery_account_request_object::account_to_recover>,
                        member<change_recovery_account_request_object, change_recovery_account_request_id_type, &change_recovery_account_request_object::id>
                >,
                composite_key_compare <
                std::less<account_name_type>, std::less<change_recovery_account_request_id_type>>
>,
ordered_unique<tag<by_effective_date>,
        composite_key < change_recovery_account_request_object,
        member<change_recovery_account_request_object, time_point_sec, &change_recovery_account_request_object::effective_on>,
        member<change_recovery_account_request_object, change_recovery_account_request_id_type, &change_recovery_account_request_object::id>
>,
composite_key_compare <std::less<time_point_sec>, std::less<change_recovery_account_request_id_type>>
>
>,
allocator<change_recovery_account_request_object>
>
change_recovery_account_request_index;

} } // golos::chain


FC_REFLECT((golos::chain::account_object),
    (id)(name)(memo_key)(proxy)(last_account_update)
    (created)(mined)
    (owner_challenged)(active_challenged)(last_owner_proved)(last_active_proved)(recovery_account)(last_account_recovery)(reset_account)
    (comment_count)(lifetime_vote_count)(post_count)(can_vote)(voting_power)(last_vote_time)
    (balance)
    (savings_balance)
    (sbd_balance)(sbd_seconds)(sbd_seconds_last_update)(sbd_last_interest_payment)
    (savings_sbd_balance)(savings_sbd_seconds)(savings_sbd_seconds_last_update)(savings_sbd_last_interest_payment)(savings_withdraw_requests)
    (vesting_shares)(delegated_vesting_shares)(received_vesting_shares)
    (vesting_withdraw_rate)(next_vesting_withdrawal)(withdrawn)(to_withdraw)(withdraw_routes)
    (benefaction_rewards)
    (curation_rewards)
    (delegation_rewards)
    (posting_rewards)
    (proxied_vsf_votes)(witnesses_voted_for)(witness_vote_staked)
    (last_comment)(last_post)
    (referrer_account)(referrer_interest_rate)(referral_end_date)(referral_break_fee)
)
CHAINBASE_SET_INDEX_TYPE(golos::chain::account_object, golos::chain::account_index)

FC_REFLECT((golos::chain::account_authority_object),
        (id)(account)(owner)(active)(posting)(last_owner_update)
)
CHAINBASE_SET_INDEX_TYPE(golos::chain::account_authority_object, golos::chain::account_authority_index)

FC_REFLECT((golos::chain::account_bandwidth_object),
        (id)(account)(type)(average_bandwidth)(lifetime_bandwidth)(last_bandwidth_update))
CHAINBASE_SET_INDEX_TYPE(golos::chain::account_bandwidth_object, golos::chain::account_bandwidth_index)

FC_REFLECT((golos::chain::account_metadata_object), (id)(account)(json_metadata))
CHAINBASE_SET_INDEX_TYPE(golos::chain::account_metadata_object, golos::chain::account_metadata_index)

FC_REFLECT((golos::chain::vesting_delegation_object), (id)(delegator)(delegatee)(vesting_shares)(interest_rate)(min_delegation_time))
CHAINBASE_SET_INDEX_TYPE(golos::chain::vesting_delegation_object, golos::chain::vesting_delegation_index)

FC_REFLECT((golos::chain::vesting_delegation_expiration_object), (id)(delegator)(vesting_shares)(expiration))
CHAINBASE_SET_INDEX_TYPE(golos::chain::vesting_delegation_expiration_object, golos::chain::vesting_delegation_expiration_index)

FC_REFLECT((golos::chain::owner_authority_history_object),
        (id)(account)(previous_owner_authority)(last_valid_time)
)
CHAINBASE_SET_INDEX_TYPE(golos::chain::owner_authority_history_object, golos::chain::owner_authority_history_index)

FC_REFLECT((golos::chain::account_recovery_request_object),
        (id)(account_to_recover)(new_owner_authority)(expires)
)
CHAINBASE_SET_INDEX_TYPE(golos::chain::account_recovery_request_object, golos::chain::account_recovery_request_index)

FC_REFLECT((golos::chain::change_recovery_account_request_object),
        (id)(account_to_recover)(recovery_account)(effective_on)
)
CHAINBASE_SET_INDEX_TYPE(golos::chain::change_recovery_account_request_object, golos::chain::change_recovery_account_request_index)
