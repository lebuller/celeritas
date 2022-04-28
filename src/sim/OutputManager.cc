//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file OutputManager.cc
//---------------------------------------------------------------------------//
#include "OutputManager.hh"

#include "celeritas_config.h"
#if CELERITAS_USE_JSON
#    include <nlohmann/json.hpp>
#endif

#include "base/Assert.hh"
#include "base/Range.hh"
#include "comm/Logger.hh"

#include "JsonPimpl.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
/*!
 * Add an interface for writing.
 */
void OutputManager::insert(SPConstInterface interface)
{
    CELER_EXPECT(interface);
    CELER_EXPECT(interface->category() != Category::size_);

    auto label = interface->label();
    CELER_VALIDATE(!label.empty(), << "empty label for output interface");

    Category cat = interface->category();
    auto     iter_inserted
        = interfaces_[cat].insert({std::move(label), std::move(interface)});
    CELER_VALIDATE(iter_inserted.second,
                   << "duplicate output entry '" << iter_inserted.first->first
                   << "' for category '" << to_cstring(cat) << "'");
}

//---------------------------------------------------------------------------//
/*!
 * Output all classes to a JSON object that's written to the given stream.
 */
void OutputManager::output(std::ostream* os) const
{
#if CELERITAS_USE_JSON
    nlohmann::json result;

    for (auto cat : range(Category::size_))
    {
        nlohmann::json cat_result;
        for (const auto& kv : interfaces_[cat])
        {
            // Hack for inlining input/result outputs in Transporter: to be
            // removed when the individual interfaces are converted to
            // OutputInterface
            bool      is_global = (kv.first == "*");
            JsonPimpl json_wrap;
            if (is_global)
            {
                json_wrap.obj = std::move(cat_result);
            }
            kv.second->output(&json_wrap);
            if (is_global)
            {
                cat_result = std::move(json_wrap.obj);
            }
            else
            {
                cat_result[kv.first] = std::move(json_wrap.obj);
            }
        }
        if (!cat_result.empty())
        {
            // Add category to the final output
            result[to_cstring(cat)] = std::move(cat_result);
        }
    }

    *os << result.dump();
#else
    // Write a JSON-compatible string showing why output is unavailable
    CELER_LOG(error) << "Cannot write output to JSON: nljson is not enabled "
                        "in the current build configuration";
    *os << "\"output unavailable\"";
#endif
}

//---------------------------------------------------------------------------//
} // namespace celeritas