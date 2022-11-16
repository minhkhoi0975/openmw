#include "referenceinterface.hpp"

#include <type_traits>

#include <apps/openmw/mwworld/ptr.hpp>

namespace MWGui
{
    ReferenceInterface::ReferenceInterface() {}

    ReferenceInterface::~ReferenceInterface() {}

    void ReferenceInterface::checkReferenceAvailable()
    {
        // check if count of the reference has become 0
        if (!mPtr.isEmpty() && mPtr.getRefData().getCount() == 0)
        {
            mPtr = MWWorld::Ptr();
            onReferenceUnavailable();
        }
    }
}
