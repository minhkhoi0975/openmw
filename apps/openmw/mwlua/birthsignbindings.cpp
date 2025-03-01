#include <components/esm3/loadbsgn.hpp>
#include <components/lua/luastate.hpp>
#include <components/misc/resourcehelpers.hpp>
#include <components/resource/resourcesystem.hpp>

#include "../mwbase/environment.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"

#include "birthsignbindings.hpp"
#include "luamanagerimp.hpp"
#include "types/types.hpp"

namespace sol
{
    template <>
    struct is_automagical<ESM::BirthSign> : std::false_type
    {
    };
    template <>
    struct is_automagical<MWWorld::Store<ESM::BirthSign>> : std::false_type
    {
    };
}

namespace MWLua
{
    sol::table initBirthSignRecordBindings(const Context& context)
    {
        sol::state_view& lua = context.mLua->sol();
        sol::table birthSigns(context.mLua->sol(), sol::create);
        addRecordFunctionBinding<ESM::BirthSign>(birthSigns, context);

        auto signT = lua.new_usertype<ESM::BirthSign>("ESM3_BirthSign");
        signT[sol::meta_function::to_string] = [](const ESM::BirthSign& rec) -> std::string {
            return "ESM3_BirthSign[" + rec.mId.toDebugString() + "]";
        };
        signT["id"] = sol::readonly_property([](const ESM::BirthSign& rec) { return rec.mId.serializeText(); });
        signT["name"] = sol::readonly_property([](const ESM::BirthSign& rec) -> std::string_view { return rec.mName; });
        signT["description"]
            = sol::readonly_property([](const ESM::BirthSign& rec) -> std::string_view { return rec.mDescription; });
        auto vfs = MWBase::Environment::get().getResourceSystem()->getVFS();
        signT["texture"] = sol::readonly_property([vfs](const ESM::BirthSign& rec) -> std::string {
            return Misc::ResourceHelpers::correctTexturePath(rec.mTexture, vfs);
        });
        signT["spells"] = sol::readonly_property([lua](const ESM::BirthSign& rec) -> sol::table {
            sol::table res(lua, sol::create);
            for (size_t i = 0; i < rec.mPowers.mList.size(); ++i)
                res[i + 1] = rec.mPowers.mList[i].serializeText();
            return res;
        });

        return LuaUtil::makeReadOnly(birthSigns);
    }
}
