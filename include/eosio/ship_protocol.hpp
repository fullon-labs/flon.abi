#pragma once

#include "abi.hpp"
#include "check.hpp"
#include "crypto.hpp"
#include "fixed_bytes.hpp"
#include "float.hpp"
#include "name.hpp"
#include "opaque.hpp"
#include "stream.hpp"
#include "time.hpp"
#include "varint.hpp"

// todo: move
namespace eosio {
template <typename S>
void to_json(const input_stream& data, S& stream) {
   return to_json_hex(data.pos, data.end - data.pos, stream);
}

constexpr const char* get_type_name(const input_stream*) { return "bytes"; }
} // namespace eosio

namespace eosio { namespace ship_protocol {

   typedef __uint128_t uint128_t;

#ifdef __eosio_cdt__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Winvalid-noreturn"
   [[noreturn]] inline void report_error(const std::string& s) { eosio::check(false, s); }
#   pragma clang diagnostic pop
#else
   [[noreturn]] inline void report_error(const std::string& s) { throw std::runtime_error(s); }
#endif

   struct extension {
      uint16_t            type = {};
      eosio::input_stream data = {};
   };

   EOSIO_REFLECT(extension, type, data)

   enum class transaction_status : uint8_t {
      executed  = 0, // succeed, no error handler executed
      soft_fail = 1, // objectively failed (not executed), error handler executed
      hard_fail = 2, // objectively failed and error handler objectively failed thus no state change
      delayed   = 3, // transaction delayed/deferred/scheduled for future execution
      expired   = 4, // transaction expired and storage space refunded to user
   };

   // todo: switch to eosio::result. switch to new serializer string support.
   inline std::string to_string(transaction_status status) {
      switch (status) {
         case transaction_status::executed: return "executed";
         case transaction_status::soft_fail: return "soft_fail";
         case transaction_status::hard_fail: return "hard_fail";
         case transaction_status::delayed: return "delayed";
         case transaction_status::expired: return "expired";
      }
      report_error("unknown status: " + std::to_string((uint8_t)status));
   }

   // todo: switch to eosio::result. switch to new serializer string support.
   inline transaction_status get_transaction_status(const std::string& s) {
      if (s == "executed")
         return transaction_status::executed;
      if (s == "soft_fail")
         return transaction_status::soft_fail;
      if (s == "hard_fail")
         return transaction_status::hard_fail;
      if (s == "delayed")
         return transaction_status::delayed;
      if (s == "expired")
         return transaction_status::expired;
      report_error("unknown status: " + s);
   }

   struct get_status_request_v0 {};

   EOSIO_REFLECT(get_status_request_v0)

   struct get_status_request_v1 {};

   EOSIO_REFLECT(get_status_request_v1)

   struct block_position {
      uint32_t           block_num = {};
      eosio::checksum256 block_id  = {};
   };

   EOSIO_REFLECT(block_position, block_num, block_id)

   struct get_status_result_base {
      block_position                      head                    = {};
      block_position                      last_irreversible       = {};
      uint32_t                            trace_begin_block       = {};
      uint32_t                            trace_end_block         = {};
      uint32_t                            chain_state_begin_block = {};
      uint32_t                            chain_state_end_block   = {};
   };

   EOSIO_REFLECT(get_status_result_base, head, last_irreversible, trace_begin_block, trace_end_block,
                 chain_state_begin_block, chain_state_end_block)

   struct get_status_result_v0 : get_status_result_base {
      might_not_exist<eosio::checksum256> chain_id;
   };

   EOSIO_REFLECT(get_status_result_v0, base get_status_result_base, chain_id)

   struct get_status_result_v1 : get_status_result_base {
      eosio::checksum256                  chain_id;
      uint32_t                            finality_data_begin_block = {};
      uint32_t                            finality_data_end_block   = {};
   };

   EOSIO_REFLECT(get_status_result_v1, base get_status_result_base, chain_id, finality_data_begin_block, finality_data_end_block)

   struct get_blocks_request_v0 {
      uint32_t                    start_block_num        = {};
      uint32_t                    end_block_num          = {};
      uint32_t                    max_messages_in_flight = {};
      std::vector<block_position> have_positions         = {};
      bool                        irreversible_only      = {};
      bool                        fetch_block            = {};
      bool                        fetch_traces           = {};
      bool                        fetch_deltas           = {};
   };

   EOSIO_REFLECT(get_blocks_request_v0, start_block_num, end_block_num, max_messages_in_flight, have_positions,
                 irreversible_only, fetch_block, fetch_traces, fetch_deltas)

   struct get_blocks_request_v1 : get_blocks_request_v0 {
      bool                        fetch_finality_data    = {};
   };

   EOSIO_REFLECT(get_blocks_request_v1, base get_blocks_request_v0, fetch_finality_data)

