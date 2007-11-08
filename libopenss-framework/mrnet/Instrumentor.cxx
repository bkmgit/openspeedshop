////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 William Hachfeld. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file
 *
 * Definition of the Instrumentor namespace.
 *
 */

#include "Instrumentor.hxx"
#include "Senders.hxx"
#include "ThreadGroup.hxx"
#include "ThreadTable.hxx"

using namespace OpenSpeedShop::Framework;



/**
 * Retain a thread.
 *
 * Retains a thread in this instrumentor. Retaining a thread simply indicates
 * to the instrumentor that the thread is currently in use. This information is
 * used by the instrumentor to decide when it is appropriate to disconnect from
 * the underlying process in the operating system that contains this thread.
 *
 * @param thread    Thread to be retained.
 */
void Instrumentor::retain(const Thread& thread)
{
    // Add this thread to the thread table
    ThreadTable::TheTable.addThread(thread);
}



/**
 * Release a thread.
 *
 * Releases a thread in this instrumentor. Releasing a thread simply indicates
 * to the instrumentor that the thread is no longer in use. This information is
 * used by the instrumentor to decide when it is appropriate to disconnect from
 * the underlying thread in the operating system that contains this thread.
 *
 * @param thread    Thread to be released.
 */
void Instrumentor::release(const Thread& thread)
{
    ThreadGroup threads;
    threads.insert(thread);

    // Request these threads be detached
    Senders::detachFromThreads(threads);
    
    // Remove this thread from the thread table
    ThreadTable::TheTable.removeThread(thread);
}



/**
 * Create a thread.
 *
 * Creates the specified thread as a new process to execute the passed command.
 * The command is created with the same initial environment as when the tool was
 * started. The process is created in a suspended state.
 *
 * @param thread             Thread to be created.
 * @param command            Command to be executed.
 * @param stdout_callback    Standard output stream callback for the process.
 * @param stderr_callback    Standard error stream callback for the process.
 */
void Instrumentor::create(const Thread& thread, 
			  const std::string& command,
			  const OutputCallback stdout_cb,
			  const OutputCallback stderr_cb)
{
    // TODO: implement!
}



/**
 * Get a thread's state.
 *
 * Returns the current state of the specified thread. Since this state changes
 * asynchronously, and may be updated across a network, there is a lag between
 * when the actual thread's state changes and when it is reflected here.
 *
 * @param thread    Thread whose state should be obtained.
 * @return          Current state of the thread.
 */
Thread::State Instrumentor::getState(const Thread& thread)
{
    // Return the thread's current state to the caller
    return ThreadTable::TheTable.getThreadState(thread);
}



/**
 * Change state of threads.
 *
 * Changes the current state of the specified threads to the passed value. Used
 * to, for example, suspend threads that were previously running. This function
 * does not wait until the threads have actually completed the state change, and
 * calling getState() immediately following changeState() will not reflect the 
 * new state until the change has actually completed.
 *
 * @pre    Only applies to threads which are connected. A ProcessUnavailable or
 *         ThreadUnavailable exception is thrown if called for a thread that is
 *         not connected. There is one exception - disconnected threads may be
 *         changed to the "connecting" state.
 *
 * @param threads    Threads whose state should be changed.
 * @param state      Change the threads to this state.
 */
void Instrumentor::changeState(const ThreadGroup& threads,
			       const Thread::State& state)
{
    // TODO:  handle connecting
    
    // Request a state change from the threads
    Senders::changeThreadsState(threads, state);
}



/**
 * Execute a library function now.
 *
 * Immediately execute the specified library function in the specified threads.
 * Used by collectors to execute functions in their runtime library.
 *
 * @pre    Only applies to threads which are connected. A ProcessUnavailable or
 *         ThreadUnavailable exception is thrown if called for a thread that is
 *         not connected.
 *
 * @param threads      Threads in which the function should be executed.
 * @param collector    Collector requesting the execution.
 * @param callee       Name of the library function to be executed.
 * @param argument     Blob argument to the function.
 */
