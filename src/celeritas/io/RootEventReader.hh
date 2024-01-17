//----------------------------------*-C++-*----------------------------------//
// Copyright 2023-2024 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/io/RootEventReader.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>

#include "corecel/Macros.hh"
#include "celeritas/ext/RootUniquePtr.hh"
#include "celeritas/phys/Primary.hh"

#include "EventIOInterface.hh"

namespace celeritas
{
class ParticleParams;

//---------------------------------------------------------------------------//
/*!
 * Read ROOT file generated by \c RootEventWriter .
 *
 * Each \c operator() call returns a vector of primaries from a single event.
 * \code
    RootEventReader read("primaries.root", particle_params);
    RootEventReader::result_type event;
    while (event = read(), !event.empty())
    {
        // Do stuff
    }
 * \endcode
 */
class RootEventReader : public EventReaderInterface
{
  public:
    //!@{
    //! \name Type aliases
    using SPConstParticles = std::shared_ptr<ParticleParams const>;
    using result_type = std::vector<Primary>;
    //!@}

    // Construct with ROOT filename
    RootEventReader(std::string const& filename, SPConstParticles params);

    //! Prevent copying and moving
    CELER_DELETE_COPY_MOVE(RootEventReader);

    // Read a single event from the ROOT file
    result_type operator()() final;

    //! Get total number of events
    size_type num_events() const final { return num_events_; }

  private:
    //// DATA ////

    SPConstParticles params_;
    std::size_t entry_count_{0};  // Current TTree entry
    std::size_t num_entries_;  // Total number of entries in the TTree
    size_type num_events_;  // Total number of events
    UPExtern<TFile> tfile_;
    UPExtern<TTree> ttree_;

    //// HELPER FUNCTIONS ////

    // Hardcoded ROOT TTree name defined by RootEventWriter
    char const* tree_name() { return "primaries"; }

    // Fetch basic data types from leaves
    template<class T>
    auto from_leaf(char const* leaf_name) -> T;

    // Fetch arrays from leaves
    Real3 from_array_leaf(char const* leaf_name);
};

//---------------------------------------------------------------------------//
#if !CELERITAS_USE_ROOT
inline RootEventReader::RootEventReader(std::string const&, SPConstParticles)
{
    CELER_DISCARD(params_);
    CELER_DISCARD(entry_count_);
    CELER_DISCARD(num_entries_);
    CELER_DISCARD(num_events_);
    CELER_DISCARD(tfile_);
    CELER_DISCARD(ttree_);
    CELER_NOT_CONFIGURED("ROOT");
}

inline RootEventReader::result_type RootEventReader::operator()()
{
    CELER_ASSERT_UNREACHABLE();
}
#endif

//---------------------------------------------------------------------------//
}  // namespace celeritas