   struct get_blocks_ack_request_v0 {
      uint32_t num_messages = {};
   };

   EOSIO_REFLECT(get_blocks_ack_request_v0, num_messages)

   using request = std::variant<get_status_request_v0, get_blocks_request_v0, get_blocks_ack_request_v0, get_blocks_request_v1, get_status_request_v1>;

   struct get_blocks_result_base {
      block_position                head              = {};
      block_position                last_irreversible = {};
      std::optional<block_position> this_block        = {};
      std::optional<block_position> prev_block        = {};
   };

   EOSIO_REFLECT(get_blocks_result_base, head, last_irreversible, this_block, prev_block)

   struct get_blocks_result_v0 : get_blocks_result_base {
      std::optional<eosio::input_stream> block  = {};
      std::optional<eosio::input_stream> traces = {};
      std::optional<eosio::input_stream> deltas = {};
   };

   EOSIO_REFLECT(get_blocks_result_v0, base get_blocks_result_base, block, traces, deltas)

   struct get_blocks_result_v1 : get_blocks_result_v0 {
      std::optional<eosio::input_stream> finality_data = {};
   };

   EOSIO_REFLECT(get_blocks_result_v1, base get_blocks_result_v0, finality_data)

   struct row_v0 {
      bool                present = {};     // false (not present), true (present, old / new)
      eosio::input_stream data    = {};
   };

   EOSIO_REFLECT(row_v0, present, data)

   struct table_delta_v0 {
      std::string         name = {};
      std::vector<row_v0> rows = {};
   };

   EOSIO_REFLECT(table_delta_v0, name, rows)

   using table_delta = std::variant<table_delta_v0>;

   struct permission_level {
      eosio::name actor      = {};
      eosio::name permission = {};
   };

   EOSIO_REFLECT(permission_level, actor, permission)

   struct action {
      eosio::name                   account       = {};
      eosio::name                   name          = {};
      std::vector<permission_level> authorization = {};
      eosio::input_stream           data          = {};
   };

   EOSIO_REFLECT(action, account, name, authorization, data)

   struct account_auth_sequence {
      eosio::name account  = {};
      uint64_t    sequence = {};
   };

   EOSIO_REFLECT(account_auth_sequence, account, sequence)
   EOSIO_COMPARE(account_auth_sequence);

   struct action_receipt_v0 {
      eosio::name                        receiver        = {};
      eosio::checksum256                 act_digest      = {};
      uint64_t                           global_sequence = {};
      uint64_t                           recv_sequence   = {};
      std::vector<account_auth_sequence> auth_sequence   = {};
      eosio::varuint32                   code_sequence   = {};
      eosio::varuint32                   abi_sequence    = {};
   };

   EOSIO_REFLECT(action_receipt_v0, receiver, act_digest, global_sequence, recv_sequence, auth_sequence, code_sequence,
                 abi_sequence)

   using action_receipt = std::variant<action_receipt_v0>;

   struct account_delta {
      eosio::name account = {};
      int64_t     delta   = {};
   };

   EOSIO_REFLECT(account_delta, account, delta)
   EOSIO_COMPARE(account_delta);

   struct ram_gas_delta {
      int64_t           ram_delta               = {};
      int64_t           gas_delta               = {};
   };
   EOSIO_REFLECT(ram_gas_delta, ram_delta, gas_delta)

   struct account_gas_trace_v0 {
      eosio::name                      account                 = {};
      uint64_t                         reserved_gas_before     = {};
      uint64_t                         reserved_gas_after      = {};
      uint64_t                         used_gas                = {};
      uint64_t                         converted_gas           = {};
      ship_protocol::ram_gas_delta     ram_gas_delta           = {};
   };
   EOSIO_REFLECT(account_gas_trace_v0, account, reserved_gas_before, reserved_gas_after, used_gas, converted_gas, ram_gas_delta)

   using account_gas_trace = std::variant<account_gas_trace_v0>;

   struct action_trace_v0 {
      eosio::varuint32              action_ordinal         = {};
      eosio::varuint32              creator_action_ordinal = {};
      std::optional<action_receipt> receipt                = {};
      eosio::name                   receiver               = {};
      action                        act                    = {};
      bool                          context_free           = {};
      int64_t                       elapsed                = {};
      std::string                   console                = {};
      std::vector<account_delta>    account_ram_deltas     = {};
      std::optional<std::string>    except                 = {};
      std::optional<uint64_t>       error_code             = {};
   };

   EOSIO_REFLECT(action_trace_v0, action_ordinal, creator_action_ordinal, receipt, receiver, act, context_free, elapsed,
                 console, account_ram_deltas, except, error_code)

