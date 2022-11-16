#include "widgets.hpp"

#include <iomanip>
#include <ostream>
#include <stdexcept>

#include <MyGUI_Align.h>
#include <MyGUI_Button.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_LogStream.h>
#include <MyGUI_ProgressBar.h>
#include <MyGUI_StringUtility.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_UString.h>
#include <MyGUI_WidgetInput.h>

#include "MyGUI_LogManager.h"

#include <apps/openmw/mwmechanics/stat.hpp>
#include <apps/openmw/mwworld/store.hpp>

#include <components/esm/attr.hpp>
#include <components/esm/defs.hpp>
#include <components/esm3/loadmgef.hpp>
#include <components/esm3/loadspel.hpp>
#include <components/misc/notnullptr.hpp>
#include <components/misc/resourcehelpers.hpp>
#include <components/resource/resourcesystem.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwworld/esmstore.hpp"

#include "ustring.hpp"

namespace MWGui::Widgets
{
    /* MWSkill */

    MWSkill::MWSkill()
        : mSkillId(ESM::Skill::Length)
        , mSkillNameWidget(nullptr)
        , mSkillValueWidget(nullptr)
    {
    }

    void MWSkill::setSkillId(ESM::Skill::SkillEnum skill)
    {
        mSkillId = skill;
        updateWidgets();
    }

    void MWSkill::setSkillNumber(int skill)
    {
        if (skill < 0)
            setSkillId(ESM::Skill::Length);
        else if (skill < ESM::Skill::Length)
            setSkillId(static_cast<ESM::Skill::SkillEnum>(skill));
        else
            throw std::runtime_error("Skill number out of range");
    }

    void MWSkill::setSkillValue(const SkillValue& value)
    {
        mValue = value;
        updateWidgets();
    }

    void MWSkill::updateWidgets()
    {
        if (mSkillNameWidget)
        {
            if (mSkillId == ESM::Skill::Length)
            {
                mSkillNameWidget->setCaption("");
            }
            else
            {
                MyGUI::UString name = toUString(MWBase::Environment::get().getWindowManager()->getGameSettingString(
                    ESM::Skill::sSkillNameIds[mSkillId], {}));
                mSkillNameWidget->setCaption(name);
            }
        }
        if (mSkillValueWidget)
        {
            SkillValue::Type modified = mValue.getModified(), base = mValue.getBase();
            mSkillValueWidget->setCaption(MyGUI::utility::toString(modified));
            if (modified > base)
                mSkillValueWidget->_setWidgetState("increased");
            else if (modified < base)
                mSkillValueWidget->_setWidgetState("decreased");
            else
                mSkillValueWidget->_setWidgetState("normal");
        }
    }

    void MWSkill::onClicked(MyGUI::Widget* _sender)
    {
        eventClicked(this);
    }

    MWSkill::~MWSkill() {}

    void MWSkill::initialiseOverride()
    {
        Base::initialiseOverride();

        assignWidget(mSkillNameWidget, "StatName");
        assignWidget(mSkillValueWidget, "StatValue");

        MyGUI::Button* button;
        assignWidget(button, "StatNameButton");
        if (button)
        {
            mSkillNameWidget = button;
            button->eventMouseButtonClick += MyGUI::newDelegate(this, &MWSkill::onClicked);
        }

        button = nullptr;
        assignWidget(button, "StatValueButton");
        if (button)
        {
            mSkillValueWidget = button;
            button->eventMouseButtonClick += MyGUI::newDelegate(this, &MWSkill::onClicked);
        }
    }

    /* MWAttribute */

    MWAttribute::MWAttribute()
        : mId(-1)
        , mAttributeNameWidget(nullptr)
        , mAttributeValueWidget(nullptr)
    {
    }

    void MWAttribute::setAttributeId(int attributeId)
    {
        mId = attributeId;
        updateWidgets();
    }

    void MWAttribute::setAttributeValue(const AttributeValue& value)
    {
        mValue = value;
        updateWidgets();
    }

