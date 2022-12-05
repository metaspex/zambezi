//
// Copyright Metaspex - 2022
// mailto:admin@metaspex.com
//

#ifndef HX2A_ZAMBEZI_PAYLOADS_HPP
#define HX2A_ZAMBEZI_PAYLOADS_HPP

#include "hx2a/element.hpp"
#include "hx2a/slot.hpp"

namespace zambezi {
  
  class pricing_policy_payload;
  using pricing_policy_payload_p = ptr<pricing_policy_payload>;
  using pricing_policy_payload_r = rfr<pricing_policy_payload>;
  
  class pricing_policy_payload: public element<>
  {
  public:
    HX2A_ELEMENT(pricing_policy_payload, "ecom:pppld", element);
  
    pricing_policy_payload(reserved_t):
      element(reserved),
      source(*this)
    {
    }

    pricing_policy_payload(const pricing_policy_r& pp):
      element(standard),
      source(*this, pp->get_source())
    {
    }

    slot<string, "source"> source;
  };

  using pricing_policy_with_id_payload = add_id<pricing_policy, "ecom::ppwidpld", pricing_policy_payload>;
  using pricing_policy_with_id_payload_p = ptr<pricing_policy_with_id_payload>;
  using pricing_policy_with_id_payload_r = rfr<pricing_policy_with_id_payload>;

}

#endif

