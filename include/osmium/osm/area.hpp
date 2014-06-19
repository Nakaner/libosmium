#ifndef OSMIUM_OSM_AREA_HPP
#define OSMIUM_OSM_AREA_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cassert>
#include <cstddef>
#include <utility>

#include <osmium/memory/collection.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/node_ref.hpp>

namespace osmium {

    namespace builder {
        template <class T> class ObjectBuilder;
    }

    /**
     * An outer ring of an Area.
     */
    class OuterRing : public osmium::memory::Collection<NodeRef, osmium::item_type::outer_ring> {

    public:

        OuterRing():
            osmium::memory::Collection<NodeRef, osmium::item_type::outer_ring>() {
        }

        size_t size() const noexcept {
            assert((byte_size() - sizeof(OuterRing)) % sizeof(NodeRef) == 0);
            return (byte_size() - sizeof(OuterRing)) / sizeof(NodeRef);
        }

        const NodeRef& operator[](size_t n) const {
            const NodeRef* node_ref = &*begin();
            return node_ref[n];
        }

    }; // class OuterRing

    static_assert(sizeof(OuterRing) % osmium::memory::align_bytes == 0, "Class osmium::OuterRing has wrong size to be aligned properly!");

    /**
     * An inner ring of an Area.
     */
    class InnerRing : public osmium::memory::Collection<NodeRef, osmium::item_type::inner_ring> {

    public:

        InnerRing():
            osmium::memory::Collection<NodeRef, osmium::item_type::inner_ring>() {
        }

        size_t size() const noexcept {
            assert((byte_size() - sizeof(InnerRing)) % sizeof(NodeRef) == 0);
            return (byte_size() - sizeof(InnerRing)) / sizeof(NodeRef);
        }

        const NodeRef& operator[](size_t n) const {
            const NodeRef* node_ref = &*begin();
            return node_ref[n];
        }

    }; // class InnerRing

    static_assert(sizeof(InnerRing) % osmium::memory::align_bytes == 0, "Class osmium::InnerRing has wrong size to be aligned properly!");

    /**
     * An OSM area created out of a closed way or a multipolygon relation.
     */
    class Area : public Object {

        friend class osmium::builder::ObjectBuilder<osmium::Area>;

        Area() :
            Object(sizeof(Area), osmium::item_type::area) {
        }

    public:

        static constexpr osmium::item_type itemtype = osmium::item_type::area;

        /**
         * Was this area created from a way? (In contrast to areas
         * created from a relation and their members.)
         */
        bool from_way() const {
            return (positive_id() & 0x1) == 0;
        }

        /**
         * Return the Id of the way or relation this area was created from.
         */
        osmium::object_id_type orig_id() const {
            return id() / 2;
        }

        /**
         * Count the number of outer and inner rings of this area.
         */
        std::pair<int, int> num_rings() const {
            std::pair<int, int> counter;

            for (auto it = cbegin(); it != cend(); ++it) {
                switch (it->type()) {
                    case osmium::item_type::outer_ring:
                        ++counter.first;
                        break;
                    case osmium::item_type::inner_ring:
                        ++counter.second;
                        break;
                    default:
                        break;
                }
            }

            return counter;
        }

    }; // class Area

    static_assert(sizeof(Area) % osmium::memory::align_bytes == 0, "Class osmium::Area has wrong size to be aligned properly!");

} // namespace osmium

#endif // OSMIUM_OSM_AREA_HPP
