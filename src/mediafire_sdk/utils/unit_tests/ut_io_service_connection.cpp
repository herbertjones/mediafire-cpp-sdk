// @copyright Copyright 2014 MediaFire, LLC.
#define BOOST_TEST_MODULE IoServiceConnection
#include <string>

#include "boost/asio/impl/src.hpp" // Define once in program
#include "boost/test/unit_test.hpp"
#include "boost/asio/io_service.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/bind.hpp"

#include "mediafire_sdk/utils/io_service_connection.hpp"

struct Fixture
{
    Fixture() :
        io_service()
    {
    }

    ~Fixture()
    {
    }

    boost::asio::io_service io_service;
};

BOOST_FIXTURE_TEST_SUITE(IoServiceConnection, Fixture)

struct DestroyedObjectException : std::exception {};

struct DestroyableObject
{
    DestroyableObject() : safe_int(0), death_trap(&safe_int) {}
    ~DestroyableObject() { death_trap = NULL; }

    void dereference_death_trap()
    {
        if ( ! death_trap ) throw DestroyedObjectException();

        // Crash the program if the object has been deleted and the memory has
        // been reused as a last resort
        *death_trap = 1;
    }

    bool death_trap_called() const
    {
        return ( *death_trap != 0 );
    }

    int safe_int;
    int* death_trap;
};

BOOST_AUTO_TEST_CASE(IoServiceConnection_Basic)
{
    boost::scoped_ptr<DestroyableObject> destroyable(new DestroyableObject);
    boost::scoped_ptr<mf::utils::IoServiceConnection> connection(new mf::utils::IoServiceConnection(&io_service));

    // Test that nothing throws if nothing is destroyed and that the function is being called
    BOOST_CHECK( ! destroyable->death_trap_called() );
    connection->Post(boost::bind(&DestroyableObject::dereference_death_trap, destroyable.get()));
    BOOST_CHECK_NO_THROW( io_service.run() );
    BOOST_CHECK( destroyable->death_trap_called() );
    io_service.reset();

    // Test that deleting the connection resolves any problem with posted messages
    connection->Post(boost::bind(&DestroyableObject::dereference_death_trap, destroyable.get()));
    connection.reset();
    BOOST_CHECK_NO_THROW( io_service.run() );
}

BOOST_AUTO_TEST_SUITE_END()
