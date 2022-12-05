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

using namespace hx2a;

namespace zambezi {

  // Ontology functions.
  
  void physical_inventory::set_inventory(const inventory_r& i){
    // Maintaining the mutual link.
    if (_inventory != nullptr){
      _inventory->remove_physical_inventory(*this);
    }
    
    _inventory = &i;
    i->add_physical_inventory(*this);
  }
  
  void physical_inventory::disconnect(){
    _inventory->remove_physical_inventory(*this);
    _inventory = nullptr;
  }
  
  double inventory::calculate_price(unsigned int count, currency::code currency_code){
    pricing_policy_p policy = _pricing_policy;
    
    if (policy == nullptr){
      return _reference_price->get_amount();
    }
    
    // Removing the previous JavaScript prologue.
    _price.reset_prologue();
    user_p u = get_current_user();
    // Defining the prologue, with all the available variables.
    
    if (u != nullptr){
      _price <<
	js_variable("user", json::value(u->get_id()))
	;

      address_p a = u->get_address();

      if (a != nullptr){
	_price <<
	  js_variable("user_pc", json::value(a->get_postal_code())) <<
	  js_variable("user_region", json::value(a->get_region())) << 
	  js_variable("user_country", json::value(a->get_country()))
	  ;
      }
      else{
	_price <<
	  js_variable("user_pc", json::value::null()) <<
	  js_variable("user_region", json::value::null()) << 
	  js_variable("user_country", json::value::null())
	  ;
      }
    }
    else{
      _price <<
	js_variable("user", json::value::null()) << 
	js_variable("user_pc", json::value::null()) <<
	js_variable("user_region", json::value::null()) << 
	js_variable("user_country", json::value::null())
	;
    }
    
    _price <<
      js_variable("available_count", _count) <<
      js_variable("count", count) <<
      js_variable("currency", currency_code) <<
      js_variable("overdraft", _overdraft) <<
      js_variable("price", _reference_price->get_amount()) <<
      js_variable("rating", _rating)
      ;
    return _price.run()->number();
  }
  
} // End namespace zambezi.