   struct action_trace_v1 {
      eosio::varuint32              action_ordinal         = {};
      eosio::varuint32              creator_action_ordinal = {};
      std::optional<action_receipt> receipt                = {};
      eosio::name                   receiver               = {};
      action                        act                    = {};
      bool                          context_free           = {};
      int64_t                       elapsed                = {};
      std::string                   console                = {};
      std::vector<account_delta>    account_ram_deltas     = {};
      std::optional<std::string>    except                 = {};
      std::optional<uint64_t>       error_code             = {};
      eosio::input_stream           return_value           = {};
   };

   EOSIO_REFLECT(action_trace_v1, action_ordinal, creator_action_ordinal, receipt, receiver, act, context_free, elapsed,
                 console, account_ram_deltas, except, error_code, return_value)

   using action_trace = std::variant<action_trace_v0, action_trace_v1>;

   struct transaction_res_usage_v0 {
      eosio::name                   payer       = {};
      uint64_t                      net_usage   = {};
      uint64_t                      net_gas     = {};
      uint64_t                      cpu_usage   = {};
      uint64_t                      cpu_gas     = {};
   };

   EOSIO_REFLECT(transaction_res_usage_v0, payer, net_usage, net_gas, cpu_usage, cpu_gas)

   using transaction_res_usage = std::variant<transaction_res_usage_v0>;

   struct partial_transaction_v0 {
      eosio::time_point_sec            expiration             = {};
      uint16_t                         ref_block_num          = {};
      uint32_t                         ref_block_prefix       = {};
      eosio::varuint32                 max_net_usage_words    = {};
      uint8_t                          max_cpu_usage_ms       = {};
      eosio::varuint32                 delay_sec              = {};
      std::vector<extension>           transaction_extensions = {};
      std::vector<eosio::signature>    signatures             = {};
      std::vector<eosio::input_stream> context_free_data      = {};
   };

   EOSIO_REFLECT(partial_transaction_v0, expiration, ref_block_num, ref_block_prefix, max_net_usage_words,
                 max_cpu_usage_ms, delay_sec, transaction_extensions, signatures, context_free_data)

   using partial_transaction = std::variant<partial_transaction_v0>;

   struct recurse_transaction_trace;

   struct transaction_trace_v0 {
      eosio::checksum256                     id                = {};
      transaction_status                     status            = {};
      uint32_t                               cpu_usage_us      = {};
      eosio::varuint32                       net_usage_words   = {};
      int64_t                                elapsed           = {};
      transaction_res_usage                  res_usage         = {};
      bool                                   scheduled         = {};
      std::vector<action_trace>              action_traces     = {};
      std::optional<account_delta>           trx_ram_delta     = {};
      std::vector<account_gas_trace>         gas_traces        = {};
      std::optional<std::string>             except            = {};
      std::optional<uint64_t>                error_code        = {};
      // semantically, this should be std::optional<transaction_trace>;
      // optional serializes as bool[,transaction_trace]
      // vector serializes as size[,transaction_trace..] but vector will only ever have 0 or 1 transaction trace
      // This assumes that bool and size for false/true serializes to same as size 0/1
      std::vector<recurse_transaction_trace> failed_dtrx_trace;
      std::optional<partial_transaction>     partial           = {};
   };

   EOSIO_REFLECT(transaction_trace_v0, id, status, cpu_usage_us, net_usage_words, elapsed, res_usage, scheduled,
                 action_traces, trx_ram_delta, gas_traces, except, error_code, failed_dtrx_trace, partial)

   using transaction_trace = std::variant<transaction_trace_v0>;

   struct recurse_transaction_trace {
      transaction_trace recurse = {};
   };

   struct producer_key {
      eosio::name       producer_name     = {};
      eosio::public_key block_signing_key = {};
   };

   EOSIO_REFLECT(producer_key, producer_name, block_signing_key)

   struct producer_schedule {
      uint32_t                  version   = {};
      std::vector<producer_key> producers = {};
   };

   EOSIO_REFLECT(producer_schedule, version, producers)

   struct transaction_receipt_header {
      transaction_status status          = {};
      uint32_t           cpu_usage_us    = {};
      eosio::varuint32   net_usage_words = {};
   };

   EOSIO_REFLECT(transaction_receipt_header, status, cpu_usage_us, net_usage_words)

   struct packed_transaction {
      std::vector<eosio::signature> signatures               = {};
      uint8_t                       compression              = {};
      eosio::input_stream           packed_context_free_data = {};
      eosio::input_stream           packed_trx               = {};
   };

   EOSIO_REFLECT(packed_transaction, signatures, compression, packed_context_free_data, packed_trx)

   using transaction_variant_v0 = std::variant<eosio::checksum256, packed_transaction>;

   struct transaction_receipt_v0 : transaction_receipt_header {
      transaction_variant_v0 trx = {};
   };

   EOSIO_REFLECT(transaction_receipt_v0, base transaction_receipt_header, trx)