    void MWAttribute::onClicked(MyGUI::Widget* _sender)
    {
        eventClicked(this);
    }

    void MWAttribute::updateWidgets()
    {
        if (mAttributeNameWidget)
        {
            if (mId < 0 || mId >= 8)
            {
                mAttributeNameWidget->setCaption("");
            }
            else
            {
                static const std::string_view attributes[8]
                    = { "sAttributeStrength", "sAttributeIntelligence", "sAttributeWillpower", "sAttributeAgility",
                          "sAttributeSpeed", "sAttributeEndurance", "sAttributePersonality", "sAttributeLuck" };
                MyGUI::UString name = toUString(
                    MWBase::Environment::get().getWindowManager()->getGameSettingString(attributes[mId], {}));
                mAttributeNameWidget->setCaption(name);
            }
        }
        if (mAttributeValueWidget)
        {
            int modified = mValue.getModified(), base = mValue.getBase();
            mAttributeValueWidget->setCaption(MyGUI::utility::toString(modified));
            if (modified > base)
                mAttributeValueWidget->_setWidgetState("increased");
            else if (modified < base)
                mAttributeValueWidget->_setWidgetState("decreased");
            else
                mAttributeValueWidget->_setWidgetState("normal");
        }
    }

    MWAttribute::~MWAttribute() {}

    void MWAttribute::initialiseOverride()
    {
        Base::initialiseOverride();

        assignWidget(mAttributeNameWidget, "StatName");
        assignWidget(mAttributeValueWidget, "StatValue");

        MyGUI::Button* button;
        assignWidget(button, "StatNameButton");
        if (button)
        {
            mAttributeNameWidget = button;
            button->eventMouseButtonClick += MyGUI::newDelegate(this, &MWAttribute::onClicked);
        }

        button = nullptr;
        assignWidget(button, "StatValueButton");
        if (button)
        {
            mAttributeValueWidget = button;
            button->eventMouseButtonClick += MyGUI::newDelegate(this, &MWAttribute::onClicked);
        }
    }

    /* MWSpell */

    MWSpell::MWSpell()
        : mSpellNameWidget(nullptr)
    {
    }

    void MWSpell::setSpellId(const ESM::RefId& spellId)
    {
        mId = spellId;
        updateWidgets();
    }

    void MWSpell::createEffectWidgets(
        std::vector<MyGUI::Widget*>& effects, MyGUI::Widget* creator, MyGUI::IntCoord& coord, int flags)
    {
        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();

        const ESM::Spell* spell = store.get<ESM::Spell>().search(mId);
        MYGUI_ASSERT(spell, "spell with id '" << mId << "' not found");

        for (const ESM::ENAMstruct& effectInfo : spell->mEffects.mList)
        {
            MWSpellEffectPtr effect
                = creator->createWidget<MWSpellEffect>("MW_EffectImage", coord, MyGUI::Align::Default);
            SpellEffectParams params;
            params.mEffectID = effectInfo.mEffectID;
            params.mSkill = effectInfo.mSkill;
            params.mAttribute = effectInfo.mAttribute;
            params.mDuration = effectInfo.mDuration;
            params.mMagnMin = effectInfo.mMagnMin;
            params.mMagnMax = effectInfo.mMagnMax;
            params.mRange = effectInfo.mRange;
            params.mIsConstant = (flags & MWEffectList::EF_Constant) != 0;
            params.mNoTarget = (flags & MWEffectList::EF_NoTarget);
            params.mNoMagnitude = (flags & MWEffectList::EF_NoMagnitude);
            effect->setSpellEffect(params);
            effects.push_back(effect);
            coord.top += effect->getHeight();
            coord.width = std::max(coord.width, effect->getRequestedWidth());
        }
    }

    void MWSpell::updateWidgets()
    {
        if (mSpellNameWidget && MWBase::Environment::get().getWindowManager())
        {
            const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();

            const ESM::Spell* spell = store.get<ESM::Spell>().search(mId);
            if (spell)
                mSpellNameWidget->setCaption(spell->mName);
            else
                mSpellNameWidget->setCaption("");
        }
    }

