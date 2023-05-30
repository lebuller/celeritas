//----------------------------------*-C++-*----------------------------------//
// Copyright 2022-2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/geo/generated/BoundaryAction.cc
//! \note Auto-generated by gen-action.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "BoundaryAction.hh"

#include <utility>

#include "corecel/Types.hh"
#include "celeritas/global/LaunchAction.hh"
#include "celeritas/global/TrackExecutor.hh"
#include "../detail/BoundaryActionImpl.hh" // IWYU pragma: associated

namespace celeritas
{
namespace generated
{
//---------------------------------------------------------------------------//
/*!
 * Launch the boundary action on host.
 */
void BoundaryAction::execute(CoreParams const& params, CoreStateHost& state) const
{
    return ::celeritas::launch_action(
        *this, params, state,
        TrackExecutor{*params.ptr<MemSpace::native>(), *state.ptr(),
                      detail::boundary_track});
}

//---------------------------------------------------------------------------//
}  // namespace generated
}  // namespace celeritas