   using transaction_variant = std::variant<eosio::checksum256, packed_transaction>;

   struct transaction_receipt : transaction_receipt_header {
      transaction_variant trx = {};
   };

   EOSIO_REFLECT(transaction_receipt, base transaction_receipt_header, trx)

   struct block_header {
      eosio::block_timestamp           timestamp{};
      eosio::name                      producer          = {};
      uint16_t                         confirmed         = {};
      eosio::checksum256               previous          = {};
      eosio::checksum256               transaction_mroot = {};
      eosio::checksum256               action_mroot      = {};
      uint32_t                         schedule_version  = {};
      std::optional<producer_schedule> new_producers     = {};
      std::vector<extension>           header_extensions = {};
   };

   EOSIO_REFLECT(block_header, timestamp, producer, confirmed, previous, transaction_mroot, action_mroot,
                 schedule_version, new_producers, header_extensions)

   struct signed_block_header : block_header {
      eosio::signature producer_signature = {};
   };

   EOSIO_REFLECT(signed_block_header, base block_header, producer_signature)

   struct signed_block : signed_block_header {
      std::vector<transaction_receipt_v0> transactions     = {};
      std::vector<extension>              block_extensions = {};
   };

   EOSIO_REFLECT(signed_block, base signed_block_header, transactions, block_extensions)

   using result = std::variant<get_status_result_v0, get_blocks_result_v0, get_blocks_result_v1, get_status_result_v1>;

   struct transaction_header {
      eosio::time_point_sec expiration          = {};
      uint16_t              ref_block_num       = {};
      uint32_t              ref_block_prefix    = {};
      eosio::varuint32      max_net_usage_words = {};
      uint8_t               max_cpu_usage_ms    = {};
      eosio::varuint32      delay_sec           = {};
   };

   EOSIO_REFLECT(transaction_header, expiration, ref_block_num, ref_block_prefix, max_net_usage_words, max_cpu_usage_ms,
                 delay_sec)

   struct transaction : transaction_header {
      std::vector<action>    context_free_actions   = {};
      std::vector<action>    actions                = {};
      std::vector<extension> transaction_extensions = {};
   };

   EOSIO_REFLECT(transaction, base transaction_header, context_free_actions, actions, transaction_extensions)

   struct code_id {
      uint8_t            vm_type    = {};
      uint8_t            vm_version = {};
      eosio::checksum256 code_hash  = {};
   };

   EOSIO_REFLECT(code_id, vm_type, vm_version, code_hash)

   struct account_v0 {
      eosio::name            name          = {};
      eosio::block_timestamp creation_date = {};
      eosio::input_stream    abi           = {};
   };

   EOSIO_REFLECT(account_v0, name, creation_date, abi)

   using account = std::variant<account_v0>;

   struct account_metadata_v0 {
      eosio::name            name             = {};
      bool                   privileged       = {};
      eosio::time_point      last_code_update = {};
      std::optional<code_id> code             = {};
   };

   EOSIO_REFLECT(account_metadata_v0, name, privileged, last_code_update, code)

   using account_metadata = std::variant<account_metadata_v0>;

   struct code_v0 {
      uint8_t             vm_type    = {};
      uint8_t             vm_version = {};
      eosio::checksum256  code_hash  = {};
      eosio::input_stream code       = {};
   };

   EOSIO_REFLECT(code_v0, vm_type, vm_version, code_hash, code)

   using code = std::variant<code_v0>;

   struct contract_table_v0 {
      eosio::name code  = {};
      eosio::name scope = {};
      eosio::name table = {};
      eosio::name payer = {};
   };

   EOSIO_REFLECT(contract_table_v0, code, scope, table, payer)

   using contract_table = std::variant<contract_table_v0>;

   struct contract_row_v0 {
      eosio::name         code        = {};
      eosio::name         scope       = {};
      eosio::name         table       = {};
      uint64_t            primary_key = {};
      eosio::name         payer       = {};
      eosio::input_stream value       = {};
   };

   EOSIO_REFLECT(contract_row_v0, code, scope, table, primary_key, payer, value)

   using contract_row = std::variant<contract_row_v0>;

   struct contract_index64_v0 {
      eosio::name code          = {};
      eosio::name scope         = {};
      eosio::name table         = {};
      uint64_t    primary_key   = {};
      eosio::name payer         = {};
      uint64_t    secondary_key = {};
   };

   EOSIO_REFLECT(contract_index64_v0, code, scope, table, primary_key, payer, secondary_key)

   using contract_index64 = std::variant<contract_index64_v0>;

   struct contract_index128_v0 {
      eosio::name code          = {};
      eosio::name scope         = {};
      eosio::name table         = {};
      uint64_t    primary_key   = {};
      eosio::name payer         = {};
      uint128_t   secondary_key = {};
   };

