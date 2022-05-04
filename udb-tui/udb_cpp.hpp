#ifndef UDBTUI_UDBCPP_HPP
#define UDBTUI_UDBCPP_HPP

/*
 *  udb.h is the C interface used by the ulm to send notifications
 *
 *  udb_cpp.hpp provides an interface for C++ listeners
 *
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ulm2/ulm.h"

//
// C++ only part
//

struct RegMsg
{
    std::size_t id;
    enum UdbMod udbMod;
};

extern int udb_activeMsg;
extern int udb_lastMsg;
extern std::unordered_map<std::size_t, enum UdbMod> udb_reg[];
extern std::unordered_map<std::uint64_t, enum UdbMod> udb_mem[];
extern std::string udb_out;
extern std::string udb_in;
extern bool udb_waitingForInput;
extern bool udb_eof;
extern std::unordered_map<std::uint64_t,std::uint64_t> udb_breakpoint;

extern std::unordered_map<std::unique_ptr<std::function<void()>>, void *>
  udb_listener;

template<typename Listener, typename Method>
void
udb_addCallback(Listener *listener, Method &&method)
{
    auto fn =
      std::make_unique<std::function<void()>>(std::bind(method, listener));
    udb_listener[std::move(fn)] = listener;
}

template<typename Listener>
void
udb_delListener(Listener *listener)
{
    // delete all callback methods from listener
    for (auto it = udb_listener.begin(); it != udb_listener.end();) {
	if (it->second == listener) {
	    it = udb_listener.erase(it);
	} else {
	    ++it;
	}
    }
}

std::uint64_t udb_getNumCycles();
std::uint64_t udb_getIP();
std::uint64_t udb_getPrevIP();
void udb_step();
void udb_run();

#endif // UDBTUI_UDBCPP_HPP