void Instrumentor::executeNow(const ThreadGroup& threads, 
			      const Collector& collector,
			      const std::string callee, 
			      const Blob& argument,
			      const bool& disableSaveFPR)
{
    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the library function be executed by these threads
    Senders::executeNow(threads, collector, disableSaveFPR, callee, argument);
}



/**
 * Execute a library function at another function's entry or exit.
 *
 * Executes the specified library function every time another function's entry
 * or exit is executed in the specified threads. Used by collectors to execute
 * functions in their runtime library.
 *
 * @pre    Only applies to threads which are connected. A ProcessUnavailable or
 *         ThreadUnavailable exception is thrown if called for a thread that is
 *         not connected.
 *
 * @param threads      Threads in which the function should be executed.
 * @param collector    Collector requesting the execution.
 * @param where        Name of the function at whose entry/exit the library
 *                     function should be executed.
 * @param at_entry     Boolean "true" if instrumenting function's entry point,
 *                     or "false" if function's exit point.
 * @param callee       Name of the library function to be executed.
 * @param argument     Blob argument to the function.
 */
void Instrumentor::executeAtEntryOrExit(const ThreadGroup& threads,
					const Collector& collector,
					const std::string& where, 
					const bool& at_entry,
					const std::string& callee, 
					const Blob& argument)
{
    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the library function be executed in these threads
    Senders::executeAtEntryOrExit(threads, collector, where,
				  at_entry, callee, argument);
}



/**
 * Execute a library function in place of another function.
 *
 * Executes the specified library function in place of another function every
 * other time that other function is called. Used by collectors to create
 * wrappers around functions for the purposes of gathering performance data on
 * their execution.
 *
 * @pre    Only applies to threads which are connected. A ProcessUnavailable or
 *         ThreadUnavailable exception is thrown if called for a thread that is
 *         not connected.
 *
 * @param threads      Threads in which the function should be executed.
 * @param collector    Collector requesting the execution.
 * @param where        Name of the function to be replaced with the library
 *                     function.
 * @param callee       Name of the library function to be executed.
 */
void Instrumentor::executeInPlaceOf(const ThreadGroup& threads,
				    const Collector& collector,
				    const std::string& where, 
				    const std::string& callee)
{
    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the library function be executed in these threads
    Senders::executeInPlaceOf(threads, collector, where, callee);
}



/**
 * Remove instrumentation from threads.
 *
 * Removes all instrumentation associated with the specified collector from the
 * specified threads. Used by collectors to indicate when they are done using
 * any instrumentation they have placed in threads.
 *
 * @pre    Only applies to threads which are connected. A ProcessUnavailable or
 *         ThreadUnavailable exception is thrown if called for a thread that is
 *         not connected.
 *
 * @param threads      Threads from which instrumentation should be removed.
 * @param collector    Collector which is removing instrumentation.
 */	
void Instrumentor::uninstrument(const ThreadGroup& threads, 
				const Collector& collector)
{
    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);
    
    // Request the collector's instrumentation be removed from these threads
    Senders::uninstrument(threads, collector);
}



/**
 * Stop at a function's entry or exit.
 *
 * Stops every time the specified function's entry or exit is executed in the
 * specified thread. Used by the framework to implement MPI job creation.
 *
 * @pre    Only applies to a thread which is connected. A ProcessUnavailable or
 *         ThreadUnavailable exception is thrown if called for a thread that is
 *         not connected.
 *
 * @param thread      Thread which should be stopped.
 * @param where       Name of the function at whose entry/exit the stop should
 *                    occur.
 * @param at_entry    Boolean "true" if instrumenting function's entry point,
 *                    or "false" if function's exit point.
 */