    void MWSpell::initialiseOverride()
    {
        Base::initialiseOverride();

        assignWidget(mSpellNameWidget, "StatName");
    }

    MWSpell::~MWSpell() {}

    /* MWEffectList */

    MWEffectList::MWEffectList()
        : mEffectList(0)
    {
    }

    void MWEffectList::setEffectList(const SpellEffectList& list)
    {
        mEffectList = list;
        updateWidgets();
    }

    void MWEffectList::createEffectWidgets(
        std::vector<MyGUI::Widget*>& effects, MyGUI::Widget* creator, MyGUI::IntCoord& coord, bool center, int flags)
    {
        // We don't know the width of all the elements beforehand, so we do it in
        // 2 steps: first, create all widgets and check their width....
        MWSpellEffectPtr effect = nullptr;
        int maxwidth = coord.width;

        for (auto& effectInfo : mEffectList)
        {
            effect = creator->createWidget<MWSpellEffect>("MW_EffectImage", coord, MyGUI::Align::Default);
            effectInfo.mIsConstant = (flags & EF_Constant) || effectInfo.mIsConstant;
            effectInfo.mNoTarget = (flags & EF_NoTarget) || effectInfo.mNoTarget;
            effectInfo.mNoMagnitude = (flags & EF_NoMagnitude) || effectInfo.mNoMagnitude;
            effect->setSpellEffect(effectInfo);
            effects.push_back(effect);
            if (effect->getRequestedWidth() > maxwidth)
                maxwidth = effect->getRequestedWidth();

            coord.top += effect->getHeight();
        }

        // ... then adjust the size for all widgets
        for (MyGUI::Widget* effectWidget : effects)
        {
            effect = effectWidget->castType<MWSpellEffect>();
            bool needcenter = center && (maxwidth > effect->getRequestedWidth());
            int diff = maxwidth - effect->getRequestedWidth();
            if (needcenter)
            {
                effect->setCoord(
                    diff / 2, effect->getCoord().top, effect->getRequestedWidth(), effect->getCoord().height);
            }
            else
            {
                effect->setCoord(0, effect->getCoord().top, effect->getRequestedWidth(), effect->getCoord().height);
            }
        }

        // inform the parent about width
        coord.width = maxwidth;
    }

    void MWEffectList::updateWidgets() {}

    void MWEffectList::initialiseOverride()
    {
        Base::initialiseOverride();
    }

    MWEffectList::~MWEffectList() {}

    SpellEffectList MWEffectList::effectListFromESM(const ESM::EffectList* effects)
    {
        SpellEffectList result;
        for (const ESM::ENAMstruct& effectInfo : effects->mList)
        {
            SpellEffectParams params;
            params.mEffectID = effectInfo.mEffectID;
            params.mSkill = effectInfo.mSkill;
            params.mAttribute = effectInfo.mAttribute;
            params.mDuration = effectInfo.mDuration;
            params.mMagnMin = effectInfo.mMagnMin;
            params.mMagnMax = effectInfo.mMagnMax;
            params.mRange = effectInfo.mRange;
            params.mArea = effectInfo.mArea;
            result.push_back(params);
        }
        return result;
    }

    /* MWSpellEffect */

    MWSpellEffect::MWSpellEffect()
        : mImageWidget(nullptr)
        , mTextWidget(nullptr)
        , mRequestedWidth(0)
    {
    }

    void MWSpellEffect::setSpellEffect(const SpellEffectParams& params)
    {
        mEffectParams = params;
        updateWidgets();
    }

