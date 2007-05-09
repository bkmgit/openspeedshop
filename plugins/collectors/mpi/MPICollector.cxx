////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
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
 * Definition of the MPICollector class.
 *
 */
 
#include "MPICollector.hxx"
#include "MPIDetail.hxx"
#include "blobs.h"

using namespace OpenSpeedShop::Framework;



namespace {

    /** Type returned for the MPI call time metrics. */
    typedef std::map<StackTrace, std::vector<double> > CallTimes;

    /** Type returned for the MPI call detail metrics. */
    typedef std::map<StackTrace, std::vector<MPIDetail> > CallDetails;
    
    

    /**
     * Traceable function table.
     *
     * Table listing the traceable MPI functions. In order for an MPI function
     * to actually be traceable, corresponding wrapper(s) must first be written
     * and compiled into this collector's runtime.
     *
     * @note    A function's index position in this table is directly used as
     *          its index position in the mpi_parameters.traced array. Thus
     *          the order the functions are listed here is significant. If it
     *          is changed, users will find that any saved databases suddenly
     *          trace different MPI functions than they did previously.
     */
    const char* TraceableFunctions[] = {

        "MPI_Allgather",
        "MPI_Allgatherv",
        "MPI_Allreduce",
        "MPI_Alltoall",
        "MPI_Alltoallv",
        "MPI_Barrier",
        "MPI_Bcast",
        "MPI_Bsend",
        "MPI_Cancel",
        "MPI_Finalize",
        "MPI_Gather",
        "MPI_Gatherv",
        "MPI_Get_count",
        "MPI_Ibsend",
        "MPI_Init",
        "MPI_Irecv",
        "MPI_Irsend",
        "MPI_Isend",
        "MPI_Issend",
        "MPI_Pack",
        "MPI_Probe",
        "MPI_Recv",
        "MPI_Reduce",
        "MPI_Reduce_scatter",
        "MPI_Request_free",
        "MPI_Rsend",
        "MPI_Scan",
        "MPI_Scatter",
        "MPI_Scatterv",
        "MPI_Send",
        "MPI_Sendrecv",
        "MPI_Sendrecv_replace",
        "MPI_Ssend",
        "MPI_Test",
        "MPI_Testall",
        "MPI_Testany",
        "MPI_Testsome",
        "MPI_Unpack",
        "MPI_Wait",
        "MPI_Waitall",
        "MPI_Waitany",
        "MPI_Waitsome",
	
	// End Of Table Entry
	NULL
    };

    std::string MPI_IMPL_NAME;
    
}    



/**
 * Collector's factory method.
 *
 * Factory method for instantiating a collector implementation. This is the
 * only function that is externally visible from outside the collector plugin.
 *
 * @return    New instance of this collector's implementation.
 */
extern "C" CollectorImpl* mpi_LTX_CollectorFactory()
{
    return new MPICollector();
}



/**
 * Default constructor.
 *
 * Constructs a new MPI collector with the proper metadata.
 */
MPICollector::MPICollector() :
    CollectorImpl("mpi",
                  "MPI Extended Event Tracing",
		  "The MPI experiment intercepts all calls to MPI functions that are specified "
		  "by the user.  The functions monitored are usually those that are doing a "
                  "significant amount of work, primarily those that send "
		  "messages. The MPI experiment records the following for each call: "
		  " the current stack trace, the start time and the end time. "
                  "The following detailed ancillary data is also stored: "
		  " the source and destination rank, the number of bytes sent," 
                  " the message tag, the communicator used, the message data type,"
                  " and other ancillary items not specified here.")
{
    // Declare our parameters
    declareParameter(Metadata("traced_functions", "Traced Functions",
			      "Set of MPI functions to be traced.",
			      typeid(std::map<std::string, bool>)));
    
    // Declare our metrics
    declareMetric(Metadata("time", "MPI Call Time",
			   "Exclusive MPI call time in seconds.",
			   typeid(double)));
    declareMetric(Metadata("inclusive_times", "Inclusive Times",
			   "Inclusive MPI call times in seconds.",
			   typeid(CallTimes)));
    declareMetric(Metadata("exclusive_times", "Exclusive Times",
			   "Exclusive MPI call times in seconds.",
			   typeid(CallTimes)));
    declareMetric(Metadata("inclusive_details", "Inclusive Details",
			   "Inclusive MPI call details.",
			   typeid(CallDetails)));
    declareMetric(Metadata("exclusive_details", "Exclusive Details",
			   "Exclusive MPI call details.",
			   typeid(CallDetails)));
}



/**
 * Get the default parameter values.
 *
 * Implement getting our default parameter values.
 *
 * @return    Blob containing the default parameter values.
 */
Blob MPICollector::getDefaultParameterValues() const
{
    // Setup an empty parameter structure    
    mpi_parameters parameters;
    memset(&parameters, 0, sizeof(parameters));

    // Set the default parameters
    for(unsigned i = 0; TraceableFunctions[i] != NULL; ++i)
	parameters.traced[i] = true;
    
    // Return the encoded blob to the caller
    return Blob(reinterpret_cast<xdrproc_t>(xdr_mpi_parameters), &parameters);
}



