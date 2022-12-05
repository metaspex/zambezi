//
// Copyright Metaspex - 2022
// mailto:admin@metaspex.com
//

// To test (update 8080 port number to whatever was configured in the web server):
// 
// curl http://localhost:8080/service_name -d '{..JSON payload...}'
// ...JSON response...

#include "hx2a/server.hpp"

#include "hx2a/zambezi/ontology.hpp"
#include "hx2a/zambezi/payloads.hpp"
#include "hx2a/basic_service.hpp"
#include "hx2a/services/query_empty.hpp"
#include "hx2a/services/query_id.hpp"
#include "hx2a/services/reply_id.hpp"
#include "hx2a/basic_get_service.hpp"
#include "hx2a/basic_remove_service.hpp"

using namespace hx2a;

namespace zambezi {

  class product_category_create: public basic_service<"product_category_create", query_id>
  {
    reply_p call(http_request&, const session_info*, const organization_p&, const user_p&, const rfr<query_id>& q) override {
      db::connector c("hx2a");

      if (q->get_id().is_null()){
        product_category_r p = make_rfr<product_category>();
        return make_ptr<reply_id>(p->get_id());
      }

      product_category_p parent = product_category::get(q->get_id());

      if (parent == nullptr){
        return {};
      }

      product_category_r p = make_rfr<product_category>(*parent);
      return make_ptr<reply_id>(p->get_id());
    }
  } _product_category_create;

  class product_create: public basic_service<"product_create", query_id>
  {
    reply_p call(http_request&, const session_info*, const organization_p&, const user_p&, const rfr<query_id>& q) override {
      db::connector c("hx2a");

      if (q->get_id().is_null()){
        product_r p = make_rfr<product>();
        return make_ptr<reply_id>(p->get_id());
      }

      product_category_p cat = product_category::get(q->get_id());

      if (cat == nullptr){
        return {};
      }

      product_r p = make_rfr<product>(*cat);
      return make_ptr<reply_id>(p->get_id());
    }
  } _product_create;

  // Pricing policy-related services.
  
  class pricing_policy_create: public basic_service<"pricing_policy_create", pricing_policy_payload>
  {
    reply_p call(http_request&, const session_info*, const organization_p&, const user_p&, const rfr<pricing_policy_payload>& q) override {
      db::connector c("hx2a");
      return make_ptr<reply_id>(make_ptr<pricing_policy>(q->source)->get_id());
    }
  } _pricing_policy_create;

  // Vanilla service using concise template.
  basic_get_service<"pricing_policy_get", pricing_policy, "hx2a"> _pricing_policy_get;

  class pricing_policy_update: public basic_service<"pricing_policy_update", pricing_policy_with_id_payload>
  {
    reply_p call(http_request&, const session_info*, const organization_p&, const user_p&, const rfr<pricing_policy_with_id_payload>& q) override {
      db::connector c("hx2a");
      return make_ptr<reply_id>(make_ptr<pricing_policy>(q->source)->get_id());
    }
  } _pricing_policy_update;

  // Vanilla service using concise template.
  basic_remove_service<"pricing_policy_remove", pricing_policy, "hx2a"> _pricing_policy_remove;

} // End namespace zambezi.