    void MWSpellEffect::updateWidgets()
    {
        if (!mEffectParams.mKnown)
        {
            mTextWidget->setCaption("?");
            mTextWidget->setCoord(sIconOffset / 2, mTextWidget->getCoord().top, mTextWidget->getCoord().width,
                mTextWidget->getCoord().height); // Compensates for the missing image when effect is not known
            mRequestedWidth = mTextWidget->getTextSize().width + sIconOffset;
            mImageWidget->setImageTexture("");
            return;
        }

        const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();

        const ESM::MagicEffect* magicEffect = store.get<ESM::MagicEffect>().search(mEffectParams.mEffectID);

        assert(magicEffect);

        std::string_view pt = MWBase::Environment::get().getWindowManager()->getGameSettingString("spoint", {});
        std::string_view pts = MWBase::Environment::get().getWindowManager()->getGameSettingString("spoints", {});
        std::string_view pct = MWBase::Environment::get().getWindowManager()->getGameSettingString("spercent", {});
        std::string_view ft = MWBase::Environment::get().getWindowManager()->getGameSettingString("sfeet", {});
        std::string_view lvl = MWBase::Environment::get().getWindowManager()->getGameSettingString("sLevel", {});
        std::string_view lvls = MWBase::Environment::get().getWindowManager()->getGameSettingString("sLevels", {});
        std::string to
            = " " + std::string{ MWBase::Environment::get().getWindowManager()->getGameSettingString("sTo", {}) } + " ";
        std::string sec
            = " " + std::string{ MWBase::Environment::get().getWindowManager()->getGameSettingString("ssecond", {}) };
        std::string secs
            = " " + std::string{ MWBase::Environment::get().getWindowManager()->getGameSettingString("sseconds", {}) };

        const std::string& effectIDStr = ESM::MagicEffect::effectIdToString(mEffectParams.mEffectID);
        std::string spellLine{ MWBase::Environment::get().getWindowManager()->getGameSettingString(effectIDStr, {}) };

        if (magicEffect->mData.mFlags & ESM::MagicEffect::TargetSkill && mEffectParams.mSkill != -1)
        {
            spellLine += ' ';
            spellLine += MWBase::Environment::get().getWindowManager()->getGameSettingString(
                ESM::Skill::sSkillNameIds[mEffectParams.mSkill], {});
        }
        if (magicEffect->mData.mFlags & ESM::MagicEffect::TargetAttribute && mEffectParams.mAttribute != -1)
        {
            spellLine += ' ';
            spellLine += MWBase::Environment::get().getWindowManager()->getGameSettingString(
                ESM::Attribute::sGmstAttributeIds[mEffectParams.mAttribute], {});
        }

        if (mEffectParams.mMagnMin || mEffectParams.mMagnMax)
        {
            ESM::MagicEffect::MagnitudeDisplayType displayType = magicEffect->getMagnitudeDisplayType();
            if (displayType == ESM::MagicEffect::MDT_TimesInt)
            {
                std::string_view timesInt
                    = MWBase::Environment::get().getWindowManager()->getGameSettingString("sXTimesINT", {});
                std::stringstream formatter;

                formatter << std::fixed << std::setprecision(1) << " " << (mEffectParams.mMagnMin / 10.0f);
                if (mEffectParams.mMagnMin != mEffectParams.mMagnMax)
                    formatter << to << (mEffectParams.mMagnMax / 10.0f);
                formatter << timesInt;

                spellLine += formatter.str();
            }
            else if (displayType != ESM::MagicEffect::MDT_None && !mEffectParams.mNoMagnitude)
            {
                spellLine += " " + MyGUI::utility::toString(mEffectParams.mMagnMin);
                if (mEffectParams.mMagnMin != mEffectParams.mMagnMax)
                    spellLine += to + MyGUI::utility::toString(mEffectParams.mMagnMax);

                if (displayType == ESM::MagicEffect::MDT_Percentage)
                    spellLine += pct;
                else if (displayType == ESM::MagicEffect::MDT_Feet)
                {
                    spellLine += ' ';
                    spellLine += ft;
                }
                else if (displayType == ESM::MagicEffect::MDT_Level)
                {
                    spellLine += ' ';
                    if (mEffectParams.mMagnMin == mEffectParams.mMagnMax && std::abs(mEffectParams.mMagnMin) == 1)
                        spellLine += lvl;
                    else
                        spellLine += lvls;
                }
                else // ESM::MagicEffect::MDT_Points
                {
                    spellLine += ' ';
                    if (mEffectParams.mMagnMin == mEffectParams.mMagnMax && std::abs(mEffectParams.mMagnMin) == 1)
                        spellLine += pt;
                    else
                        spellLine += pts;
                }
            }
        }

        // constant effects have no duration and no target
        if (!mEffectParams.mIsConstant)
        {
            if (!(magicEffect->mData.mFlags & ESM::MagicEffect::AppliedOnce))
                mEffectParams.mDuration = std::max(1, mEffectParams.mDuration);

            if (mEffectParams.mDuration > 0 && !(magicEffect->mData.mFlags & ESM::MagicEffect::NoDuration))
            {
                spellLine += ' ';
                spellLine += MWBase::Environment::get().getWindowManager()->getGameSettingString("sfor", {});
                spellLine += ' ' + MyGUI::utility::toString(mEffectParams.mDuration)
                    + ((mEffectParams.mDuration == 1) ? sec : secs);
            }

            if (mEffectParams.mArea > 0)
            {
                spellLine += " #{sin} " + MyGUI::utility::toString(mEffectParams.mArea) + " #{sfootarea}";
            }

            // potions have no target
            if (!mEffectParams.mNoTarget)
            {
                spellLine += ' ';
                spellLine += MWBase::Environment::get().getWindowManager()->getGameSettingString("sonword", {});
                spellLine += ' ';
                if (mEffectParams.mRange == ESM::RT_Self)
                    spellLine += MWBase::Environment::get().getWindowManager()->getGameSettingString("sRangeSelf", {});
                else if (mEffectParams.mRange == ESM::RT_Touch)
                    spellLine += MWBase::Environment::get().getWindowManager()->getGameSettingString("sRangeTouch", {});
                else if (mEffectParams.mRange == ESM::RT_Target)
                    spellLine
                        += MWBase::Environment::get().getWindowManager()->getGameSettingString("sRangeTarget", {});
            }
        }

        mTextWidget->setCaptionWithReplacing(spellLine);
        mRequestedWidth = mTextWidget->getTextSize().width + sIconOffset;

        mImageWidget->setImageTexture(Misc::ResourceHelpers::correctIconPath(
            magicEffect->mIcon, MWBase::Environment::get().getResourceSystem()->getVFS()));
    }

