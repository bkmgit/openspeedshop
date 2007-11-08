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
 * Definition of the Senders namespace.
 *
 */

#include "Blob.hxx"
#include "CollectorImpl.hxx"
#include "Frontend.hxx"
#include "Protocol.h"
#include "Senders.hxx"
#include "ThreadGroup.hxx"

using namespace OpenSpeedShop::Framework;



namespace {



    /**
     * Convert string for protocol use.
     *
     * Converts the specified C++ string to a C character array as used in
     * protocol messages.
     *
     * @note    The caller assumes responsibility for releasing the C character
     *          array when it is no longer needed.
     *
     * @param in      C++ string to be converted.
     * @retval out    C character array to hold the results.
     */
    void convert(const std::string& in, char*& out)
    {
	if(!in.empty()) {
	    out = new char[in.size()];
	    strcpy(out, in.c_str());
	}
	else
	    out = NULL;
    }
    


    /**
     * Convert collector for protocol use.
     *
     * Converts the identifying information for the specified framework
     * collector object to the structure used in protocol messages.
     *
     * @param in      Collector to be converted.
     * @retval out    Structure to hold the results.
     */
    void convert(const Collector& in, OpenSS_Protocol_Collector& out)
    {
	out.experiment = CollectorImpl::getExperimentId(in);
	out.collector = CollectorImpl::getCollectorId(in);
    }



    /**
     * Convert thread for protocol use.
     *
     * Converts the identifying information for the specified framework
     * thread object to the structure used in protocol messages.
     *
     * @note    The caller assumes responsibility for releasing all allocated
     *          memory when it is no longer needed.
     *
     * @param in      Thread to be converted.
     * @retval out    Structure to hold the results.
     */
    void convert(const Thread& in, OpenSS_Protocol_ThreadName& out)
    {
	convert(in.getHost(), out.host);
	out.pid = in.getProcessId();
	std::pair<bool, pthread_t> posix_tid = in.getPosixThreadId();
	out.posix_tid = posix_tid.first ? posix_tid.second : 0;
    }



    /**
     * Convert thread group for protocol use.
     *
     * Converts the identifying information for the specified framework
     * thread group object to the structure used in protocol messages.
     *
     * @note    The caller assumes responsibility for releasing all allocated
     *          memory when it is no longer needed.
     *
     * @param in      Thread group to be converted.
     * @retval out    Structure to hold the results.
     */
    void convert(const ThreadGroup& in, OpenSS_Protocol_ThreadNameGroup& out)
    {
	// Is the thread group non-empty?
	if(!in.empty()) {

	    // Allocate an appropriately sized array of entries
	    out.names.names_len = in.size();
	    out.names.names_val = new OpenSS_Protocol_ThreadName[in.size()];
	    
	    // Iterate over each thread of this group
	    OpenSS_Protocol_ThreadName* ptr = out.names.names_val;
	    for(ThreadGroup::const_iterator
		    i = in.begin(); i != in.end(); ++i, ++ptr)
		convert(*i, *ptr);

	}
	else {
	    out.names.names_len = 0;
	    out.names.names_val = NULL;
	}
    }



    /**
     * Convert thread for protocol use.
     *
     * Converts the identifying information for the specified framework
     * thread object to the structure used in protocol messages.
     *
     * @note    The caller assumes responsibility for releasing all allocated
     *          memory when it is no longer needed.
     *
     * @param in      Thread to be converted.
     * @retval out    Structure to hold the results.
     */
    void convert(const Thread& in, OpenSS_Protocol_ThreadNameGroup& out)
    {
	ThreadGroup threads;
	threads.insert(in);
	return convert(threads, out);
    }


    
    /**
     * Convert blob for protocol use.
     *
     * Converts the specified framework blob object to the structure used in
     * protocol messages.
     *
     * @note    The caller assumes responsibility for releasing all allocated
     *          memory when it is no longer needed.
     *
     * @param in     Blob to be converted.
     * @param out    Structure to hold the results.
     */
    void convert(const Blob& in, OpenSS_Protocol_Blob& out)
    {
	out.data.data_len = in.getSize();
	out.data.data_val = new uint8_t[in.getSize()];
	memcpy(out.data.data_val, in.getContents(), in.getSize());
    }



}



