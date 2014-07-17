/* The code in this file is released into the Public Domain. */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

std::string filename(const char* test_id) {
    const char* testdir = getenv("TESTDIR");
    if (!testdir) {
        std::cerr << "You have to set TESTDIR environment variable before running testdata-xml\n";
        exit(2);
    }

    std::string f;
    f += testdir;
    f += "/";
    f += test_id;
    f += "/data.osm";
    return f;
}

struct header_buffer_type {
    osmium::io::Header header;
    osmium::memory::Buffer buffer;
};

// This helper function is used to parse XML data without the usual threading.
// It is only for use in testing because it makes some assumptions which will
// not always be true in normal code.
header_buffer_type read_xml(const char* test_id) {
    osmium::thread::Queue<std::string> input_queue;
    osmium::thread::Queue<osmium::memory::Buffer> output_queue;
    std::promise<osmium::io::Header> header_promise;
    std::atomic<bool> done {false};

    osmium::io::detail::XMLParser parser(input_queue, output_queue, header_promise, osmium::osm_entity_bits::all, done);

    int fd = osmium::io::detail::open_for_reading(filename(test_id));
    assert(fd >= 0);
    std::string input(10000, '\0');
    int n = ::read(fd, reinterpret_cast<unsigned char*>(const_cast<char*>(input.data())), 10000);
    assert(n >= 0);
    input.resize(n);
    input_queue.push(input);
    input_queue.push(std::string());

//    std::cerr << "call parser for " << test_id << ":\n";
    parser();

    header_buffer_type result;
//    std::cerr << "  get header...\n";
    result.header = header_promise.get_future().get();
//    std::cerr << "  get buffer...\n";
    output_queue.wait_and_pop(result.buffer);

//    std::cerr << "  check is done...\n";
    if (result.buffer) {
        osmium::memory::Buffer buffer;
        output_queue.wait_and_pop(buffer);
        assert(!buffer);
    }

//    std::cerr << "  DONE\n";
    close(fd);

    return result;
}


TEST_CASE("Reading OSM XML 100") {

    SECTION("Direct") {
        header_buffer_type r = read_xml("100-correct_but_no_data");

        REQUIRE(r.header.get("generator") == "testdata");
        REQUIRE(0 == r.buffer.committed());
        REQUIRE(! r.buffer);
    }

    SECTION("Using Reader") {
        osmium::io::Reader reader(filename("100-correct_but_no_data"));

        osmium::io::Header header = reader.header();
        REQUIRE(header.get("generator") == "testdata");

        osmium::memory::Buffer buffer = reader.read();
        REQUIRE(0 == buffer.committed());
        REQUIRE(! buffer);
    }

}

// =============================================

TEST_CASE("Reading OSM XML 101") {

    SECTION("Direct") {
        REQUIRE_THROWS_AS(read_xml("101-missing_version"), osmium::format_version_error);
        try {
            read_xml("101-missing_version");
        } catch (osmium::format_version_error& e) {
            REQUIRE(e.version.empty());
        }
    }

#if 0
    SECTION("Using Reader") {
        try {
            osmium::io::Reader reader(filename("101-missing_version"));

            osmium::io::Header header = reader.header();
            REQUIRE(header.get("generator") == "testdata");

            osmium::memory::Buffer buffer = reader.read();
        } catch (osmium::format_version_error& e) {
            REQUIRE(e.version.empty());
        }
    }
#endif

}

// =============================================

TEST_CASE("Reading OSM XML 102") {

    SECTION("Direct") {
        REQUIRE_THROWS_AS(read_xml("102-wrong_version"), osmium::format_version_error);
        try {
            read_xml("102-wrong_version");
        } catch (osmium::format_version_error& e) {
            REQUIRE(e.version == "0.1");
        }
    }

#if 0
    SECTION("Using Reader") {
        try {
            osmium::io::Reader reader(filename("102-wrong_version"));

            osmium::io::Header header = reader.header();
            REQUIRE(header.get("generator") == "testdata");

            osmium::memory::Buffer buffer = reader.read();
        } catch (osmium::format_version_error& e) {
            REQUIRE(e.version == "0.1");
        }
    }
#endif

}

// =============================================

TEST_CASE("Reading OSM XML 103") {

    SECTION("Direct") {
        REQUIRE_THROWS_AS(read_xml("103-old_version"), osmium::format_version_error);
        try {
            read_xml("103-old_version");
        } catch (osmium::format_version_error& e) {
            REQUIRE(e.version == "0.5");
        }
    }

#if 0
    SECTION("Using Reader") {
        osmium::io::Reader reader(filename("103-old_version"));

        osmium::io::Header header = reader.header();
        REQUIRE(header.get("generator") == "testdata");

        osmium::memory::Buffer buffer = reader.read();
    }
#endif

}

// =============================================

TEST_CASE("Reading OSM XML 104") {

    SECTION("Direct") {
        REQUIRE_THROWS_AS(read_xml("104-empty_file"), osmium::xml_error);
        try {
            read_xml("104-empty_file");
        } catch (osmium::xml_error& e) {
            REQUIRE(e.line == 1);
            REQUIRE(e.column == 0);
        }
    }

#if 0
    SECTION("Using Reader") {
        osmium::io::Reader reader(filename("104-empty_file"));

        osmium::io::Header header = reader.header();
        REQUIRE(header.get("generator") == "testdata");

        osmium::memory::Buffer buffer = reader.read();
    }
#endif

}

// =============================================

TEST_CASE("Reading OSM XML 105") {

    SECTION("Direct") {
        REQUIRE_THROWS_AS(read_xml("105-incomplete_xml_file"), osmium::xml_error);
    }

#if 0
    SECTION("Using Reader") {
        osmium::io::Reader reader(filename("105-incomplete_xml_file"));

        osmium::io::Header header = reader.header();
        REQUIRE(header.get("generator") == "testdata");

        osmium::memory::Buffer buffer = reader.read();
    }
#endif

}

// =============================================

TEST_CASE("Reading OSM XML 200") {

    SECTION("Test 200") {
        header_buffer_type r = read_xml("200-nodes");

        REQUIRE(r.header.get("generator") == "testdata");
        REQUIRE(r.buffer.committed() > 0);
        REQUIRE(r.buffer.get<osmium::memory::Item>(0).type() == osmium::item_type::node);
        REQUIRE(r.buffer.get<osmium::Node>(0).id() == 36966060);
        REQUIRE(std::distance(r.buffer.begin(), r.buffer.end()) == 3);
    }

}