/**
 * Get a parameter value.
 *
 * Implement getting one of our parameter values.
 *
 * @param parameter    Unique identifier of the parameter.
 * @param data         Blob containing the parameter values.
 * @retval ptr         Untyped pointer to the parameter value.
 */
void MPICollector::getParameterValue(const std::string& parameter,
				      const Blob& data, void* ptr) const
{
    // Decode the blob containing the parameter values
    mpi_parameters parameters;
    memset(&parameters, 0, sizeof(parameters));
    data.getXDRDecoding(reinterpret_cast<xdrproc_t>(xdr_mpi_parameters),
                        &parameters);

    // Handle the "traced_functions" parameter
    if(parameter == "traced_functions") {
	std::map<std::string, bool>* value =
	    reinterpret_cast<std::map<std::string, bool>*>(ptr);    
        for(unsigned i = 0; TraceableFunctions[i] != NULL; ++i)
	    value->insert(std::make_pair(TraceableFunctions[i],
					  parameters.traced[i]));
    }
}



/**
 * Set a parameter value.
 *
 * Implements setting one of our parameter values.
 *
 * @param parameter    Unique identifier of the parameter.
 * @param ptr          Untyped pointer to the parameter value.
 * @retval data        Blob containing the parameter values.
 */
void MPICollector::setParameterValue(const std::string& parameter,
				      const void* ptr, Blob& data) const
{
    // Decode the blob containing the parameter values
    mpi_parameters parameters;
    memset(&parameters, 0, sizeof(parameters));
    data.getXDRDecoding(reinterpret_cast<xdrproc_t>(xdr_mpi_parameters),
                        &parameters);
    
    // Handle the "traced_functions" parameter
    if(parameter == "traced_functions") {
	const std::map<std::string, bool>* value = 
	    reinterpret_cast<const std::map<std::string, bool>*>(ptr);
	for(unsigned i = 0; TraceableFunctions[i] != NULL; ++i)
	    parameters.traced[i] =
		(value->find(TraceableFunctions[i]) != value->end()) &&
		value->find(TraceableFunctions[i])->second;
    }
    
    // Re-encode the blob containing the parameter values
    data = Blob(reinterpret_cast<xdrproc_t>(xdr_mpi_parameters), &parameters);
}



/**
 * Start data collection.
 *
 * Implement starting data collection for a particular thread.
 *
 * @param collector    Collector starting data collection.
 * @param thread       Thread for which to start collecting data.
 */
void MPICollector::startCollecting(const Collector& collector,
				    const Thread& thread) const
{
    // Get the set of traced functions for this collector
    std::map<std::string, bool> traced;
    collector.getParameterValue("traced_functions", traced);
    
    // Assemble and encode arguments to mpi_start_tracing()
    mpi_start_tracing_args args;
    memset(&args, 0, sizeof(args));
    getECT(collector, thread, args.experiment, args.collector, args.thread);
    Blob arguments(reinterpret_cast<xdrproc_t>(xdr_mpi_start_tracing_args),
                   &args);

    MPI_IMPL_NAME = getRuntimeLibraryName(thread);
    // Execute mpi_stop_tracing() before we exit the thread
    executeBeforeExit(collector, thread,
		      MPI_IMPL_NAME + ": mpi_stop_tracing",
		      Blob());
    
    // Execute mpi_start_tracing() in the thread
    executeNow(collector, thread,
               MPI_IMPL_NAME + ": mpi_start_tracing",
	       arguments);

    // Execute our wrappers in place of the real MPI functions
    for(unsigned i = 0; TraceableFunctions[i] != NULL; ++i)	
	if((traced.find(TraceableFunctions[i]) != traced.end()) &&
	   traced.find(TraceableFunctions[i])->second) {
	    
	    std::string P_name = std::string("P") + TraceableFunctions[i];

	    // Wrap the MPI function
	    executeInPlaceOf(
		collector, thread,
		P_name, MPI_IMPL_NAME + ": mpi_" + P_name
		);
	    
	}
}



/**
 * Stops data collection.
 *
 * Implement stopping data collection for a particular thread.
 *
 * @param collector    Collector stopping data collection.
 * @param thread       Thread for which to stop collecting data.
 */
void MPICollector::stopCollecting(const Collector& collector,
				   const Thread& thread) const
{
    // Execute mpi_stop_tracing() in the thread
    executeNow(collector, thread,
               MPI_IMPL_NAME + ": mpi_stop_tracing", Blob());
    
    // Remove all instrumentation associated with this collector/thread pairing
    uninstrument(collector, thread);
}



/**
 * Get metric values.
 *
 * Implements getting one of this collector's metric values over all subextents
 * of the specified extent for a particular thread, for one of the collected
 * performance data blobs.
 *
 * @param metric        Unique identifier of the metric.
 * @param collector     Collector for which to get values.
 * @param thread        Thread for which to get values.
 * @param extent        Extent of the performance data blob.
 * @param blob          Blob containing the performance data.
 * @param subextents    Subextents for which to get values.
 * @retval ptr          Untyped pointer to the values of the metric.
 */