   EOSIO_REFLECT(contract_index128_v0, code, scope, table, primary_key, payer, secondary_key)

   using contract_index128 = std::variant<contract_index128_v0>;

   struct contract_index256_v0 {
      eosio::name        code          = {};
      eosio::name        scope         = {};
      eosio::name        table         = {};
      uint64_t           primary_key   = {};
      eosio::name        payer         = {};
      eosio::checksum256 secondary_key = {};
   };

   EOSIO_REFLECT(contract_index256_v0, code, scope, table, primary_key, payer, secondary_key)

   using contract_index256 = std::variant<contract_index256_v0>;

   struct contract_index_double_v0 {
      eosio::name code          = {};
      eosio::name scope         = {};
      eosio::name table         = {};
      uint64_t    primary_key   = {};
      eosio::name payer         = {};
      double      secondary_key = {};
   };

   EOSIO_REFLECT(contract_index_double_v0, code, scope, table, primary_key, payer, secondary_key)

   using contract_index_double = std::variant<contract_index_double_v0>;

   struct contract_index_long_double_v0 {
      eosio::name     code          = {};
      eosio::name     scope         = {};
      eosio::name     table         = {};
      uint64_t        primary_key   = {};
      eosio::name     payer         = {};
      eosio::float128 secondary_key = {};
   };

   EOSIO_REFLECT(contract_index_long_double_v0, code, scope, table, primary_key, payer, secondary_key)

   using contract_index_long_double = std::variant<contract_index_long_double_v0>;

   struct key_weight {
      eosio::public_key key    = {};
      uint16_t          weight = {};
   };

   EOSIO_REFLECT(key_weight, key, weight)

   struct block_signing_authority_v0 {
      uint32_t                threshold = {};
      std::vector<key_weight> keys      = {};
   };

   EOSIO_REFLECT(block_signing_authority_v0, threshold, keys)

   using block_signing_authority = std::variant<block_signing_authority_v0>;

   struct producer_authority {
      eosio::name             producer_name = {};
      block_signing_authority authority     = {};
   };

   EOSIO_REFLECT(producer_authority, producer_name, authority)

   struct producer_authority_schedule {
      uint32_t                        version   = {};
      std::vector<producer_authority> producers = {};
   };

   EOSIO_REFLECT(producer_authority_schedule, version, producers)

   struct chain_config_v0 {
      uint64_t max_block_net_usage                 = {};
      uint32_t target_block_net_usage_pct          = {};
      uint32_t max_transaction_net_usage           = {};
      uint32_t base_per_transaction_net_usage      = {};
      uint32_t net_usage_leeway                    = {};
      uint32_t context_free_discount_net_usage_num = {};
      uint32_t context_free_discount_net_usage_den = {};
      uint32_t max_block_cpu_usage                 = {};
      uint32_t target_block_cpu_usage_pct          = {};
      uint32_t max_transaction_cpu_usage           = {};
      uint32_t min_transaction_cpu_usage           = {};
      uint32_t max_transaction_lifetime            = {};
      uint32_t deferred_trx_expiration_window      = {};
      uint32_t max_transaction_delay               = {};
      uint32_t max_inline_action_size              = {};
      uint16_t max_inline_action_depth             = {};
      uint16_t max_authority_depth                 = {};
      uint64_t max_total_ram_usage                 = {};
      uint32_t gas_per_cpu_ms                      = {};
      uint32_t gas_per_net_kb                      = {};
      uint32_t gas_per_ram_kb                      = {};
   };

   EOSIO_REFLECT(chain_config_v0, max_block_net_usage, target_block_net_usage_pct, max_transaction_net_usage,
                 base_per_transaction_net_usage, net_usage_leeway, context_free_discount_net_usage_num,
                 context_free_discount_net_usage_den, max_block_cpu_usage, target_block_cpu_usage_pct,
                 max_transaction_cpu_usage, min_transaction_cpu_usage, max_transaction_lifetime,
                 deferred_trx_expiration_window, max_transaction_delay, max_inline_action_size, max_inline_action_depth,
                 max_authority_depth, max_total_ram_usage, gas_per_cpu_ms, gas_per_net_kb, gas_per_ram_kb)

