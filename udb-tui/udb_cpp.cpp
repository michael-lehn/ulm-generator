/*
 *  implemenatation for:
 *  - the C interface specified in udb.h
 *  - the C++ interface specified in udb_cpp.hpp
 */

#include "udb_cpp.hpp"
#include "../ulm2/udb.h"

// implementation for the C++ interface

std::unordered_map<std::size_t, enum UdbMod> udb_reg[2];
std::unordered_map<std::uint64_t, enum UdbMod> udb_mem[2];
std::string udb_out;
std::string udb_in;
bool udb_waitingForInput;
bool udb_eof;
std::unordered_map<std::uint64_t, std::uint64_t> udb_breakpoint;
std::unordered_map<std::unique_ptr<std::function<void()>>, void *> udb_listener;

std::uint64_t numCycles;
std::uint64_t prevIP;

static bool
waitingForInput()
{
    if (udb_in.length() == 0 ||
	(udb_in.front() == '\\' && udb_in.length() == 1)) {
	ulm_waiting = udb_waitingForInput = true;
	return true;
    }
    ulm_waiting = udb_waitingForInput = false;
    return false;
}

std::uint64_t
udb_getNumCycles()
{
    return numCycles;
}

std::uint64_t
udb_getIP()
{
    return ulm_instrPtr;
}

std::uint64_t
udb_getPrevIP()
{
    return prevIP;
}

void
udb_step()
{
    if (!ulm_halted) {
	prevIP = ulm_instrPtr;
	++numCycles;
	ulm_step();
    }
}

void
udb_run()
{
    udb_inStep = false;
    while (!ulm_halted && !udb_illegalInstruction) {
	prevIP = ulm_instrPtr;
	ulm_step();
	if (udb_waitingForInput) {
	    break;
	}
	if (udb_breakpoint.find(ulm_instrPtr) != udb_breakpoint.end()) {
	    break;
	}
    }
    udb_inStep = true;
}

// implementation for the C interface

extern "C" void
udb_addRegMsg(std::size_t regId, enum UdbMod udbMod)
{
    auto found = udb_reg[udb_activeMsg].find(regId);

    if (found != udb_reg[udb_activeMsg].end()) {
	found->second = static_cast<enum UdbMod>(found->second | udbMod);
    } else {
	udb_reg[udb_activeMsg][regId] = udbMod;
    }
}

extern "C" void
udb_addMemMsg(std::uint64_t addr, enum UdbMod udbMod)
{
    auto found = udb_mem[udb_activeMsg].find(addr);

    if (found != udb_mem[udb_activeMsg].end()) {
	found->second = static_cast<enum UdbMod>(found->second | udbMod);
    } else {
	udb_mem[udb_activeMsg][addr] = udbMod;
    }
}

extern "C" void
udb_notify()
{
    for (auto &m : udb_listener) {
	m.first->operator()();
    }
}

extern "C" void
udb_clearMsg()
{
    udb_reg[udb_lastMsg].clear();
    udb_mem[udb_lastMsg].clear();
    std::swap(udb_activeMsg, udb_lastMsg);
}

extern "C" void
ulm_halt(uint64_t code)
{
    ulm_halted = true;
    ulm_exitCode = code;
}

extern "C" {

void
ulm_printChar(int ch)
{
    udb_out += ch;
}

int
ulm_readChar()
{
    if (waitingForInput()) {
	if (udb_eof) {
	    return -1;
	}
	return 0;
    }
    char ch = udb_in.front();
    udb_in = udb_in.substr(1);
    if (ch == '\\') {
	ch = udb_in.front();
	udb_in = udb_in.substr(1);
	switch (ch) {
	    case 'n':
		ch = '\n';
		break;
	    case 't':
		ch = '\t';
		break;
	    default:;
	}
    }
    return ch;
}

} // extern "C"

