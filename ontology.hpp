//
// Copyright Metaspex - 2022
// mailto:admin@metaspex.com
//

#ifndef HX2A_ZAMBEZI_ONTOLOGY_HPP
#define HX2A_ZAMBEZI_ONTOLOGY_HPP

#include "hx2a/element.hpp"
#include "hx2a/root.hpp"
#include "hx2a/slot.hpp"
#include "hx2a/slot_js.hpp"
#include "hx2a/key_attribute.hpp"
#include "hx2a/own.hpp"
#include "hx2a/link.hpp"
#include "hx2a/weak_link.hpp"
#include "hx2a/weak_link_list.hpp"
#include "hx2a/add_id.hpp"
#include "hx2a/components/organization.hpp"
#include "hx2a/components/user.hpp"
#include "hx2a/components/money.hpp"
#include "hx2a/zambezi/cart.hpp"

using namespace hx2a;

namespace zambezi {

  // Ontology.
  
  class product_category;
  using product_category_p = ptr<product_category>;
  using product_category_r = rfr<product_category>;

  class product;
  using product_p = ptr<product>;
  using product_r = rfr<product>;

  class inventory_snapshot;
  using inventory_snapshot_p = ptr<inventory_snapshot>;
  using inventory_snapshot_r = rfr<inventory_snapshot>;

  class persona;
  using persona_p = ptr<persona>;
  using persona_r = rfr<persona>;

  class physical_inventory;
  using physical_inventory_p = ptr<physical_inventory>;
  using physical_inventory_r = rfr<physical_inventory>;

  class inventoried_product;
  using inventoried_product_p = ptr<inventoried_product>;
  using inventoried_product_r = rfr<inventoried_product>;

  class pricing_policy;
  using pricing_policy_p = ptr<pricing_policy>;
  using pricing_policy_r = rfr<pricing_policy>;

  class inventory;
  using inventory_p = ptr<inventory>;
  using inventory_r = rfr<inventory>;

  using warehouse = organization;
  using warehouse_p = ptr<warehouse>;
  using warehouse_r = rfr<warehouse>;

  using count_type = uint64_t;
  
  class product_category: public root<>
  {
    HX2A_ROOT(product_category, "ecom:product_category", 1, root);
    
  public:

    // Reserved constructor.
    product_category(reserved_t, const doc_id& id):
      root(reserved, id),
      _parent(*this)
    {
    }

    product_category():
      root(standard),
      _parent(*this)
    {
    }

    product_category(product_category_r pc):
      root(standard),
      _parent(*this, &pc)
    {
    }

    product_category_p get_parent() const { return _parent; }

  private:
    link<product_category, "parent"> _parent;
  };
  
  class product: public root<>
  {
    HX2A_ROOT(product, "ecom:product", 1, root);
    
  public:

    // Reserved constructor.
    product(reserved_t, const doc_id& id):
      root(reserved, id),
      _category(*this)
    {
    }

    product():
      root(standard),
      _category(*this)
    {
    }
    
    product(product_category_r category):
      root(standard),
      _category(*this, &category)
    {
    }

    product_category_r get_category() const { return *_category; }

  private:
    link<product_category, "category"> _category;
  };

  // To be extended.
  class inventory_snapshot: public element<>
  {
  public:
    HX2A_ELEMENT(inventory_snapshot, "ecom:invsnap", element);
  
    inventory_snapshot(reserved_t):
      element(reserved)
    {
    }

    inventory_snapshot():
      element(standard)
    {
    }
  };

  class persona: public root<>
  {
    HX2A_ROOT(persona, "ecom:persona", 1, root);
    
  public:

    static ptr<inventory_snapshot> take_snapshot(const rfr<inventory>&){ return {}; }
    // Using Metaspex's Foundation Ontology zambezi cart type. 
    using mycart = hx2a::zambezi::cart_with_snapshots<
      inventory, // Items in the cart.
      "ecom:cart", // Type tag for the cart type.
      "ecom:cart_line_base", // Type tag for the cart line base type.
      "ecom:cart_line", // Type tag for the cart line type.
      "ecom:cart_folder", // Type tag for the cart folder type.
      inventory_snapshot, // Snapshot type.
      take_snapshot // Function to create a snapshot from an item.
      >;
    using mycart_p = ptr<mycart>;
    using mycart_r = rfr<mycart>;
    
    // Reserved constructor.
    persona(reserved_t, const doc_id& id):
      root(reserved, id),
      _user(*this),
      _cart(*this)
    {
    }

