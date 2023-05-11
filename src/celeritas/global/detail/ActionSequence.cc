//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/global/detail/ActionSequence.cc
//---------------------------------------------------------------------------//
#include "ActionSequence.hh"

#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>

#include "corecel/device_runtime_api.h"
#include "corecel/Types.hh"
#include "corecel/cont/EnumArray.hh"
#include "corecel/cont/Range.hh"
#include "corecel/sys/Stopwatch.hh"
#include "celeritas/global/ActionInterface.hh"

#include "../ActionRegistry.hh"

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Construct from an action registry and sequence options.
 */
ActionSequence::ActionSequence(ActionRegistry const& reg, Options options)
    : options_(std::move(options))
{
    // Loop over all action IDs
    for (auto aidx : range(reg.num_actions()))
    {
        // Get abstract action shared pointer and see if it's explicit
        auto const& base = reg.action(ActionId{aidx});
        if (auto expl
            = std::dynamic_pointer_cast<ExplicitActionInterface const>(base))
        {
            // Add explicit action to our array
            actions_.push_back(std::move(expl));
        }
    }

    // Sort actions by increasing order (and secondarily, increasing IDs)
    std::sort(actions_.begin(),
              actions_.end(),
              [](SPConstExplicit const& a, SPConstExplicit const& b) {
                  return std::make_tuple(a->order(), a->action_id())
                         < std::make_tuple(b->order(), b->action_id());
              });

    // Initialize timing
    accum_time_.resize(actions_.size());

    CELER_ENSURE(actions_.size() == accum_time_.size());
}

//---------------------------------------------------------------------------//
/*!
 * Call the given action ID with host or device data.
 *
 * The given action ID \em must be an explicit action.
 */
template<MemSpace M>
void ActionSequence::execute(CoreParams const& params, CoreState<M>& state)
{
    if (M == MemSpace::host || options_.sync)
    {
        // Execute all actions and record the time elapsed
        for (auto i : range(actions_.size()))
        {
            Stopwatch get_time;
            actions_[i]->execute(params, state);
            if (M == MemSpace::device)
            {
                CELER_DEVICE_CALL_PREFIX(DeviceSynchronize());
            }
            accum_time_[i] += get_time();
        }
    }
    else
    {
        // Just loop over the actions
        for (SPConstExplicit const& sp_action : actions_)
        {
            sp_action->execute(params, state);
        }
    }
}

//---------------------------------------------------------------------------//
// Explicit template instantiation
//---------------------------------------------------------------------------//

template void
ActionSequence::execute(CoreParams const&, CoreState<MemSpace::host>&);
template void
ActionSequence::execute(CoreParams const&, CoreState<MemSpace::device>&);

//---------------------------------------------------------------------------//
}  // namespace detail
}  // namespace celeritas