/**
 * Attach to threads.
 *
 * Issue a request to the backends for the specified threads be attached.
 *
 * @param threads    Threads to be attached.
 */
void Senders::attachToThreads(const ThreadGroup& threads)
{
    // Assemble the request into a message
    OpenSS_Protocol_AttachToThreads message;
    convert(threads, message.threads);
    
    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_AttachToThreads),
	&message
	);

    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_ATTACH_TO_THREADS,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_AttachToThreads),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Change state of threads.
 *
 * Issue a request to the backends for the current state of every thread
 * in the specified group be changed to the specified value. Used to, for
 * example, suspend threads that were previously running.
 *
 * @param threads    Threads whose state should be changed.
 * @param state      Change the threads to this state.
 */
void Senders::changeThreadsState(const ThreadGroup& threads,
				 const Thread::State& state)
{
    // Assemble the request into a message
    OpenSS_Protocol_ChangeThreadsState message;
    convert(threads, message.threads);
    switch(state) {
    case Thread::Running:
	message.state = Running;
	break;
    case Thread::Suspended:
	message.state = Suspended;
	break;
    case Thread::Terminated:
	message.state = Terminated;
	break;
    }
    
    // Encode the message into a blob
    Blob blob(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ChangeThreadsState),
	&message
	);

    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_CHANGE_THREADS_STATE,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ChangeThreadsState),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Create a thread.
 *
 * Issue a request to the backends for the specified thread be created as
 * a new process to execute the specified command. The command is created
 * with the specified initial environment and the process is created in a
 * suspended state.
 *
 * @param thread         Thread to be created (only the host name is actually
 *                       used).
 * @param command        Command to be executed.
 * @param environment    Environment in which to execute the command.
 */
void Senders::createProcess(const Thread& thread,
			    const std::string& command,
			    const Blob& environment)
{
    // Assemble the request into a message
    OpenSS_Protocol_CreateProcess message;
    convert(thread, message.thread);
    convert(command, message.command);
    convert(environment, message.environment);
    
    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_CreateProcess),
	&message
	);

    // Send the encoded message to the appropriate backends
    OpenSS_Protocol_ThreadNameGroup threads;
    convert(thread, threads);
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_CREATE_PROCESS,
			     blob, threads);
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ThreadNameGroup),
	reinterpret_cast<char*>(&threads)
	);
    
    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_CreateProcess),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Detach from threads.
 *
 * Issue a request to the backends for the specified threads be detached.
 *
 * @param threads    Threads to be detached.
 */
void Senders::detachFromThreads(const ThreadGroup& threads)
{
    // Assemble the request into a message
    OpenSS_Protocol_DetachFromThreads message;
    convert(threads, message.threads);
    
    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_DetachFromThreads),
	&message
	);

    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_DETACH_FROM_THREADS,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_DetachFromThreads),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Execute a library function now.
 *
 * Issue a request to the backends for the immediate execution of the
 * specified library function in the specified threads. Used by collectors
 * to execute functions in their runtime library.
 *
 * @param threads             Threads in which the function should be executed.
 * @param collector           Collector requesting the execution.
 * @param disable_save_fpr    Boolean "true" if the floating-point registers
 *                            should NOT be saved before executing the library
 *                            function, or "false" if they should be saved.
 * @param callee              Name of the library function to be executed.
 * @param argument            Blob argument to the function.
 */