   struct chain_config_v1 {
      uint64_t max_block_net_usage                 = {};
      uint32_t target_block_net_usage_pct          = {};
      uint32_t max_transaction_net_usage           = {};
      uint32_t base_per_transaction_net_usage      = {};
      uint32_t net_usage_leeway                    = {};
      uint32_t context_free_discount_net_usage_num = {};
      uint32_t context_free_discount_net_usage_den = {};
      uint32_t max_block_cpu_usage                 = {};
      uint32_t target_block_cpu_usage_pct          = {};
      uint32_t max_transaction_cpu_usage           = {};
      uint32_t min_transaction_cpu_usage           = {};
      uint32_t max_transaction_lifetime            = {};
      uint32_t deferred_trx_expiration_window      = {};
      uint32_t max_transaction_delay               = {};
      uint32_t max_inline_action_size              = {};
      uint16_t max_inline_action_depth             = {};
      uint16_t max_authority_depth                 = {};
      uint64_t max_total_ram_usage                 = {};
      uint32_t gas_per_cpu_ms                      = {};
      uint32_t gas_per_net_kb                      = {};
      uint32_t gas_per_ram_kb                      = {};
      uint32_t max_action_return_value_size        = {};
   };

   EOSIO_REFLECT(chain_config_v1, max_block_net_usage, target_block_net_usage_pct, max_transaction_net_usage,
               base_per_transaction_net_usage, net_usage_leeway, context_free_discount_net_usage_num,
               context_free_discount_net_usage_den, max_block_cpu_usage, target_block_cpu_usage_pct,
               max_transaction_cpu_usage, min_transaction_cpu_usage, max_transaction_lifetime,
               deferred_trx_expiration_window, max_transaction_delay, max_inline_action_size, max_inline_action_depth,
               max_authority_depth, max_total_ram_usage, gas_per_cpu_ms, gas_per_net_kb, gas_per_ram_kb,
               max_action_return_value_size)

   using chain_config = std::variant<chain_config_v0, chain_config_v1>;

   struct wasm_config_v0 {
      uint32_t max_mutable_global_bytes = {};
      uint32_t max_table_elements       = {};
      uint32_t max_section_elements     = {};
      uint32_t max_linear_memory_init   = {};
      uint32_t max_func_local_bytes     = {};
      uint32_t max_nested_structures    = {};
      uint32_t max_symbol_bytes         = {};
      uint32_t max_module_bytes         = {};
      uint32_t max_code_bytes           = {};
      uint32_t max_pages                = {};
      uint32_t max_call_depth           = {};
   };

   EOSIO_REFLECT(wasm_config_v0, max_mutable_global_bytes, max_table_elements, max_section_elements,
                 max_linear_memory_init, max_func_local_bytes, max_nested_structures, max_symbol_bytes,
                 max_module_bytes, max_code_bytes, max_pages, max_call_depth)

   using wasm_config = std::variant<wasm_config_v0>;

   struct global_property_v0 {
      std::optional<uint32_t> proposed_schedule_block_num = {};
      producer_schedule       proposed_schedule           = {};
      chain_config            configuration               = {};
   };

   EOSIO_REFLECT(global_property_v0, proposed_schedule_block_num, proposed_schedule, configuration)

   struct global_property_v1 {
      std::optional<uint32_t>      proposed_schedule_block_num = {};
      producer_authority_schedule  proposed_schedule           = {};
      chain_config                 configuration               = {};
      eosio::checksum256           chain_id                    = {};
      might_not_exist<wasm_config> wasm_configuration;
   };

   EOSIO_REFLECT(global_property_v1, proposed_schedule_block_num, proposed_schedule, configuration, chain_id, wasm_configuration)

   using global_property = std::variant<global_property_v0, global_property_v1>;

   struct generated_transaction_v0 {
      eosio::name         sender     = {};
      uint128_t           sender_id  = {};
      eosio::name         payer      = {};
      eosio::checksum256  trx_id     = {};
      eosio::input_stream packed_trx = {};
   };

   EOSIO_REFLECT(generated_transaction_v0, sender, sender_id, payer, trx_id, packed_trx)

   using generated_transaction = std::variant<generated_transaction_v0>;

   struct activated_protocol_feature_v0 {
      eosio::checksum256 feature_digest       = {};
      uint32_t           activation_block_num = {};
   };

   EOSIO_REFLECT(activated_protocol_feature_v0, feature_digest, activation_block_num)

   using activated_protocol_feature = std::variant<activated_protocol_feature_v0>;

   struct protocol_state_v0 {
      std::vector<activated_protocol_feature> activated_protocol_features = {};
   };

   EOSIO_REFLECT(protocol_state_v0, activated_protocol_features)

   using protocol_state = std::variant<protocol_state_v0>;

   struct permission_level_weight {
      permission_level permission = {};
      uint16_t         weight     = {};
   };

   EOSIO_REFLECT(permission_level_weight, permission, weight)

   struct wait_weight {
      uint32_t wait_sec = {};
      uint16_t weight   = {};
   };

   EOSIO_REFLECT(wait_weight, wait_sec, weight)

   struct authority {
      uint32_t                             threshold = {};
      std::vector<key_weight>              keys      = {};
      std::vector<permission_level_weight> accounts  = {};
      std::vector<wait_weight>             waits     = {};
   };