void Instrumentor::stopAtEntryOrExit(const Thread& thread,
				     const std::string& where, 
				     const bool& at_entry)
{
    ThreadGroup threads;
    threads.insert(thread);

    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the stop be added to these threads
    Senders::stopAtEntryOrExit(threads, where, at_entry);
}



/**
 * Get value of an integer global variable from a thread.
 *
 * Gets the current value of a signed integer global variable within this
 * thread. Used to extract certain types of data, such as MPI job identifiers,
 * from a process.
 *
 * @pre    Only applies to a process which is connected. A ProcessUnavailable
 *         exception is thrown if called for a thread that is not connected.
 *
 * @param thread    Thread from which the global variable value should be
 *                  retrieved.
 * @param global    Name of global variable whose value is being requested.
 * @retval value    Current value of that variable.
 * @return          Boolean "true" if the variable's value was successfully
 *                  retrieved, "false" otherwise.
 */
bool Instrumentor::getGlobal(const Thread& thread,
			     const std::string& global,
			     int64_t& value)
{
    ThreadGroup threads;
    threads.insert(thread);

    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the global variable be retrieved from the thread
    Senders::getGlobalInteger(thread, global);

    // TODO: wait for response and return it

    return false;  // TEMPORARY DUMMY RETURN
}



/**
 * Get value of a string global variable from a thread.
 *
 * Gets the current value of a character string global variable within this
 * thread. Used to extract certain types of data, such as MPI job identifiers,
 * from a process.
 *
 * @pre    Only applies to a thread which is connected. A ThreadUnavailable
 *         exception is thrown if called for a thread that is not connected.
 *
 * @param thread    Thread from which the global variable value should be
 *                  retrieved.
 * @param global    Name of global variable whose value is being requested.
 * @retval value    Current value of that variable.
 * @return          Boolean "true" if the variable's value was successfully
 *                  retrieved, "false" otherwise.
 */
bool Instrumentor::getGlobal(const Thread& thread,
			     const std::string& global,
			     std::string& value)
{
    ThreadGroup threads;
    threads.insert(thread);

    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the global variable be retrieved from the thread
    Senders::getGlobalString(thread, global);

    // TODO: wait for response and return it

    return false;  // TEMPORARY DUMMY RETURN
}



/**
 * Get value of the MPICH process table from a thread.
 *
 * Gets the current value of the MPICH process table within this thread. Used
 * to obtain this information for the purposes of attaching to an entire MPI
 * job.
 *
 * @pre    Only applies to a thread which is connected. A ThreadUnavailable
 *         exception is thrown if called for a thread that is not connected.
 *
 * @param thread    Thread from which the MPICH process table should be
 *                  retrieved. 
 * @retval value    Current value of the MPICH process table.
 * @return          Boolean "true" if the MPICH process table was successfully
 *                  retrieved, "false" otherwise.
 */
bool Instrumentor::getMPICHProcTable(const Thread& thread, Job& value)
{
    ThreadGroup threads;
    threads.insert(thread);

    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the MPICH process table be retrieved from the thread
    Senders::getMPICHProcTable(thread);

    // TODO: wait for reaponse and return it

    return false;  // TEMPORARY DUMMY RETURN
}



/**
 * Set value of an integer global variable in a thread.
 *
 * Sets the value of a signed integer global variable within this thread.
 * Used to modify certain values, such as MPI debug gates, in a process.
 *
 * @pre    Only applies to a process which is connected. A ProcessUnavailable
 *         exception is thrown if called for a thread that is not connected.
 *
 * @param thread    Thread in which the global variable value should be set.
 * @param global    Name of global variable whose value is being set.
 * @param value     New value of that variable.
 */
void Instrumentor::setGlobal(const Thread& thread,
			     const std::string& global,
			     int64_t value)
{
    ThreadGroup threads;
    threads.insert(thread);

    // Check preconditions
    ThreadTable::TheTable.validateThreads(threads);

    // Request the global variable be set in the process
    Senders::setGlobalInteger(thread, global, value);
}