    persona(user_r u):
      root(standard),
      _user(*this, &u),
      _cart(*this)
    {
    }

    user_r get_user() const { return *_user; }
    mycart_r get_cart() const { return *_cart; }
    
  private:
    link<user, "user"> _user;
    own<mycart, "cart"> _cart;
  };

  // Do not use unpublish if the inventory link is non null, it does not maintain the mutual link with the logical inventory.
  // Use disconnect or remove below.
  class physical_inventory: public root<>
  {
    HX2A_ROOT(physical_inventory, "ecom:phyinv", 1, root);
    
  public:

    // Reserved constructor.
    physical_inventory(reserved_t, const doc_id& id):
      root(reserved, id),
      _inventory(*this),
      _warehouse(*this),
      _count(*this)
    {
    }

    // Do not call this constructor, use instead the function adding a physical
    // inventory to an inventory.
    physical_inventory(const inventory_r& inv, const warehouse_r& ware):
      root(standard),
      _inventory(*this, &inv),
      _warehouse(*this, &ware),
      _count(*this, 0)
    {
    }

    inventory_r get_inventory() const { return *_inventory; }
    
    void set_inventory(const inventory_r& i);
    
    warehouse_r get_warehouse() const { return *_warehouse; }
    count_type get_count() const { return _count; }

    void disconnect();

    void remove(){
      disconnect();
      unpublish();
    }
    
  private:
    link<inventory, "i"> _inventory;
    link<warehouse, "w"> _warehouse;
    // Active because it is used to calculate the semantic attribute counting the number of items in a logical inventory.
    slot<count_type, "c", active> _count;
  };

  // Just a placeholder to reserve the possibility to enrich product.
  class inventoried_product: public root<>
  {
    HX2A_ROOT(inventoried_product, "ecom:invprod", 1, root);
    
  public:

    // Reserved constructor.
    inventoried_product(reserved_t, const doc_id& id):
      root(reserved, id),
      _product(*this)
    {
    }

    inventoried_product(const product_r& prod):
      root(standard),
      _product(*this, &prod)
    {
    }

    product_r get_product() const { return *_product; }

  private:
    link<product, "p"> _product;
  };

  class pricing_policy: public root<>
  {
    HX2A_ROOT(pricing_policy, "ecom:ppolicy", 1, root);
    
  public:

    // Reserved constructor.
    pricing_policy(reserved_t, const doc_id& id):
      root(reserved, id),
      _source(*this)
    {
    }

    // The variables which can be used are:
    //
    // - available_count (a number containing the items count in the inventory).
    // - count (a number containing the items count requested).
    // - currency (a number corresponding to the ISO 4127 currency code).
    // - overdraft (a boolean, 0 if false).
    // - price (a number containing the reference price).
    // - rating (a floating-point value between 1 and 5).
    // - user (a string containing the document identifier of the current user requesting the price calculation).
    // - user_country (a number corresponding to the ISO 3166 country code).
    // - user_pc (a string containing the postal code of the user's address, if any).
    // - user_region (A string containing the region of the user's address, if any. In the USA it corresponds to the State).
    //
    // When the corresponding variable is not available (no current user or no address for instance, the null JSON value is
    // assigned).
    //
    // Meant for ad-hoc calculation (e.g. insertion of the inventory in a searchable kd-tree).
    // Easy to transform into a (key) semantic attribute. Just make the input variables and the link to
    // the pricing policy active and associate the calculation function to the semantic attribute.
    // Then the calculated price will be "cached" by the semantic attribute value, and not recalculated
    // at every request. Just be careful about the one-to-many relationship between inventories and the
    // pricing policy. In case too many inventories have the same pricing policy, updating the pricing
    // policy should be done offline and not through a semantic attribute.
    
    // Examples of policies
    //
    // Just returning the reference price, ignoring the currency (equivalent to no policy whatsoever).
    //
    /*
    "price"
    */

    // Sale.
    //
    // Half the reference price if the count is less than 10. Ignoring the currency.
    /*
    "count < 10 ? price / 2 : price"
    */

    // Overcharge.
    //
    // Double the reference price if the count is less than 10. Ignoring the currency.
    /*
    "count < 10 ? 2 * price : price"
    */

    // Buying the market.
    //
    // Reference price until rating 2, then linearly increased up to 50% if rating is 5/5.
    /*
    "rating < 2 ? price : price * 1.5 * (rating - 2) / 3"
    */