   EOSIO_REFLECT(authority, threshold, keys, accounts, waits)

   struct permission_v0 {
      eosio::name       owner        = {};
      eosio::name       name         = {};
      eosio::name       parent       = {};
      eosio::time_point last_updated = {};
      authority         auth         = {};
   };

   EOSIO_REFLECT(permission_v0, owner, name, parent, last_updated, auth)

   using permission = std::variant<permission_v0>;

   struct permission_link_v0 {
      eosio::name account             = {};
      eosio::name code                = {};
      eosio::name message_type        = {};
      eosio::name required_permission = {};
   };

   EOSIO_REFLECT(permission_link_v0, account, code, message_type, required_permission)

   using permission_link = std::variant<permission_link_v0>;

   struct resource_limits_v0 {
      eosio::name owner          = {};
      uint64_t    gas            = {};
      bool        is_unlimited   = {};
   };

   EOSIO_REFLECT(resource_limits_v0, owner, ram_bytes, gas, is_unlimited)

   using resource_limits = std::variant<resource_limits_v0>;

   struct usage_accumulator_v0 {
      uint32_t last_ordinal = {};
      uint64_t value_ex     = {};
      uint64_t consumed     = {};
   };

   EOSIO_REFLECT(usage_accumulator_v0, last_ordinal, value_ex, consumed)

   using usage_accumulator = std::variant<usage_accumulator_v0>;

   struct resource_usage_v0 {
      eosio::name       owner     = {};
      uint64_t          net_usage = {};
      uint64_t          cpu_usage = {};
      uint64_t          ram_usage = {};
   };

   EOSIO_REFLECT(resource_usage_v0, owner, net_usage, cpu_usage, ram_usage)

   using resource_usage = std::variant<resource_usage_v0>;

   struct resource_limits_state_v0 {
      usage_accumulator average_block_net_usage = {};
      usage_accumulator average_block_cpu_usage = {};
      uint64_t          total_net_usage        = {};
      uint64_t          total_cpu_usage        = {};
      uint64_t          total_ram_usage         = {};
      uint64_t          virtual_net_limit       = {};
      uint64_t          virtual_cpu_limit       = {};
   };

   EOSIO_REFLECT(resource_limits_state_v0, average_block_net_usage, average_block_cpu_usage, total_net_usage,
                 total_cpu_usage, total_ram_usage, virtual_net_limit, virtual_cpu_limit)

   using resource_limits_state = std::variant<resource_limits_state_v0>;

   struct resource_limits_ratio_v0 {
      uint64_t numerator   = {};
      uint64_t denominator = {};
   };

   EOSIO_REFLECT(resource_limits_ratio_v0, numerator, denominator)

   using resource_limits_ratio = std::variant<resource_limits_ratio_v0>;

   struct elastic_limit_parameters_v0 {
      uint64_t              target         = {};
      uint64_t              max            = {};
      uint32_t              periods        = {};
      uint32_t              max_multiplier = {};
      resource_limits_ratio contract_rate  = {};
      resource_limits_ratio expand_rate    = {};
   };

   EOSIO_REFLECT(elastic_limit_parameters_v0, target, max, periods, max_multiplier, contract_rate, expand_rate)

   using elastic_limit_parameters = std::variant<elastic_limit_parameters_v0>;

   struct resource_limits_config_v0 {
      elastic_limit_parameters cpu_limit_parameters             = {};
      elastic_limit_parameters net_limit_parameters             = {};
      uint32_t                 account_cpu_usage_average_window = {};
      uint32_t                 account_net_usage_average_window = {};
      uint32_t                 gas_per_cpu_ms                   = {};
      uint32_t                 gas_per_net_kb                   = {};
      uint32_t                 gas_per_ram_kb                   = {};
   };

   EOSIO_REFLECT(resource_limits_config_v0, cpu_limit_parameters, net_limit_parameters,
                 account_cpu_usage_average_window, account_net_usage_average_window,
                 gas_per_cpu_ms, gas_per_net_kb, gas_per_ram_kb)

   using resource_limits_config = std::variant<resource_limits_config_v0>;

   struct finalizer_authority {
      std::string       description = {};
      uint64_t          weight      = {};
      std::vector<char> public_key  = {};
   };

   EOSIO_REFLECT(finalizer_authority, description, weight, public_key)

   struct finalizer_authority_with_string_key {
      std::string   description = {};
      uint64_t      weight      = {};
      std::string   public_key  = {};
   };

   EOSIO_REFLECT(finalizer_authority_with_string_key, description, weight, public_key)

   struct finalizer_policy_with_string_key {
      uint32_t                                         generation = {};
      uint64_t                                         threshold  = {};
      std::vector<finalizer_authority_with_string_key> finalizers = {};
   };

   EOSIO_REFLECT(finalizer_policy_with_string_key, generation, threshold, finalizers)