    MWSpellEffect::~MWSpellEffect() {}

    void MWSpellEffect::initialiseOverride()
    {
        Base::initialiseOverride();

        assignWidget(mTextWidget, "Text");
        assignWidget(mImageWidget, "Image");
    }

    /* MWDynamicStat */

    MWDynamicStat::MWDynamicStat()
        : mValue(0)
        , mMax(1)
        , mTextWidget(nullptr)
        , mBarWidget(nullptr)
        , mBarTextWidget(nullptr)
    {
    }

    void MWDynamicStat::setValue(int cur, int max)
    {
        mValue = cur;
        mMax = max;

        if (mBarWidget)
        {
            mBarWidget->setProgressRange(std::max(0, mMax));
            mBarWidget->setProgressPosition(std::max(0, mValue));
        }

        if (mBarTextWidget)
        {
            std::stringstream out;
            out << mValue << "/" << mMax;
            mBarTextWidget->setCaption(out.str().c_str());
        }
    }
    void MWDynamicStat::setTitle(std::string_view text)
    {
        if (mTextWidget)
            mTextWidget->setCaption(toUString(text));
    }

    MWDynamicStat::~MWDynamicStat() {}

    void MWDynamicStat::initialiseOverride()
    {
        Base::initialiseOverride();

        assignWidget(mTextWidget, "Text");
        assignWidget(mBarWidget, "Bar");
        assignWidget(mBarTextWidget, "BarText");
    }
}