void MPICollector::getMetricValues(const std::string& metric,
				   const Collector& collector,
				   const Thread& thread,
				   const Extent& extent,
				   const Blob& blob,
				   const ExtentGroup& subextents,
				   void* ptr) const
{
    // Determine which metric was specified
    bool is_time = (metric == "time");
    bool is_inclusive_times = (metric == "inclusive_times");
    bool is_exclusive_times = (metric == "exclusive_times");
    bool is_inclusive_details = (metric == "inclusive_details");
    bool is_exclusive_details = (metric == "exclusive_details");

    // Don't return anything if an invalid metric was specified
    if(!is_time &&
       !is_inclusive_times && !is_exclusive_times &&
       !is_inclusive_details && !is_exclusive_details)
	return;
     
    // Check assertions
    if(is_time) {
	Assert(reinterpret_cast<std::vector<double>*>(ptr)->size() >=
	       subextents.size());
    }
    else if(is_inclusive_times || is_exclusive_times) {
	Assert(reinterpret_cast<std::vector<CallTimes>*>(ptr)->size() >=
	       subextents.size());
    }
    else if(is_inclusive_details || is_exclusive_details) {
	Assert(reinterpret_cast<std::vector<CallDetails>*>(ptr)->size() >=
	       subextents.size());
    }

    // Decode this data blob
    mpi_data data;
    memset(&data, 0, sizeof(data));
    blob.getXDRDecoding(reinterpret_cast<xdrproc_t>(xdr_mpi_data), &data);
    
    // Iterate over each of the events
    for(unsigned i = 0; i < data.events.events_len; ++i) {

	// Get the time interval attributable to this event
	TimeInterval interval(Time(data.events.events_val[i].start_time),
			      Time(data.events.events_val[i].stop_time));

	// Get the stack trace for this event
	StackTrace trace(thread, interval.getBegin());
	for(unsigned j = data.events.events_val[i].stacktrace;
	    data.stacktraces.stacktraces_val[j] != 0;
	    ++j)
	    trace.push_back(Address(data.stacktraces.stacktraces_val[j]));
	
	// Iterate over each of the frames in this event's stack trace
	for(StackTrace::const_iterator 
		j = trace.begin(); j != trace.end(); ++j) {

	    // Stop after the first frame if this is "exclusive" anything
	    if((is_time || is_exclusive_times || is_exclusive_details) &&
	       (j != trace.begin()))
		break;
	    
	    // Find the subextents that contain this frame
	    std::set<ExtentGroup::size_type> intersection =
		subextents.getIntersectionWith(
		    Extent(interval, AddressRange(*j))
		    );
	    
	    // Iterate over each subextent in the intersection
	    for(std::set<ExtentGroup::size_type>::const_iterator
		    k = intersection.begin(); k != intersection.end(); ++k) {
		
		// Calculate intersection time (in nS) of subextent and event
		double t_intersection = static_cast<double>
		    ((interval & subextents[*k].getTimeInterval()).getWidth());

		// Add this event to the results for this subextent
		if(is_time) {

		    // Add this event's time (in seconds) to the results
		    (*reinterpret_cast<std::vector<double>*>(ptr))[*k] +=
			t_intersection / 1000000000.0;
		    
		}
		else if(is_inclusive_times || is_exclusive_times) {

		    // Find this event's stack trace in the results (or add it)
		    CallTimes::iterator l =
			(*reinterpret_cast<std::vector<CallTimes>*>(ptr))
			[*k].insert(
			    std::make_pair(trace, std::vector<double>())
			    ).first;

		    // Add this event's time (in seconds) to the results
		    l->second.push_back(t_intersection / 1000000000.0);

		}
		else if(is_inclusive_details || is_exclusive_details) {

		    // Find this event's stack trace in the results (or add it)
		    CallDetails::iterator l =
			(*reinterpret_cast<std::vector<CallDetails>*>(ptr))
			[*k].insert(
			    std::make_pair(trace, std::vector<MPIDetail>())
			    ).first;
		    
		    // Add this event's details structure to the results
		    MPIDetail details;
		    details.dm_interval = interval;
		    details.dm_time = t_intersection / 1000000000.0;
		    l->second.push_back(details);
		    
		}
		
	    }
	    
	}

    }

    // Free the decoded data blob
    xdr_free(reinterpret_cast<xdrproc_t>(xdr_mpi_data),
	     reinterpret_cast<char*>(&data));
}



std::string MPICollector::getRuntimeLibraryName(const Thread& thread) const
{
    /*
     * A cache could be maintained here to avoid repeated calls to
     * CollectorImpl::getMPIImplementationName() for the same threads.
     */
    std::string mpi_implementation_name = getMPIImplementationName(thread);

    return (std::string("mpi-rt-") + mpi_implementation_name);
}