   struct finality_data {
      uint32_t                        major_version                      = {};
      uint32_t                        minor_version                      = {};
      uint32_t                        active_finalizer_policy_generation = {};
      eosio::checksum256              action_mroot                       = {};
      eosio::checksum256              reversible_blocks_mroot            = {};
      uint32_t                        latest_qc_claim_block_num          = {};
      eosio::checksum256              latest_qc_claim_finality_digest    = {};
      eosio::block_timestamp          latest_qc_claim_timestamp          = {};
      eosio::checksum256              base_digest                        = {};
      std::optional<finalizer_policy_with_string_key> pending_finalizer_policy = {};
      uint32_t                        last_pending_finalizer_policy_generation = {};
   };

   EOSIO_REFLECT(finality_data, major_version, minor_version, active_finalizer_policy_generation,
                 action_mroot, reversible_blocks_mroot, latest_qc_claim_block_num, latest_qc_claim_finality_digest,
                 latest_qc_claim_timestamp, base_digest, pending_finalizer_policy, last_pending_finalizer_policy_generation)

   struct protocol_feature_activation_extension {
      std::vector<eosio::checksum256> protocol_features = {};
   };

   EOSIO_REFLECT(protocol_feature_activation_extension, protocol_features)

   struct producer_schedule_change_extension : producer_authority_schedule {};

   EOSIO_REFLECT(producer_schedule_change_extension, base producer_authority_schedule);

   struct qc_claim {
      uint32_t  block_num    = {};
      bool      is_strong_qc = {};
   };

   EOSIO_REFLECT(qc_claim, block_num, is_strong_qc)

   struct insert_finalizer_policy_index_pair {
      uint16_t            index = {};
      finalizer_authority value = {};
   };

   EOSIO_REFLECT(insert_finalizer_policy_index_pair, index, value)

   struct finalizer_policy_diff {
      uint32_t                                        generation     = {};
      uint64_t                                        threshold      = {};
      std::vector<uint16_t>                           remove_indexes = {};
      std::vector<insert_finalizer_policy_index_pair> insert_indexes = {};
   };

   EOSIO_REFLECT(finalizer_policy_diff, generation, threshold, remove_indexes, insert_indexes)

   struct insert_proposer_policy_index_pair {
      uint16_t           index = {};
      producer_authority value = {};
   };

   EOSIO_REFLECT(insert_proposer_policy_index_pair, index, value)

   struct proposer_policy_diff {
      uint32_t                                       version        = {};
      eosio::block_timestamp                         proposal_time  = {};
      std::vector<uint16_t>                          remove_indexes = {};
      std::vector<insert_proposer_policy_index_pair> insert_indexes = {};
   };

   EOSIO_REFLECT(proposer_policy_diff, version, proposal_time, remove_indexes, insert_indexes)

   struct finality_extension {
      ship_protocol::qc_claim              qc_claim                  = {};
      std::optional<finalizer_policy_diff> new_finalizer_policy_diff = {};
      std::optional<proposer_policy_diff>  new_proposer_policy_diff  = {};
   };

   EOSIO_REFLECT(finality_extension, qc_claim, new_finalizer_policy_diff, new_proposer_policy_diff)

   using block_header_extension = std::variant<protocol_feature_activation_extension, producer_schedule_change_extension, finality_extension>;

}} // namespace eosio::ship_protocol

namespace eosio {

   template <typename S>
   void to_json(const ship_protocol::transaction_status& status, S& stream) {
      // todo: switch to new serializer string support.
      return eosio::to_json((uint8_t)status, stream);
   }

   template <typename S>
   void from_json(ship_protocol::transaction_status& status, S& stream) {
      uint8_t v;
      eosio::from_json(v, stream);
      status = (ship_protocol::transaction_status)v;
   }

   template <typename S>
   void to_bin(const ship_protocol::recurse_transaction_trace& obj, S& stream) {
      return to_bin(obj.recurse, stream);
   }

   template <typename S>
   void from_bin(ship_protocol::recurse_transaction_trace& obj, S& stream) {
      return from_bin(obj.recurse, stream);
   }

   template <typename S>
   void to_json(const ship_protocol::recurse_transaction_trace& obj, S& stream) {
      return to_json(obj.recurse, stream);
   }

   template <typename S>
   void to_json(const std::vector<ship_protocol::recurse_transaction_trace>& obj, S& stream) {
      if (!obj.empty()) {
         to_json(obj[0], stream);
      } else {
         stream.write("null", 4);
      }
   }

   template <typename S>
   void from_json(std::vector<ship_protocol::recurse_transaction_trace>& result, S& stream) {
      if(stream.get_null_pred()) {
         result.clear();
      } else {
         result.emplace_back();
         from_json(result[0], stream);
      }
   }

}