void Senders::executeNow(const ThreadGroup& threads,
			 const Collector& collector,
			 const bool& disable_save_fpr,
			 const std::string& callee,
			 const Blob& argument)
{
    // Assemble the request into a message
    OpenSS_Protocol_ExecuteNow message;
    convert(threads, message.threads);
    convert(collector, message.collector);
    message.disable_save_fpr = disable_save_fpr;
    convert(callee, message.callee);
    convert(argument, message.argument);

    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteNow),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_EXECUTE_NOW,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteNow),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Execute a library function at another function's entry or exit.
 *
 * Issue a request to the backends for the execution of the specified
 * library function every time another function's entry or exit is
 * executed in the specified threads. Used by collectors to execute
 * functions in their runtime library.
 *
 * @param threads      Threads in which the function should be executed.
 * @param collector    Collector requesting the execution.
 * @param where        Name of the function at whose entry/exit the library
 *                     function should be executed.
 * @param at_entry     Boolean "true" if instrumenting the function's entry
 *                     point, or "false" if function's exit point.
 * @param callee       Name of the library function to be executed.
 * @param argument     Blob argument to the function.
 */
void Senders::executeAtEntryOrExit(const ThreadGroup& threads,
				   const Collector& collector,
				   const std::string& where,
				   const bool& at_entry,
				   const std::string& callee,
				   const Blob& argument)
{
    // Assemble the request into a message
    OpenSS_Protocol_ExecuteAtEntryOrExit message;
    convert(threads, message.threads);
    convert(collector, message.collector);
    convert(where, message.where);
    message.at_entry = at_entry;
    convert(callee, message.callee);
    convert(argument, message.argument);
    
    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteAtEntryOrExit),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_EXECUTE_AT_ENTRY_OR_EXIT,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteAtEntryOrExit),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Execute a library function in place of another function.
 *
 * Issue a request to the backends for the execution of the specified
 * library function every time another function's entry or exit is
 * executed in the specified threads. Used by collectors to execute
 * functions in their runtime library.
 *
 * @param threads      Threads in which the function should be executed.
 * @param collector    Collector requesting the execution.
 * @param where        Name of the function to be replaced with the library
 *                     function.
 * @param callee       Name of the library function to be executed.
 */
void Senders::executeInPlaceOf(const ThreadGroup& threads,
			       const Collector& collector,
			       const std::string& where,
			       const std::string& callee)
{
    // Assemble the request into a message
    OpenSS_Protocol_ExecuteInPlaceOf message;
    convert(threads, message.threads);
    convert(collector, message.collector);
    convert(where, message.where);
    convert(callee, message.callee);

    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteInPlaceOf),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_EXECUTE_IN_PLACE_OF,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ExecuteInPlaceOf),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Get value of an integer global variable from a thread.
 *
 * Issue a request to the backends for the current value of a signed integer
 * global variable within the specified thread. Used to extract certain types
 * of data, such as MPI job identifiers, from a process.
 *
 * @param thread    Thread from which the global variable should be retrieved.
 * @param global    Name of global variable whose value is being requested.
 */
void Senders::getGlobalInteger(const Thread& thread, const std::string& global)
{
    // Assemble the request into a message
    OpenSS_Protocol_GetGlobalInteger message;
    convert(thread, message.thread);
    convert(global, message.global);    

    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetGlobalInteger),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    OpenSS_Protocol_ThreadNameGroup threads;
    convert(thread, threads);
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_GET_GLOBAL_INTEGER,
			     blob, threads);
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ThreadNameGroup),
	reinterpret_cast<char*>(&threads)
	);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetGlobalInteger),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Get value of a string global variable from a thread.
 *
 * Issue a request to the backends for the current value of a character string
 * global variable within the specified thread. Used to extract certain types
 * of data, such as MPI job identifiers, from a process.
 *
 * @param thread    Thread from which the global variable should be retrieved.
 * @param global    Name of global variable whose value is being requested.
 */
void Senders::getGlobalString(const Thread& thread, const std::string& global)
{
    // Assemble the request into a message
    OpenSS_Protocol_GetGlobalString message;
    convert(thread, message.thread);
    convert(global, message.global);

    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetGlobalString),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    OpenSS_Protocol_ThreadNameGroup threads;
    convert(thread, threads);
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_GET_GLOBAL_STRING,
			     blob, threads);
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ThreadNameGroup),
	reinterpret_cast<char*>(&threads)
	);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetGlobalString),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Get value of the MPICH process table from a thread.
 *
 * Issue a request to the backends for the current value of the MPICH process
 * table within the specified thread. Used to obtain this information for the
 * purposes of attaching to an entire MPI job.
 *
 * @param thread    Thread from which the MPICH process table should be
 *                  retrieved.
 */
