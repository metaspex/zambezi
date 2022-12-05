//
// Copyright Metaspex - 2022
// mailto:admin@metaspex.com
//

#ifndef HX2A_CART_HPP
#define HX2A_CART_HPP

#include <algorithm>

#include "hx2a/link.hpp"
#include "hx2a/slot.hpp"
#include "hx2a/own.hpp"
#include "hx2a/own_list.hpp"

namespace hx2a::zambezi {

  constexpr tag_t DefaultItemTag = {"i"};
  constexpr tag_t DefaultCountTag = {"c"};
  
  template <
    typename Item, // Item type to put in the cart.
    tag_t Tag, // Type tag for the cart line type.
    tag_t ItemTag = DefaultItemTag, // Tag for the item link.
    tag_t CountTag = DefaultCountTag // Tag for the count.
    >
  class cart_line: public element<>
  {
    HX2A_ELEMENT(cart_line, Tag, element);

  public:

    using ItemP = ptr<Item>;
    using ItemR = rfr<Item>;
    
    cart_line(reserved_t):
      hx2a_base(reserved),
      _item(*this),
      _count(*this, 0)
    {
    }

    cart_line(const ItemR& item, uint32_t count = 1):
      hx2a_base(standard),
      _item(*this, &item),
      _count(*this, count)
    {
      if (!count){
	throw count_is_null();
      }
    }

    ItemR item() const {
      HX2A_ASSERT(_count);
      return *_item;
    }

    uint32_t count() const {
      HX2A_ASSERT(_count);
      return _count;
    }

    void increment_count() {
      HX2A_ASSERT(_count);
      _count = _count + 1;
    }

    void decrement_count() {
      HX2A_ASSERT(_count);
      
      if (_count.get() == 1){
	throw count_is_null();
      }

      _count = _count - 1;
    }

    void update_count(uint32_t count){
      HX2A_ASSERT(_count);
      
      if (!count){
	throw count_is_null();
      }

      _count = count;
    }
      
  private:

    link<Item, ItemTag> _item;
    slot<uint32_t, CountTag> _count;
  };

  constexpr tag_t DefaultSnapshotTag = {"s"};
  
  template <
    typename Item, // Item type to put in the cart.
    tag_t Tag, // Type tag for the cart line type.
    tag_t CartLineBaseTypeTag, // Type tag for the base class of the cart line with snapshot (bearing only the item link and the item count).
    typename Snapshot, // Snapshot type owned by the cart line. Element or anchor.
    ptr<Snapshot> (*TakeSnapshot)(const rfr<Item>&), // Function creating a snapshot from an item.
    tag_t ItemTag = DefaultItemTag, // Tag for the item link.
    tag_t CountTag = DefaultCountTag, // Tag for the count.
    tag_t SnapshotTag = DefaultSnapshotTag // Tag for the snapshot ownership.
    >
  class cart_line_with_snapshot: public cart_line<Item, CartLineBaseTypeTag, ItemTag, CountTag>
  {
  public:
    
    using SnapshotP = ptr<Snapshot>;
    using SnapshotR = rfr<Snapshot>;

    using cart_base_line_type = cart_line<Item, CartLineBaseTypeTag, ItemTag, CountTag>;

    using ItemP = typename cart_base_line_type::ItemP;
    using ItemR = typename cart_base_line_type::ItemR;

    HX2A_ELEMENT(cart_line_with_snapshot, Tag, cart_base_line_type);

    cart_line_with_snapshot(reserved_t):
      hx2a_base(reserved),
      _snapshot(*this)
    {
    }

    cart_line_with_snapshot(const ItemR& item, uint32_t count = 1):
      hx2a_base(item, count),
      _snapshot(*this, TakeSnapshot(item))
    {
    }

    SnapshotP get_snapshot() const { return _snapshot; }
      
  private:

    own<Snapshot, SnapshotTag> _snapshot;
  };

  constexpr tag_t DefaultCartLinesTag = {"n"};
  
  constexpr tag_t DefaultFolderNameTag = {"l"};
  
