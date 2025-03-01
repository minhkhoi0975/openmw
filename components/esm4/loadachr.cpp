/*
  Copyright (C) 2016, 2018, 2020-2021 cc9cii

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  cc9cii cc9c@iinet.net.au

  Much of the information on the data structures are based on the information
  from Tes4Mod:Mod_File_Format and Tes5Mod:File_Formats but also refined by
  trial & error.  See http://en.uesp.net/wiki for details.

*/
#include "loadachr.hpp"

#include <stdexcept>

#include "reader.hpp"
//#include "writer.hpp"

void ESM4::ActorCharacter::load(ESM4::Reader& reader)
{
    mId = reader.getFormIdFromHeader();
    mFlags = reader.hdr().record.flags;
    mParent = reader.currCell();

    while (reader.getSubRecordHeader())
    {
        const ESM4::SubRecordHeader& subHdr = reader.subRecordHeader();
        switch (subHdr.typeId)
        {
            case ESM4::SUB_EDID:
                reader.getZString(mEditorId);
                break;
            case ESM4::SUB_FULL:
                reader.getZString(mFullName);
                break;
            case ESM4::SUB_NAME:
                reader.getFormId(mBaseObj);
                break;
            case ESM4::SUB_DATA:
                reader.get(mPos);
                break;
            case ESM4::SUB_XSCL:
                reader.get(mScale);
                break;
            case ESM4::SUB_XOWN:
            {
                switch (subHdr.dataSize)
                {
                    case 4:
                        reader.getFormId(mOwner);
                        break;
                    case 12:
                    {
                        reader.getFormId(mOwner);
                        std::uint32_t dummy;
                        reader.get(dummy); // Unknown
                        reader.get(dummy); // No crime flag, FO4
                        break;
                    }
                    default:
                        reader.skipSubRecordData();
                        break;
                }
                break;
            }
            case ESM4::SUB_XESP:
                reader.getFormId(mEsp.parent);
                reader.get(mEsp.flags);
                break;
            case ESM4::SUB_XCNT:
            {
                reader.get(mCount);
                break;
            }
            case ESM4::SUB_XRGD: // ragdoll
            case ESM4::SUB_XRGB: // ragdoll biped
            case ESM4::SUB_XHRS: // horse formId
            case ESM4::SUB_XMRC: // merchant container formId
            // TES5
            case ESM4::SUB_XAPD: // activation parent
            case ESM4::SUB_XAPR: // active parent
            case ESM4::SUB_XEZN: // encounter zone
            case ESM4::SUB_XHOR:
            case ESM4::SUB_XLCM: // levelled creature
            case ESM4::SUB_XLCN: // location
            case ESM4::SUB_XLKR: // location route?
            case ESM4::SUB_XLRT: // location type
            //
            case ESM4::SUB_XPRD:
            case ESM4::SUB_XPPA:
            case ESM4::SUB_INAM:
            case ESM4::SUB_PDTO:
            //
            case ESM4::SUB_XIS2:
            case ESM4::SUB_XPCI: // formId
            case ESM4::SUB_XLOD:
            case ESM4::SUB_VMAD:
            case ESM4::SUB_XLRL: // Unofficial Skyrim Patch
            case ESM4::SUB_XRDS: // FO3
            case ESM4::SUB_XIBS: // FO3
            case ESM4::SUB_SCHR: // FO3
            case ESM4::SUB_TNAM: // FO3
            case ESM4::SUB_XATO: // FONV
            case ESM4::SUB_MNAM: // FO4
            case ESM4::SUB_XATP: // FO4
            case ESM4::SUB_XEMI: // FO4
            case ESM4::SUB_XFVC: // FO4
            case ESM4::SUB_XHLT: // FO4
            case ESM4::SUB_XHTW: // FO4
            case ESM4::SUB_XLKT: // FO4
            case ESM4::SUB_XLYR: // FO4
            case ESM4::SUB_XMBR: // FO4
            case ESM4::SUB_XMSP: // FO4
            case ESM4::SUB_XPLK: // FO4
            case ESM4::SUB_XRFG: // FO4
            case ESM4::SUB_XRNK: // FO4
                reader.skipSubRecordData();
                break;
            default:
                throw std::runtime_error("ESM4 ACHR/ACRE load - Unknown subrecord " + ESM::printName(subHdr.typeId));
        }
    }
}

// void ESM4::ActorCharacter::save(ESM4::Writer& writer) const
//{
// }

// void ESM4::ActorCharacter::blank()
//{
// }
