////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 William Hachfeld. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file
 *
 * Definition of the Callbacks namespace.
 *
 */

#include "Blob.hxx"
#include "Backend.hxx"
#include "Callbacks.hxx"
#include "Protocol.h"
#include "Senders.hxx"
#include "ThreadName.hxx"
#include "ThreadNameGroup.hxx"
#include "ThreadTable.hxx"
#include "Utility.hxx"

#include <BPatch.h>
#include <iostream>
#include <sstream>

using namespace OpenSpeedShop::Framework;



/**
 * Attach to threads.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::attachToThreads(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_AttachToThreads message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_AttachToThreads),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // Iterate over each thread to be attached
    ThreadNameGroup threads_to_attach = message.threads, threads_attached;
    for(ThreadNameGroup::const_iterator
	    i = threads_to_attach.begin(); i != threads_to_attach.end(); ++i) {
	
	// Has this thread already been attached?
	if(ThreadTable::TheTable.getPtr(*i) != NULL) {
#ifndef NDEBUG
	    if(Backend::isDebugEnabled()) {
		std::stringstream output;
		output << "[TID " << pthread_self() << "] Callbacks::"
		       << "attachToThreads(): Thread " << toString(*i)
		       << " is already attached." << std::endl;
		std::cerr << output.str();
	    }
#endif
	    continue;
	}

	// Has this thread already been attached in a different experiment?
	BPatch_thread* thread = ThreadTable::getPtrDirectly(*i);
	if(thread != NULL) {
	    
#ifndef NDEBUG
	    if(Backend::isDebugEnabled()) {
		std::stringstream output;
		output << "[TID " << pthread_self() << "] Callbacks::"
		       << "attachToThreads(): Thread " << toString(*i)
		       << " is already attached in another experiment." 
		       << std::endl;
		std::cerr << output.str();
	    }
#endif
	    
	    // Add this thread to the thread table and group of attached threads
	    ThreadName name(i->getExperiment(), *thread);
	    ThreadTable::TheTable.addThread(name, thread);
	    threads_attached.insert(name);
	    
	    continue;
	}

	// Otherwise attach to the process containing the specified thread
	BPatch* bpatch = BPatch::getBPatch();
	Assert(bpatch != NULL);
	thread = bpatch->attachProcess(NULL, i->getProcessId());
	Assert(thread != NULL);
	BPatch_process* process = thread->getProcess();
	Assert(process != NULL);
	
	// Obtain the list of threads in this process
	BPatch_Vector<BPatch_thread*> threads;
	process->getThreads(threads);
	Assert(!threads.empty());
	
	// Iterate over each thread in this process
	for(int j = 0; j < threads.size(); ++j) {
	    Assert(threads[j] != NULL);

	    // Add this thread to the thread table and group of attached threads
	    ThreadName name(i->getExperiment(), *(threads[j]));
	    ThreadTable::TheTable.addThread(name, threads[j]);
	    threads_attached.insert(name);
	    
	}
	
    }
    
    // Send the frontend the list of threads that were attached
    Senders::attachedToThreads(threads_attached);

    // TODO: continue implementing!

}



/**
 * Change state of threads.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::changeThreadsState(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_ChangeThreadsState message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ChangeThreadsState),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // Iterate over each thread to be changed
    ThreadNameGroup threads_to_change = message.threads, threads_changed;
    for(ThreadNameGroup::const_iterator
	    i = threads_to_change.begin(); i != threads_to_change.end(); ++i) {

	// Obtain the Dyninst thread and process pointers for this thread
	BPatch_thread* thread = ThreadTable::TheTable.getPtr(*i);
	if(thread == NULL) {
#ifndef NDEBUG
	    if(Backend::isDebugEnabled()) {
		std::stringstream output;
		output << "[TID " << pthread_self() << "] Callbacks::"
		       << "changeThreadsState(): Thread " << toString(*i)
		       << " is not attached." << std::endl;
		std::cerr << output.str();
	    }
#endif
	    continue;
	}
	BPatch_process* process = thread->getProcess();
	Assert(process != NULL);
	
	// Change the state of the process containing this thread
	bool did_change = false;
	switch(message.state) {
	case Running:
	    did_change = process->continueExecution();
	    break;
	case Suspended:
	    did_change = process->stopExecution();
	    break;
	case Terminated:
	    did_change = process->terminateExecution();
	    break;
	default:
	    break;
	}

	// Did the state of the process change?
	if(did_change) {

	    // Add all of these threads to the group of changed threads
	    ThreadNameGroup threads(i->getExperiment(), *process);
	    threads_changed.insert(threads.begin(), threads.end());
	    
	}
	
    }

    // Send the frontend the list of threads that were changed
    Senders::threadsStateChanged(threads_changed, message.state);
}



/**
 * Create a thread.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::createProcess(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_CreateProcess message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_CreateProcess),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Detach from threads.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::detachFromThreads(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_DetachFromThreads message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_DetachFromThreads),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Execute a library function now.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::executeNow(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_ExecuteNow message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteNow),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Execute a library function at another function's entry or exit.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::executeAtEntryOrExit(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_ExecuteAtEntryOrExit message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteAtEntryOrExit),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Execute a library function in place of another function.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::executeInPlaceOf(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_ExecuteInPlaceOf message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteInPlaceOf),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Get value of an integer global variable from a thread.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::getGlobalInteger(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_GetGlobalInteger message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetGlobalInteger),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Get value of a string global variable from a thread.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::getGlobalString(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_GetGlobalString message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetGlobalString),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Get value of the MPICH process table from a thread.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::getMPICHProcTable(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_GetMPICHProcTable message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetMPICHProcTable),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Set value of an integer global variable in a thread.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::setGlobalInteger(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_SetGlobalInteger message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_SetGlobalInteger),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Stop at a function's entry or exit.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::stopAtEntryOrExit(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_StopAtEntryOrExit message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_StopAtEntryOrExit),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}



/**
 * Remove instrumentation from threads.
 *
 * ...
 *
 * @param blob    Blob containing the message.
 */
void Callbacks::uninstrument(const Blob& blob)
{
    // Decode the message
    OpenSS_Protocol_Uninstrument message;
    memset(&message, 0, sizeof(message));
    blob.getXDRDecoding(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_Uninstrument),
	&message
	);

#ifndef NDEBUG
    if(Backend::isDebugEnabled()) {
	std::stringstream output;
	output << "[TID " << pthread_self() << "] Callbacks::"
	       << toString(message);
	std::cerr << output.str();
    }
#endif

    // TODO: implement!
}