  template <typename Item, tag_t Tag, typename CartLine, tag_t NameTag = DefaultFolderNameTag, tag_t LinesTag = DefaultCartLinesTag>
  class folder: public element<>
  {
    HX2A_ELEMENT(folder, Tag, element);

  public:

    using ItemP = ptr<Item>;
    using ItemR = rfr<Item>;

    using line = CartLine;
    using line_p = ptr<line>;
    using line_r = rfr<line>;

    using lines = own_list<line, LinesTag>;
    using lines_iterator = typename lines::iterator;
    using lines_const_iterator = typename lines::const_iterator;
    using lines_reverse_iterator = typename lines::reverse_iterator;
    using lines_const_reverse_iterator = typename lines::const_reverse_iterator;

    folder(reserved_t):
      hx2a_base(reserved),
      _name(*this),
      _lines(*this)
    {
    }

    folder(std::string_view name):
      hx2a_base(standard),
      _name(*this, name),
      _lines(*this)
    {
    }

    const string& get_name() const { return _name; }
    
    size_t lines_size() const { return _lines.size(); }

    // Total number of items.
    size_t items_count() const {
      size_t count = 0;
      std::for_each(_lines.cbegin(), _lines.cend(), [&](const line_p& f){count += f->count();});
      return count;
    }

    void add_item(const ItemR& item){
      line_p f = find_item(item);

      if (!f){
	_lines.push_front(make_rfr<line>(item));
      }
      else{
	f->increment_count();
      }
    }

    bool remove_item(const ItemR& item){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& l){return l->item() == item;});

      if (fi == e){
	return false;
      }

      line_r l = **fi;
      
      if (l.get().count() > 1){
	l.get().decrement_count();
      }
      else{
	// Last item, we remove the line.
	_lines.erase(fi);
      }
      