    // Favoring one currency.
    //
    // 15% discount if the currency is USD (ref. ISO 4217 currency codes, USD is 840).
    /*
    "currency == 840 ? price * 0.85 : price"
    */

    // Favoring several currencies.
    //
    // 15% discount if NORAM, 10% if Euro.
    //
    /*
    "switch(currency){" // ISO 4217.
    "case 124:" // CAD.
    "case 840:" // USD.
      "price * 0.85;"
      "break;"
    "case 978:" // EUR.
      "price * 0.9;"
     "break;"
    "default:"
      "price;"
    "}"
    */
    
    pricing_policy(string source):
      root(standard),
      _source(*this, source)
    {
    }

    string get_source() const { return _source; }
    void set_source(string source) { _source = source; }

  private:
    slot<string, "s"> _source;
  };
  
  class inventory: public root<>
  {
    HX2A_ROOT(inventory, "ecom:inventory", 1, root);
    
  public:

    // Reserved constructor.
    inventory(reserved_t, const doc_id& id):
      root(reserved, id),
      _product(*this),
      _count(*this),
      _overdraft(*this),
      _rating(*this),
      _reference_price(*this),
      _pricing_policy(*this),
      _price(*this),
      _physical_inventories(*this)
    {
    }

    inventory(const inventoried_product_r& iprod, currency::code cc, double reference_price):
      root(standard),
      _product(*this, &iprod),
      _count(*this),
      _overdraft(*this, false),
      _rating(*this, 0),
      _reference_price(*this, make_ptr<money>(reference_price, cc)),
      _pricing_policy(*this),
      _price(*this),
      _physical_inventories(*this)
    {
    }

    count_type get_count() /* cannot be const */ {
      // As we have a weak link list, there can be null relationships. This call will remove them.
      _physical_inventories.skim();
      return _count;
    }

    // When true, sales are allowed even when the count is null. The sale is marked as "back order".
    bool get_overdraft() const { return _overdraft; }
    void set_overdraft(bool flag = true){ _overdraft = flag; }

    void add_physical_inventory(const physical_inventory_r& pi){
      // Establishing the mutual link.
      _physical_inventories.push_front(pi);
      pi->set_inventory(*this);
    }

    // The remove is done by the physical inventory itself to ensure that the mutual link is properly maintained.
    // Do not use the function below.
    void remove_physical_inventory(const physical_inventory_r& pi){
      _physical_inventories.remove(pi);
    }

    currency::code get_reference_currency() const { return _reference_price->get_currency(); }
    
    money_r get_reference_price() const { return *_reference_price; }
    void set_reference_price(const money_r& p){ _reference_price = &p; }

    // No pricing policy means returning the reference price.
    pricing_policy_p get_pricing_policy() const { return _pricing_policy; }
    
    void set_pricing_policy(const pricing_policy_p& pp){
      _pricing_policy = pp;

      if (pp != nullptr){
	// Assigning the policy source.
	_price = _pricing_policy->get_source();
      }
    }

    double calculate_price(unsigned int requested_count, currency::code currency_code);

  private:
    
    link<inventoried_product, "p"> _product;

    count_type calculateCount(const count_type& /* ignored */) const {
      count_type count = 0;
      
      std::for_each(_physical_inventories.cbegin(),
		    _physical_inventories.cend(),
		    [&count](const auto& pi)
		    {
		      // As we have a weak link list, there can be null relationships.
		      if (pi != nullptr){
			count += pi->get_count();
		      }
		    });
      
      return count;
    }
    key_attribute<count_type, &inventory::calculateCount, "c"> _count;
    
    slot<bool, "o"> _overdraft;
    // A rating derived from all ratings. Not necessarily an average, can be some secret sauce like most
    // eCommerce sites (weighted - with user characteristics/demographics, date of review, etc., and/or
    // removing outliers, etc.).
    slot<float, "r"> _rating;
    own<money, "rp"> _reference_price;
    // Weak link, the inventory still exists if the pricing policy is removed.
    weak_link<pricing_policy, "pp"> _pricing_policy;
    slot_js<"p"> _price;
    // Weak link list and not a regular strong link list so that when an inventory is loaded the
    // physical inventories are loaded only if necessary.
    weak_link_list<physical_inventory, "i", infinite /* max size */, active> _physical_inventories;
  };
  
} // End namespace zambezi.

#endif
