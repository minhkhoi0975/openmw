#include "loadbook.hpp"

#include "esmreader.hpp"
#include "esmwriter.hpp"

#include <components/esm/defs.hpp>
#include <components/esm/esmcommon.hpp>
#include <components/esm/fourcc.hpp>

namespace ESM
{
    void Book::load(ESMReader& esm, bool& isDeleted)
    {
        isDeleted = false;
        mRecordFlags = esm.getRecordFlags();

        bool hasName = false;
        bool hasData = false;
        while (esm.hasMoreSubs())
        {
            esm.getSubName();
            switch (esm.retSubName().toInt())
            {
                case SREC_NAME:
                    mId = esm.getRefId();
                    hasName = true;
                    break;
                case fourCC("MODL"):
                    mModel = esm.getHString();
                    break;
                case fourCC("FNAM"):
                    mName = esm.getHString();
                    break;
                case fourCC("BKDT"):
                    esm.getHTSized<20>(mData);
                    hasData = true;
                    break;
                case fourCC("SCRI"):
                    mScript = esm.getRefId();
                    break;
                case fourCC("ITEX"):
                    mIcon = esm.getHString();
                    break;
                case fourCC("ENAM"):
                    mEnchant = esm.getRefId();
                    break;
                case fourCC("TEXT"):
                    mText = esm.getHString();
                    break;
                case SREC_DELE:
                    esm.skipHSub();
                    isDeleted = true;
                    break;
                default:
                    esm.fail("Unknown subrecord");
                    break;
            }
        }

        if (!hasName)
            esm.fail("Missing NAME subrecord");
        if (!hasData && !isDeleted)
            esm.fail("Missing BKDT subrecord");
    }
    void Book::save(ESMWriter& esm, bool isDeleted) const
    {
        esm.writeHNCString("NAME", mId.getRefIdString());

        if (isDeleted)
        {
            esm.writeHNString("DELE", "", 3);
            return;
        }

        esm.writeHNCString("MODL", mModel);
        esm.writeHNOCString("FNAM", mName);
        esm.writeHNT("BKDT", mData, 20);
        esm.writeHNOCString("SCRI", mScript.getRefIdString());
        esm.writeHNOCString("ITEX", mIcon);
        esm.writeHNOString("TEXT", mText);
        esm.writeHNOCString("ENAM", mEnchant.getRefIdString());
    }

    void Book::blank()
    {
        mRecordFlags = 0;
        mData.mWeight = 0;
        mData.mValue = 0;
        mData.mIsScroll = 0;
        mData.mSkillId = 0;
        mData.mEnchant = 0;
        mName.clear();
        mModel.clear();
        mIcon.clear();
        mScript = ESM::RefId::sEmpty;
        mEnchant = ESM::RefId::sEmpty;
        mText.clear();
    }
}