      return true;
    }

    bool remove_item_all(const ItemR& item){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& i){return i->item() == item;});

      if (fi != e){
	_lines.erase(fi);
	return true;
      }

      return false;
    }

    void update_item_count(const ItemR& item, uint32_t count){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& i){return i->item() == item;});

      if (fi == e){
	if (!count){
	  // Nothing to do.
	  return;
	}
	
	_lines.push_front(make_rfr<line>(item, count));
	return;
      }

      if (!count){
	_lines.erase(fi);
	return;
      }
      
      (*fi)->update_count(count);
    }

    line_p find_item(const ItemR& item){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& i){return i->item() == item;});

      if (fi != e){
	return *fi;
      }

      return {};
    }

    lines_iterator lines_begin() { return _lines.begin(); }
    lines_iterator lines_end() { return _lines.end(); }
    lines_const_iterator lines_cbegin() const { return _lines.cbegin(); }
    lines_const_iterator lines_cend() const { return _lines.cend(); }

    lines_reverse_iterator lines_rbegin() { return _lines.rbegin(); }
    lines_reverse_iterator lines_rend() { return _lines.rend(); }
    lines_const_reverse_iterator lines_crbegin() const { return _lines.crbegin(); }
    lines_const_reverse_iterator lines_crend() const { return _lines.crend(); }

  private:

    slot<string, NameTag> _name;
    lines _lines;
  };
  
  constexpr tag_t DefaultFoldersTag = {"f"};

  struct TopLevel{};
  struct IncludingFolders{};

  // A cart contains lines and folders containing lines. Typically folders are meant for items that have to be
  // all available or none.
  // Only one level of folder is authorized.
  
  template <
    typename Item, // Item type to put in the cart.
    tag_t Tag, // Type tag for the cart type.
    tag_t LineTypeTag, // Type tag for the line type in the cart.
    tag_t FolderTypeTag, // Type tag for the folder type in the cart.
    tag_t LineItemTag = DefaultItemTag, // Tag for the line link to the item in the line type.
    tag_t LineCountTag = DefaultCountTag, // Tag for the line count in the line type.
    typename CartLine = cart_line<Item, LineTypeTag, LineItemTag, LineCountTag>, // Cart line type.
    tag_t LinesTag = DefaultCartLinesTag, // Tag for the lines in the cart.
    tag_t FoldersTag = DefaultFoldersTag, // Tag for the folders in the cart.
    tag_t FolderNameTag = DefaultFolderNameTag // Tag for the folder name in the folder type.
    >
  class gen_cart: public element<>
  {
    HX2A_ELEMENT(gen_cart, Tag, element);

  public:

    using ItemP = ptr<Item>;
    using ItemR = rfr<Item>;

    using line = CartLine;
    using line_p = ptr<line>;
    using line_r = rfr<line>;

    using lines = own_list<line, LinesTag>;
    using lines_iterator = typename lines::iterator;
    using lines_const_iterator = typename lines::const_iterator;
    using lines_reverse_iterator = typename lines::reverse_iterator;
    using lines_const_reverse_iterator = typename lines::const_reverse_iterator;

    using folder_type = folder<Item, FolderTypeTag, line, FolderNameTag, LinesTag>;
    using folder_p = ptr<folder_type>;
    using folder_r = rfr<folder_type>;
    
    using folders = own_list<folder_type, FoldersTag>;
    using folders_iterator = typename folders::iterator;
    using folders_const_iterator = typename folders::const_iterator;
    using folders_reverse_iterator = typename folders::reverse_iterator;
    using folders_const_reverse_iterator = typename folders::const_reverse_iterator;

    gen_cart(reserved_t):
      hx2a_base(reserved),
      _lines(*this),
      _folders(*this)
    {
    }

    gen_cart():
      hx2a_base(standard),
      _lines(*this),
      _folders(*this)
    {
    }

    size_t lines_size() const { return _lines.size(); }

    size_t folders_size() const { return _folders.size(); }

    // Total number of items.
    template <typename Thoroughness = IncludingFolders>
    size_t items_count() const {
      size_t count = 0;
      
      std::for_each(_lines.cbegin(), _lines.cend(), [&](const line_p& f){count += f->count();});

      if constexpr (std::is_same<Thoroughness, IncludingFolders>::value){
        std::for_each(_folders.cbegin(), _folders.cend(), [&](const folder_p& f){count += f->items_count();});
      }
      
      return count;
    }

    // Adds at top level.
    void add_item(const ItemR& item){
      line_p f = find_item(item);

      if (!f){
	_lines.push_front(make_rfr<line>(item));
      }
      else{
	f->increment_count();
      }
    }

    // Removes at top level.
    bool remove_item(const ItemR& item){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& l){return l->item() == item;});

      if (fi == e){
	return false;
      }

      line_r l = **fi;
      
      if (l.get().count() > 1){
	l.get().decrement_count();
      }
      else{
	// Last item, we remove the line.
	_lines.erase(fi);
      }
      
      return true;
    }

    // Removes only at top level.
    bool remove_item_all(const ItemR& item){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& i){return i->item() == item;});

      if (fi != e){
	_lines.erase(fi);
	return true;
      }

      return false;
    }

    // Operates only at top level.
    void update_item_count(const ItemR& item, uint32_t count){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& i){return i->item() == item;});

      if (fi == e){
	if (!count){
	  // Nothing to do.
	  return;
	}
	
	_lines.push_front(make_rfr<line>(item, count));
	return;
      }

      if (!count){
	_lines.erase(fi);
	return;
      }
      
      (*fi)->update_count(count);
    }

    // Operates only at top level.
    line_p find_item(const ItemR& item){
      auto e = _lines.cend();
      auto fi = std::find_if(_lines.cbegin(), e, [&](const line_p& i){return i->item() == item;});

      if (fi != e){
	return *fi;
      }

      return {};
    }

    // Will return true if no homonymous folder has been found and the folder was created and added.
    // Will return false otherwise.
    bool add_folder(std::string_view name){
      folder_p f = find_folder(name);

      if (!f){
	_lines.push_front(make_rfr<folder_type>(name));
	return true;
      }

      return false;
    }

    bool remove_folder(std::string_view name){
      auto e = _folders.cend();
      auto fi = std::find_if(_folders.cbegin(), e, [&](const folder_p& l){return l->get_name() == name;});

      if (fi == e){
	return false;
      }

      _lines.erase(fi);
      return true;
    }

    folder_p find_folder(std::string_view name){
      auto e = _folders.cend();
      auto fi = std::find_if(_folders.cbegin(), e, [&](const folder_p& i){return i->get_name() == name;});

      if (fi != e){
	return *fi;
      }

      return {};
    }

    lines_iterator lines_begin() { return _lines.begin(); }
    lines_iterator lines_end() { return _lines.end(); }
    lines_const_iterator lines_cbegin() const { return _lines.cbegin(); }
    lines_const_iterator lines_cend() const { return _lines.cend(); }

    lines_reverse_iterator lines_rbegin() { return _lines.rbegin(); }
    lines_reverse_iterator lines_rend() { return _lines.rend(); }
    lines_const_reverse_iterator lines_crbegin() const { return _lines.crbegin(); }
    lines_const_reverse_iterator lines_crend() const { return _lines.crend(); }

    folders_iterator folders_begin() { return _folders.begin(); }
    folders_iterator folders_end() { return _folders.end(); }
    folders_const_iterator folders_cbegin() const { return _folders.cbegin(); }
    folders_const_iterator folders_cend() const { return _folders.cend(); }

    folders_reverse_iterator folders_rbegin() { return _folders.rbegin(); }
    folders_reverse_iterator folders_rend() { return _folders.rend(); }
    folders_const_reverse_iterator folders_crbegin() const { return _folders.crbegin(); }
    folders_const_reverse_iterator folders_crend() const { return _folders.crend(); }

  private:

    lines _lines;
    folders _folders;
  };

  template <
    typename Item, // Item type to put in the cart.
    tag_t Tag, // Type tag for the cart type.
    tag_t LineTypeTag, // Type tag for the line type in the cart.
    tag_t FolderTypeTag, // Type tag for the folder type in the cart.
    tag_t LinesTag = DefaultCartLinesTag, // Tag for the lines in the cart.
    tag_t FoldersTag = DefaultFoldersTag, // Tag for the folders in the cart.
    tag_t LineItemTag = DefaultItemTag, // Tag for the line link to the item in the line type.
    tag_t LineCountTag = DefaultCountTag, // Tag for the line count in the line type.
    tag_t FolderNameTag = DefaultFolderNameTag // Tag for the folder name in the folder type.
   >
  using cart =
    gen_cart<
    Item,
    Tag,
    LineTypeTag,
    FolderTypeTag,
    LineItemTag,
    LineCountTag,
    cart_line<Item, LineTypeTag, LineItemTag, LineCountTag>,
    LinesTag,
    FoldersTag,
    FolderNameTag
    >;

  template <
    typename Item, // Item type to put in the cart.
    tag_t Tag, // Type tag for the cart type.
    tag_t LineBaseTypeTag, // Type tag for the base class line type in the cart (bearing only the item link and the item count).
    tag_t LineWithSnapshotTypeTag, // Type tag for the line type.
    tag_t FolderTypeTag, // Type tag for the folder type in the cart.
    typename Snapshot, // Snapshot type owned by the cart line. Element or anchor.
    ptr<Snapshot> (*TakeSnapshot)(const rfr<Item>&), // Function to take a snapshot from an item.
    tag_t LinesTag = DefaultCartLinesTag, // Tag for the lines in the cart.
    tag_t FoldersTag = DefaultFoldersTag, // Tag for the folders in the cart.
    tag_t LineItemTag = DefaultItemTag, // Tag for the line link to the item in the line type.
    tag_t LineCountTag = DefaultCountTag, // Tag for the line count in the line type.
    tag_t LineSnapshotTag = DefaultSnapshotTag, // Tag for the snapshot ownership in the line type.
    tag_t FolderNameTag = DefaultFolderNameTag // Tag for the folder name in the folder type. 
    >
  using cart_with_snapshots =
    gen_cart<
    Item,
    Tag,
    LineBaseTypeTag,
    FolderTypeTag,
    LineItemTag,
    LineCountTag,
    cart_line_with_snapshot<Item, LineWithSnapshotTypeTag, LineBaseTypeTag, Snapshot, TakeSnapshot, LineItemTag, LineCountTag, LineSnapshotTag>,
    LinesTag,
    FoldersTag,
    FolderNameTag
    >;

}

#endif