void Senders::getMPICHProcTable(const Thread& thread)
{
    // Assemble the request into a message
    OpenSS_Protocol_GetMPICHProcTable message;
    convert(thread, message.thread);
    
    // Encode the message into a blob
    Blob blob(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetMPICHProcTable),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    OpenSS_Protocol_ThreadNameGroup threads;
    convert(thread, threads);
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_GET_MPICH_PROC_TABLE,
			     blob, threads);
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ThreadNameGroup),
	reinterpret_cast<char*>(&threads)
	);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_GetMPICHProcTable),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Set value of an integer global variable in a thread.
 *
 * Issue a request to the backends for a change to the current value of a
 * signed integer global variable within the specified thread. Used to modify
 * certain values, such as MPI debug gates, in a process.
 *
 * @param thread    Thread in which the global variable value should be set.
 * @param global    Name of global variable whose value is being set.
 * @param value     New value of that variable.
 */
void Senders::setGlobalInteger(const Thread& thread,
			       const std::string& global,
			       const int64_t& value)
{
    // Assemble the request into a message
    OpenSS_Protocol_SetGlobalInteger message;
    convert(thread, message.thread);
    convert(global, message.global);
    message.value = value;
    
    // Encode the message into a blob
    Blob blob(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_SetGlobalInteger),
	&message
	);

    // Send the encoded message to the appropriate backends
    OpenSS_Protocol_ThreadNameGroup threads;
    convert(thread, threads);
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_SET_GLOBAL_INTEGER,
			     blob, threads);
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_ThreadNameGroup),
	reinterpret_cast<char*>(&threads)
	);
    
    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_SetGlobalInteger),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Stop at a function's entry or exit.
 *
 * Issue a request to the backends for a stop every time the specified
 * function's entry or exit is executed in the specified threads. Used
 * by the framework to implement MPI job creation.
 *
 * @param threads     Threads which should be stopped.
 * @param where       Name of the function at whose entry/exit the stop
 *                    should occur.
 * @param at_entry    Boolean "true" if instrumenting function's entry
 *                    point or "false" if function's exit point.
 */
void Senders::stopAtEntryOrExit(const ThreadGroup& threads,
				const std::string& where,
				const bool& at_entry)
{
    // Assemble the request into a message
    OpenSS_Protocol_StopAtEntryOrExit message;
    convert(threads, message.threads);
    convert(where, message.where);
    message.at_entry = at_entry;

    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_StopAtEntryOrExit),
	&message
	);
    
    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_STOP_AT_ENTRY_OR_EXIT,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_StopAtEntryOrExit),
	reinterpret_cast<char*>(&message)
	);
}



/**
 * Remove instrumentation from threads.
 *
 * Issue a request to the backends for the removal of all instrumentation
 * associated with the specified collector from the specified threads. Used
 * by collectors to indicate when they are done using any instrumentation
 * they have placed in threads.
 *
 * @param threads      Threads from which instrumentation should be removed.
 * @param collector    Collector which is removing instrumentation
 */
void Senders::uninstrument(const ThreadGroup& threads,
			   const Collector& collector)
{
    // Assemble the request into a message
    OpenSS_Protocol_Uninstrument message;
    convert(threads, message.threads);
    convert(collector, message.collector);
    
    // Encode the message into a blob
    Blob blob(
        reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_Uninstrument),
	&message
	);

    // Send the encoded message to the appropriate backends
    Frontend::sendToBackends(OPENSS_PROTOCOL_TAG_UNINSTRUMENT,
			     blob, message.threads);

    // Destroy the message
    xdr_free(
	reinterpret_cast<xdrproc_t>(xdr_OpenSS_Protocol_Uninstrument),
	reinterpret_cast<char*>(&message)
	);
}
